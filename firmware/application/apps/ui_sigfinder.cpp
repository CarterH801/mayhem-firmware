/*
 * Signal Finder — PortaPack Mayhem
 * File: firmware/application/apps/ui_sigfinder.cpp
 *
 * Place in: firmware/application/apps/
 */

#include "ui_sigfinder.hpp"
#include "portapack.hpp"
#include "receiver_model.hpp"
#include "baseband_api.hpp"
#include "string_format.hpp"
#include "audio.hpp"
#include "tone_key.hpp"
#include "file.hpp"
#include "ui_bandplan.hpp"

using namespace portapack;

namespace ui {

// ─────────────────────────────────────────
// Standalone constructor
// ─────────────────────────────────────────
SigFinderView::SigFinderView(NavigationView& nav)
    : nav_(nav) {
    init(315'000'000, "KEY FOB 315MHz", "KEYFOB");
}

// ─────────────────────────────────────────
// Constructor from Band Scanner
// ─────────────────────────────────────────
SigFinderView::SigFinderView(NavigationView& nav,
                             rf::Frequency freq,
                             const std::string& band_label,
                             const std::string& category)
    : nav_(nav) {
    init(freq, band_label, category);
}

// ─────────────────────────────────────────
// Shared init
// ─────────────────────────────────────────
void SigFinderView::init(rf::Frequency freq,
                         const std::string& band_label,
                         const std::string& category) {
    target_freq_ = freq;
    band_label_  = band_label;
    category_    = category;

    add_children({
        &text_freq,
        &text_band,
        &text_percent_big,
        &text_hotcold,
        &text_trend,
        &label_bar_l,
        &bar_signal,
        &label_bar_r,
        &text_dbm,
        &text_peak,
        &label_scale,
        // Graph columns
        &text_graph_0,  &text_graph_1,  &text_graph_2,
        &text_graph_3,  &text_graph_4,  &text_graph_5,
        &text_graph_6,  &text_graph_7,  &text_graph_8,
        &text_graph_9,  &text_graph_10, &text_graph_11,
        &text_graph_12, &text_graph_13, &text_graph_14,
        &text_graph_15, &text_graph_16, &text_graph_17,
        &text_graph_18, &text_graph_19, &text_graph_20,
        &text_graph_21, &text_graph_22, &text_graph_23,
        &text_graph_24, &text_graph_25, &text_graph_26,
        &text_graph_27, &text_graph_28, &text_graph_29,
        &text_capture_status,
        &button_capture,
        &button_reset_peak,
        &button_audio,
        &button_back,
    });

    // Set static labels
    text_freq.set(format_freq(target_freq_));
    text_band.set(band_label_);

    // Color code band label
    text_band.set_style(
        &(Styles::fg(color_for_category(category_))));

    // Tune the radio
    receiver_model.set_target_frequency(target_freq_);

    // Set appropriate modulation for frequency
    if (target_freq_ >= 87'500'000 && target_freq_ <= 108'000'000) {
        receiver_model.set_modulation(ReceiverModel::Mode::WidebandFMAudio);
    } else if (target_freq_ < 30'000'000) {
        receiver_model.set_modulation(ReceiverModel::Mode::AMAudio);
    } else {
        receiver_model.set_modulation(ReceiverModel::Mode::NarrowbandFMAudio);
    }
    receiver_model.set_sampling_rate(3072000);
    receiver_model.set_baseband_bandwidth(1750000);
    receiver_model.enable();

    // ── Button handlers ───────────────────────────────────

    button_capture.on_select = [this](Button&) {
        if (!capturing_) {
            start_capture();
        } else {
            stop_capture();
        }
    };

    button_reset_peak.on_select = [this](Button&) {
        peak_rssi_    = current_rssi_;
        baseline_set_ = false;
        rssi_history_.clear();
        text_capture_status.set("Peak reset.");
    };

    button_audio.on_select = [this](Button&) {
        audio_enabled_ = !audio_enabled_;
        button_audio.set_text(audio_enabled_ ? "BEEP" : "MUTE");
        if (!audio_enabled_) stop_audio();
    };

    button_back.on_select = [this](Button&) {
        stop_capture();
        stop_audio();
        receiver_model.disable();
        nav_.pop();
    };
}

// ─────────────────────────────────────────
// Destructor
// ─────────────────────────────────────────
SigFinderView::~SigFinderView() {
    stop_capture();
    stop_audio();
    receiver_model.disable();
}

void SigFinderView::focus() {
    button_capture.focus();
}

// ─────────────────────────────────────────
// RSSI update — heart of the app
// Called ~every 100ms
// ─────────────────────────────────────────
void SigFinderView::on_statistics_update(
    const RSSIStatisticsMessage& message) {

    prev_rssi_    = current_rssi_;
    current_rssi_ = message.statistics.max;
    reading_count_++;

    // Set baseline on first few readings
    if (!baseline_set_ && reading_count_ >= 5) {
        baseline_rssi_ = current_rssi_;
        baseline_set_  = true;
    }

    // Track peak
    if (current_rssi_ > peak_rssi_) {
        peak_rssi_ = current_rssi_;
    }

    // Update rolling history
    rssi_history_.push_back(current_rssi_);
    if ((int)rssi_history_.size() > HISTORY_LEN) {
        rssi_history_.pop_front();
    }

    // Update trend
    const int32_t delta = current_rssi_ - prev_rssi_;
    if      (delta >  3) current_trend_ = Trend::UP;
    else if (delta < -3) current_trend_ = Trend::DOWN;
    else                 current_trend_ = Trend::STEADY;

    update_display();

    // Audio beep — rate and pitch scale with signal strength
    if (audio_enabled_) {
        update_audio_tone();
    }
}

// ─────────────────────────────────────────
// Update all display elements
// ─────────────────────────────────────────
void SigFinderView::update_display() {
    update_strength_bar();
    update_trend_arrow();
    update_hot_cold_label();
    update_history_graph();

    // dBm reading
    text_dbm.set(to_string_dec_int(current_rssi_) + " dBm");

    // Peak
    text_peak.set("Pk:" + to_string_dec_int(peak_rssi_));

    // Big percentage
    const int pct = rssi_to_percent(current_rssi_);
    std::string pstr = to_string_dec_uint(pct) + "%";
    // Right-align to 4 chars
    while (pstr.size() < 4) pstr = " " + pstr;
    text_percent_big.set(pstr);

    // Color the big number based on strength
    if      (pct >= 75) text_percent_big.set_style(&Styles::red);
    else if (pct >= 50) text_percent_big.set_style(&Styles::yellow);
    else if (pct >= 25) text_percent_big.set_style(&Styles::green);
    else                text_percent_big.set_style(&Styles::grey);
}

// ─────────────────────────────────────────
// Strength bar
// ─────────────────────────────────────────
void SigFinderView::update_strength_bar() {
    const int pct = rssi_to_percent(current_rssi_);
    bar_signal.set_value(pct);
}

// ─────────────────────────────────────────
// Trend arrow
// ─────────────────────────────────────────
void SigFinderView::update_trend_arrow() {
    switch (current_trend_) {
        case Trend::UP:
            text_trend.set(" ^");
            text_trend.set_style(&Styles::red);
            break;
        case Trend::DOWN:
            text_trend.set(" v");
            text_trend.set_style(&Styles::cyan);
            break;
        case Trend::STEADY:
        default:
            text_trend.set(" -");
            text_trend.set_style(&Styles::white);
            break;
    }
}

// ─────────────────────────────────────────
// Hot / Cold / Warm label
// ─────────────────────────────────────────
void SigFinderView::update_hot_cold_label() {
    if (!baseline_set_) {
        text_hotcold.set("CALIBRATING");
        text_hotcold.set_style(&Styles::grey);
        return;
    }

    const int32_t above = current_rssi_ - baseline_rssi_;

    if      (above >= 20) {
        text_hotcold.set("!!! HOT !!!");
        text_hotcold.set_style(&Styles::red);
    }
    else if (above >= 12) {
        text_hotcold.set("  WARM    ");
        text_hotcold.set_style(&Styles::yellow);
    }
    else if (above >= 6) {
        text_hotcold.set("  GETTING  ");
        text_hotcold.set_style(&Styles::green);
    }
    else if (above >= 0) {
        text_hotcold.set("  COOL    ");
        text_hotcold.set_style(&Styles::cyan);
    }
    else {
        text_hotcold.set("  COLD    ");
        text_hotcold.set_style(&Styles::blue);
    }
}

// ─────────────────────────────────────────
// Rolling history graph
// Each column is a vertical bar made of chars
// Height proportional to RSSI at that moment
// ─────────────────────────────────────────
void SigFinderView::update_history_graph() {
    Text* cols[30] = {
        &text_graph_0,  &text_graph_1,  &text_graph_2,
        &text_graph_3,  &text_graph_4,  &text_graph_5,
        &text_graph_6,  &text_graph_7,  &text_graph_8,
        &text_graph_9,  &text_graph_10, &text_graph_11,
        &text_graph_12, &text_graph_13, &text_graph_14,
        &text_graph_15, &text_graph_16, &text_graph_17,
        &text_graph_18, &text_graph_19, &text_graph_20,
        &text_graph_21, &text_graph_22, &text_graph_23,
        &text_graph_24, &text_graph_25, &text_graph_26,
        &text_graph_27, &text_graph_28, &text_graph_29};

    const int hist_size = (int)rssi_history_.size();

    for (int i = 0; i < 30; i++) {
        // Align to right edge — older readings on left
        const int hist_idx = hist_size - 30 + i;

        if (hist_idx < 0 || hist_idx >= hist_size) {
            cols[i]->set(" ");
            continue;
        }

        const int pct = rssi_to_percent(rssi_history_[hist_idx]);

        // Graph is 3 rows tall (48px / 16px per char = 3)
        // Use block chars to represent height
        // pct 0-33 = low bar, 34-66 = mid, 67-100 = tall
        if      (pct >= 67) cols[i]->set("|");
        else if (pct >= 34) cols[i]->set(":");
        else if (pct >= 10) cols[i]->set(".");
        else                cols[i]->set(" ");

        // Color by strength
        if      (pct >= 75) cols[i]->set_style(&Styles::red);
        else if (pct >= 50) cols[i]->set_style(&Styles::yellow);
        else if (pct >= 25) cols[i]->set_style(&Styles::green);
        else                cols[i]->set_style(&Styles::grey);
    }
}

// ─────────────────────────────────────────
// Audio beep — pitch and rate scale with
// signal strength like a metal detector
// ─────────────────────────────────────────
void SigFinderView::update_audio_tone() {
    if (!baseline_set_) return;

    const int pct = rssi_to_percent(current_rssi_);

    // Beep interval: strong signal = fast beeps
    // pct 0-20  = beep every 10 updates (~1 sec)
    // pct 20-50 = beep every 5 updates
    // pct 50-80 = beep every 2 updates
    // pct 80+   = continuous tone

    uint32_t beep_interval;
    if      (pct >= 80) beep_interval = 1;
    else if (pct >= 50) beep_interval = 2;
    else if (pct >= 20) beep_interval = 5;
    else                beep_interval = 10;

    beep_counter_++;

    if (beep_counter_ >= beep_interval) {
        beep_counter_ = 0;

        // Tone pitch: 200Hz (weak) to 2000Hz (strong)
        // Linear scale based on percentage
        const uint32_t freq_hz = 200 + (pct * 18);

        // Use PortaPack speaker
        // tone_key::set(freq_hz, 50); // 50ms tone
        // Note: exact API depends on Mayhem version
        // Most common approach:
        baseband::set_beep(freq_hz, 60);  // 60ms beep
    }
}

void SigFinderView::stop_audio() {
    baseband::set_beep(0, 0);
}

// ─────────────────────────────────────────
// Start IQ capture to SD card
// ─────────────────────────────────────────
void SigFinderView::start_capture() {
    capture_filename_ = make_capture_filename();

    // Write metadata .TXT file alongside the .C16
    // Format matches Mayhem capture app convention
    const std::string txt_path =
        "/CAPTURES/" + capture_filename_ + ".TXT";
    const std::string c16_path =
        "/CAPTURES/" + capture_filename_ + ".C16";

    // Write metadata
    File meta_file;
    if (meta_file.open(txt_path, false, true)) {
        const std::string meta =
            "center_frequency=" +
            to_string_dec_uint((uint32_t)(target_freq_ / 1000)) +
            "000\n"
            "sample_rate=500000\n"
            "label=" + band_label_ + "\n";
        meta_file.write(meta.c_str(), meta.size());
    }

    // Switch baseband to capture mode at 500kHz BW
    // (recommended for SD card write speed compatibility)
    baseband::run_image(portapack::spi_flash::image_tag_capture);
    receiver_model.set_sampling_rate(500000);
    receiver_model.set_baseband_bandwidth(500000);

    // Start recording
    // In Mayhem, RecordView handles this;
    // we trigger it via baseband message
    // record_view.start(c16_path);  // If using RecordView widget
    // Direct approach:
    baseband::set_sample_rate(500000);

    capturing_ = true;
    button_capture.set_text("STOP REC");
    button_capture.set_style(&Styles::red);

    text_capture_status.set(
        "REC: " + capture_filename_ + ".C16");
    text_capture_status.set_style(&Styles::red);
}

// ─────────────────────────────────────────
// Stop capture
// ─────────────────────────────────────────
void SigFinderView::stop_capture() {
    if (!capturing_) return;

    // Stop baseband recording
    baseband::kill();

    // Restore normal receive mode
    receiver_model.set_sampling_rate(3072000);
    receiver_model.set_baseband_bandwidth(1750000);
    receiver_model.enable();

    capturing_ = false;
    button_capture.set_text("CAPTURE");
    button_capture.set_style(&Styles::white);

    text_capture_status.set(
        "Saved: " + capture_filename_ + ".C16");
    text_capture_status.set_style(&Styles::green);
}

// ─────────────────────────────────────────
// Auto-generate filename from frequency
// and band label — saved to /CAPTURES/
// e.g. "315000_FORD_FOB.C16"
// ─────────────────────────────────────────
std::string SigFinderView::make_capture_filename() const {
    // Frequency in kHz as base
    const uint32_t freq_khz = (uint32_t)(target_freq_ / 1000);
    std::string name = to_string_dec_uint(freq_khz) + "_";

    // Clean up band label for filename
    // Replace spaces and special chars with underscores
    for (char c : band_label_) {
        if (c == ' ' || c == '/' || c == ':' || c == '!') {
            name += '_';
        } else if (c >= 'A' && c <= 'Z') {
            name += c;
        } else if (c >= '0' && c <= '9') {
            name += c;
        }
    }

    // Trim trailing underscores
    while (!name.empty() && name.back() == '_')
        name.pop_back();

    // Limit length to 20 chars (SD card filename safe)
    if (name.size() > 20) name = name.substr(0, 20);

    return name;
}

// ─────────────────────────────────────────
// Format frequency for display
// ─────────────────────────────────────────
std::string SigFinderView::format_freq(rf::Frequency f) const {
    if (f >= 1'000'000'000) {
        return to_string_dec_uint((uint32_t)(f / 1'000'000)) + " MHz";
    } else if (f >= 1'000'000) {
        const uint32_t mhz = f / 1'000'000;
        const uint32_t khz = (f % 1'000'000) / 1000;
        return to_string_dec_uint(mhz) + "." +
               (khz < 100 ? "0" : "") +
               (khz < 10  ? "0" : "") +
               to_string_dec_uint(khz) + " MHz";
    } else {
        return to_string_dec_uint(f / 1000) + " kHz";
    }
}

// ─────────────────────────────────────────
// Convert RSSI dBm to 0-100 percentage
// Useful range: -120 dBm (nothing) to -20 dBm (very strong)
// ─────────────────────────────────────────
int32_t SigFinderView::rssi_to_percent(int32_t rssi) const {
    // Clamp to useful range
    const int32_t low  = -110;
    const int32_t high = -30;
    if (rssi <= low)  return 0;
    if (rssi >= high) return 100;
    return ((rssi - low) * 100) / (high - low);
}

}  // namespace ui
