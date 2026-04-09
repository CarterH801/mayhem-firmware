/*
 * NOAA Weather Satellite Receiver implementation
 * File: firmware/application/apps/ui_noaa_sat.cpp
 */

#include "ui_noaa_sat.hpp"
#include "portapack.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "rtc_time.hpp"
#include "baseband_api.hpp"

using namespace portapack;

namespace ui {

// ─────────────────────────────────────────────────────────
// APTDecoder
// ─────────────────────────────────────────────────────────

float APTDecoder::detect_am(float sample) {
    // Simple envelope detector
    float rectified = std::fabs(sample);
    am_level_ = am_level_ * 0.95f + rectified * 0.05f;
    return am_level_;
}

bool APTDecoder::feed_sample(float sample) {
    float envelope = detect_am(sample);

    // Advance carrier phase for sync detection
    carrier_phase_ += (2.0f * 3.14159265f * CARRIER_FREQ) / SAMPLE_RATE;
    if (carrier_phase_ > 2.0f * 3.14159265f)
        carrier_phase_ -= 2.0f * 3.14159265f;

    // Convert AM level to pixel (0-255)
    uint8_t pixel = static_cast<uint8_t>(
        std::min(255.0f, std::max(0.0f, envelope * 512.0f)));

    if (pixel_count_ < LINE_WIDTH) {
        line_buf_[pixel_count_++] = pixel;
    }

    if (pixel_count_ >= LINE_WIDTH) {
        pixel_count_ = 0;
        line_count_++;
        return true;  // complete line
    }
    return false;
}

// ─────────────────────────────────────────────────────────
// NOAASatView
// ─────────────────────────────────────────────────────────

NOAASatView::NOAASatView(NavigationView& nav)
    : nav_(nav) {

    add_children({
        &label_sat,
        &options_sat,
        &text_status,
        &text_signal,
        &text_lines,
        &bar_progress,
        &text_filename,
        &button_rx,
        &button_clear,
        &button_back,
        &label_hint,
        &label_hint2,
    });

    options_sat.on_change = [this](size_t idx, int32_t) {
        selected_sat_ = idx;
    };

    button_rx.on_select = [this](Button&) {
        if (!receiving_)
            start_receive();
        else
            stop_receive();
    };

    button_clear.on_select = [this](Button&) {
        preview_.fill(0);
        preview_line_ = 0;
        preview_dirty_ = true;
        lines_received_ = 0;
        text_lines.set("Lines: 0");
        bar_progress.set_value(0);
        set_dirty();
    };

    button_back.on_select = [this](Button&) {
        stop_receive();
        nav_.pop();
    };

    bar_progress.set_max(2400);
    bar_progress.set_value(0);
}

NOAASatView::~NOAASatView() {
    stop_receive();
}

void NOAASatView::focus() {
    button_rx.focus();
}

void NOAASatView::start_receive() {
    if (receiving_) return;

    const auto& sat = NOAA_SATS[selected_sat_];
    receiver_model.set_target_frequency(sat.freq_hz);
    receiver_model.set_modulation(ReceiverModel::Mode::WidebandFMAudio);
    receiver_model.set_am_configuration(0);
    receiver_model.enable();

    decoder_.reset();
    preview_.fill(0);
    preview_line_ = 0;
    lines_received_ = 0;
    image_started_ = false;

    open_image_file();

    receiving_ = true;
    button_rx.set_text("STOP");
    text_status.set(std::string("Listening: ") + sat.name);
}

void NOAASatView::stop_receive() {
    if (!receiving_) return;

    receiving_ = false;
    receiver_model.disable();
    button_rx.set_text("RX");
    text_status.set("Stopped. Lines: " +
                    to_string_dec_uint(lines_received_));
}

void NOAASatView::process_audio_line(const uint8_t* line_data) {
    lines_received_++;
    text_lines.set("Lines: " + to_string_dec_uint(lines_received_));
    bar_progress.set_value(std::min((uint32_t)2400, lines_received_));

    update_preview(line_data, lines_received_);

    // Write raw line to PGM file
    image_file_.write(line_data, APTDecoder::LINE_WIDTH);
}

void NOAASatView::update_preview(const uint8_t* line_data, int line_num) {
    if (preview_line_ >= PREVIEW_H) return;

    // Downsample: LINE_WIDTH -> PREVIEW_W
    float scale = static_cast<float>(APTDecoder::LINE_WIDTH) / PREVIEW_W;
    for (int x = 0; x < PREVIEW_W; x++) {
        int src = static_cast<int>(x * scale);
        if (src >= APTDecoder::LINE_WIDTH) src = APTDecoder::LINE_WIDTH - 1;
        preview_[preview_line_ * PREVIEW_W + x] = line_data[src];
    }
    preview_line_++;
    preview_dirty_ = true;
    (void)line_num;
}

void NOAASatView::draw_preview(Painter& painter) {
    for (int y = 0; y < preview_line_ && y < PREVIEW_H; y++) {
        for (int x = 0; x < PREVIEW_W; x++) {
            uint8_t v = preview_[y * PREVIEW_W + x];
            Color c(v, v, v);
            painter.fill_rectangle({{x, IMG_TOP + y}, {1, 1}}, c);
        }
    }
    preview_dirty_ = false;
}

void NOAASatView::paint(Painter& painter) {
    View::paint(painter);
    if (preview_dirty_) {
        draw_preview(painter);
    }
}

void NOAASatView::open_image_file() {
    rtc::RTC datetime;
    rtc_time::now(datetime);

    image_filename_ = "NOAA/noaa_" +
        to_string_dec_uint(datetime.year(), 4, '0') +
        to_string_dec_uint(datetime.month(), 2, '0') +
        to_string_dec_uint(datetime.day(), 2, '0') +
        "_" +
        to_string_dec_uint(datetime.hour(), 2, '0') +
        ".pgm";

    ensure_directory("NOAA");
    auto err = image_file_.create(image_filename_);
    if (err.is_valid()) {
        text_filename.set("File: ERR");
        return;
    }

    write_pgm_header();
    text_filename.set("File: " + image_filename_);
}

void NOAASatView::write_pgm_header() {
    // PGM header: P5 <width> <height> <maxval>
    std::string hdr = "P5\n" +
        to_string_dec_uint(APTDecoder::LINE_WIDTH) + " 2400\n255\n";
    image_file_.write(hdr.data(), hdr.size());
}

}  // namespace ui
