/*
 * Spectrum Waterfall Recorder implementation
 * File: firmware/application/apps/ui_waterfall_rec.cpp
 */

#include "ui_waterfall_rec.hpp"
#include "portapack.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "rtc_time.hpp"

using namespace portapack;

namespace ui {

// Band presets
const WaterfallRecView::BandPreset
    WaterfallRecView::PRESETS[] = {
    {"FM 87-108MHz",  87'500'000, 108'000'000, 200'000},
    {"VHF 136-175",  136'000'000, 175'000'000, 500'000},
    {"KFOB 313-317", 313'000'000, 317'000'000,  25'000},
    {"UHF 430-440",  430'000'000, 440'000'000, 100'000},
    {"900-928MHz",   902'000'000, 928'000'000, 500'000},
    {"Custom",        87'500'000, 108'000'000, 200'000},
};
const int WaterfallRecView::PRESET_COUNT = 6;

WaterfallRecView::WaterfallRecView(NavigationView& nav)
    : nav_(nav) {

    // Init waterfall buffer
    for (int r = 0; r < WFALL_ROWS; r++)
        for (int c = 0; c < WFALL_COLS; c++)
            wfall_buf_[r][c] = -120;

    add_children({
        &label_band, &options_band,
        &label_interval, &options_interval,
        &text_status,
        &text_sweeps, &text_file_size,
        &label_wfall,
        &text_scale_l, &text_scale_r,
        &text_filename,
        &button_rec,
        &button_clear,
        &button_back,
    });

    // Update scale labels
    text_scale_l.set(
        to_string_dec_uint(
            (uint32_t)(scan_start_ / 1'000'000)) + "MHz");
    text_scale_r.set(
        to_string_dec_uint(
            (uint32_t)(scan_end_ / 1'000'000)) + "MHz");

    options_band.on_change = [this](size_t idx,
                                    OptionsField::value_t){
        if (idx < (size_t)PRESET_COUNT && !recording_) {
            const auto& p = PRESETS[idx];
            scan_start_ = p.start;
            scan_end_   = p.end;
            scan_step_  = p.step;

            // Recalculate steps
            sweep_total_steps_ = (int)
                ((scan_end_ - scan_start_) /
                 scan_step_);
            if (sweep_total_steps_ > MAX_STEPS)
                sweep_total_steps_ = MAX_STEPS;

            text_scale_l.set(
                to_string_dec_uint(
                    (uint32_t)(scan_start_ / 1'000'000)) +
                "MHz");
            text_scale_r.set(
                to_string_dec_uint(
                    (uint32_t)(scan_end_ / 1'000'000)) +
                "MHz");
        }
    };

    options_interval.on_change = [this](size_t idx,
                                        OptionsField::value_t){
        static const uint32_t intervals[] =
            {10, 30, 60, 300};
        if (idx < 4) interval_sec_ = intervals[idx];
    };

    button_rec.on_select = [this](Button&) {
        if (!recording_) start_recording();
        else             stop_recording();
    };

    button_clear.on_select = [this](Button&) {
        for (int r = 0; r < WFALL_ROWS; r++)
            for (int c = 0; c < WFALL_COLS; c++)
                wfall_buf_[r][c] = -120;
        wfall_dirty_ = true;
        set_dirty();
    };

    button_back.on_select = [this](Button&) {
        stop_recording();
        nav_.pop();
    };

    // Calculate initial step count
    sweep_total_steps_ = (int)
        ((scan_end_ - scan_start_) / scan_step_);
    if (sweep_total_steps_ > MAX_STEPS)
        sweep_total_steps_ = MAX_STEPS;
}

WaterfallRecView::~WaterfallRecView() {
    stop_recording();
}

void WaterfallRecView::focus() {
    button_rec.focus();
}

void WaterfallRecView::start_recording() {
    recording_   = true;
    sweep_count_ = 0;
    sweep_idx_   = 0;

    button_rec.set_text("STOP");
    button_rec.set_style(ui::Theme::getInstance()->fg_light);

    open_log_file();
    start_sweep();

    receiver_model.set_modulation(
        ReceiverModel::Mode::NarrowbandFMAudio);
    receiver_model.set_sampling_rate(3072000);
    receiver_model.set_baseband_bandwidth(1750000);
    receiver_model.enable();

    text_status.set("Recording...");
}

void WaterfallRecView::stop_recording() {
    if (!recording_) return;
    recording_    = false;
    scan_active_  = false;

    receiver_model.disable();

    if (file_open_) {
        log_file_.close();
        file_open_ = false;
    }

    button_rec.set_text("REC");
    button_rec.set_style(ui::Theme::getInstance()->fg_light);
    text_status.set(
        "Stopped. " +
        to_string_dec_uint(sweep_count_) +
        " sweeps saved to " + log_filename_);
}

void WaterfallRecView::start_sweep() {
    scan_active_  = true;
    sweep_idx_    = 0;
    current_freq_ = scan_start_;
    receiver_model.set_target_frequency(current_freq_);
}

void WaterfallRecView::next_step() {
    sweep_idx_++;
    if (sweep_idx_ >= sweep_total_steps_) {
        finish_sweep();
        return;
    }
    current_freq_ += scan_step_;
    receiver_model.set_target_frequency(current_freq_);
}

void WaterfallRecView::finish_sweep() {
    scan_active_ = false;
    sweep_count_++;

    // Add to waterfall display buffer
    const int row = wfall_row_ % WFALL_ROWS;
    for (int c = 0; c < sweep_total_steps_ &&
                    c < WFALL_COLS; c++) {
        // Downsample if needed
        const int src = c * sweep_total_steps_ /
                        WFALL_COLS;
        wfall_buf_[row][c] =
            src < MAX_STEPS ? sweep_buf_[src] : -120;
    }
    wfall_row_   = (wfall_row_ + 1) % WFALL_ROWS;
    wfall_dirty_ = true;

    // Write to file if interval has elapsed
    const auto now_sec = rtc_time::now().second();
    if (sweep_count_ == 1 ||
        (sweep_count_ % (interval_sec_ / 1)) == 0) {
        write_sweep_to_file();
    }

    text_sweeps.set(
        "Sweeps:" + to_string_dec_uint(sweep_count_));
    set_dirty();

    // Start next sweep after interval
    // In a real implementation use a timer
    // For now start immediately
    if (recording_) start_sweep();
}

void WaterfallRecView::on_statistics_update(
    const RSSIStatisticsMessage& message) {

    if (!recording_ || !scan_active_) return;

    // Store RSSI for current step
    if (sweep_idx_ < MAX_STEPS) {
        sweep_buf_[sweep_idx_] = message.statistics.max;
    }

    next_step();
}

void WaterfallRecView::write_sweep_to_file() {
    if (!file_open_) return;

    const auto now = rtc_time::now();
    std::string row =
        to_string_dec_uint(now.hour()) + ":" +
        to_string_dec_uint(now.minute()) + ":" +
        to_string_dec_uint(now.second()) + "," +
        to_string_dec_uint(
            (uint32_t)(scan_start_ / 1'000'000)) + "," +
        to_string_dec_uint(
            (uint32_t)(scan_end_ / 1'000'000));

    for (int i = 0; i < sweep_total_steps_; i++) {
        row += "," + to_string_dec_int(sweep_buf_[i]);
    }
    row += "\n";

    log_file_.write(row.c_str(), row.size());
}

void WaterfallRecView::open_log_file() {
    make_new_directory("/WATERFALL");

    const auto now = rtc_time::now();
    log_filename_ =
        "/WATERFALL/wfall_" +
        to_string_dec_uint(now.year()) +
        to_string_dec_uint(now.month()) +
        to_string_dec_uint(now.day()) + "_" +
        to_string_dec_uint(now.hour()) +
        ".csv";

    if (log_file_.open(log_filename_, false, true)) {
        file_open_ = true;
        // Write CSV header
        std::string hdr =
            "time,start_mhz,end_mhz";
        for (int i = 0; i < sweep_total_steps_; i++) {
            const uint64_t f =
                scan_start_ + i * scan_step_;
            hdr += "," +
                to_string_dec_uint(
                    (uint32_t)(f / 1000)) + "k";
        }
        hdr += "\n";
        log_file_.write(hdr.c_str(), hdr.size());

        text_filename.set("File: " + log_filename_);
    }
}

// ─────────────────────────────────────────
// Paint — draw the waterfall
// Each pixel = one RSSI sample
// Color = heat map: blue→cyan→green→yellow→red
// ─────────────────────────────────────────
void WaterfallRecView::paint(Painter& painter) {
    View::paint(painter);

    if (!wfall_dirty_) return;
    wfall_dirty_ = false;

    // Waterfall area: x=0..239, y=70..182
    const int WFALL_Y_START = 70;
    const int WFALL_Y_END   = 182;
    const int WFALL_H = WFALL_Y_END - WFALL_Y_START;
    const int WFALL_W = 240;

    // Draw rows newest at top, oldest at bottom
    for (int row = 0; row < WFALL_H; row++) {
        // Map row to circular buffer index
        const int buf_row =
            (wfall_row_ - 1 - row + WFALL_ROWS) %
            WFALL_ROWS;

        for (int col = 0; col < WFALL_W; col++) {
            // Map col to buffer column
            const int buf_col =
                col * WFALL_COLS / WFALL_W;
            const int32_t rssi =
                wfall_buf_[buf_row][buf_col];
            const Color c = rssi_to_color(rssi);

            painter.fill_rectangle(
                {{col, WFALL_Y_START + row}, {1, 1}}, c);
        }
    }
}

// Convert RSSI to heatmap color
// -120 dBm = black/dark blue
// -90 dBm  = blue
// -70 dBm  = cyan
// -60 dBm  = green
// -50 dBm  = yellow
// -40 dBm  = orange
// -30 dBm  = red/bright
Color WaterfallRecView::rssi_to_color(
    int32_t rssi) const {

    if (rssi <= -120) return {0,   0,  20};  // Near black
    if (rssi <= -100) return {0,   0,  80};  // Dark blue
    if (rssi <=  -90) return {0,   0, 255};  // Blue
    if (rssi <=  -80) return {0, 150, 200};  // Cyan-blue
    if (rssi <=  -70) return {0, 255, 255};  // Cyan
    if (rssi <=  -60) return {0, 255,   0};  // Green
    if (rssi <=  -50) return {255,255,  0};  // Yellow
    if (rssi <=  -40) return {255,128,  0};  // Orange
    return                   {255,  0,  0};  // Red
}

}  // namespace ui
