/*
 * Satellite Pass Detector implementation
 * File: firmware/application/apps/ui_sat_pass.cpp
 */

#include "ui_sat_pass.hpp"
#include "portapack.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "rtc_time.hpp"
#include "baseband_api.hpp"

using namespace portapack;

namespace ui {

SatPassView::SatPassView(NavigationView& nav)
    : nav_(nav) {

    // Initialize detection arrays
    detected_.fill(false);
    peak_rssi_.fill(-120);

    add_children({
        &label_header,
        &text_scanning,
        &text_alert,
        &text_scan_prog,
        &label_log_hdr,
        &text_log_0, &text_log_1, &text_log_2,
        &text_log_3, &text_log_4, &text_log_5,
        &text_log_6, &text_log_7,
        &text_stats,
        &text_freq_now,
        &button_watch,
        &button_clear,
        &button_back,
        &label_hint,
    });

    button_watch.on_select = [this](Button&) {
        if (!watching_) start_watch();
        else            stop_watch();
    };

    button_clear.on_select = [this](Button&) {
        events_.clear();
        detected_.fill(false);
        peak_rssi_.fill(-120);
        text_alert.set("");
        text_stats.set("0 passes detected");
        draw_event_log();
    };

    button_back.on_select = [this](Button&) {
        stop_watch();
        nav_.pop();
    };
}

SatPassView::~SatPassView() {
    stop_watch();
}

void SatPassView::focus() {
    button_watch.focus();
}

void SatPassView::start_watch() {
    watching_          = true;
    current_freq_idx_  = 0;
    dwell_counter_     = 0;

    button_watch.set_text("STOP");
    button_watch.set_style(ui::Theme::getInstance()->fg_light);
    text_scanning.set(
        "Scanning " +
        to_string_dec_uint(SAT_FREQ_COUNT) +
        " satellite frequencies...");

    receiver_model.set_modulation(
        ReceiverModel::Mode::NarrowbandFMAudio);
    receiver_model.set_sampling_rate(3072000);
    receiver_model.set_baseband_bandwidth(1750000);
    receiver_model.enable();

    advance_scan();
}

void SatPassView::stop_watch() {
    if (!watching_) return;
    watching_ = false;
    receiver_model.disable();
    button_watch.set_text("WATCH");
    button_watch.set_style(ui::Theme::getInstance()->fg_light);
    text_scanning.set("Stopped.");
    text_freq_now.set("");
}

void SatPassView::advance_scan() {
    // Find next enabled frequency
    for (int i = 1; i <= SAT_FREQ_COUNT; i++) {
        const int next =
            (current_freq_idx_ + i) % SAT_FREQ_COUNT;
        current_freq_idx_ = next;
        dwell_counter_    = 0;

        const auto& sf = SAT_FREQS[next];
        receiver_model.set_target_frequency(sf.freq_hz);

        text_freq_now.set(
            std::string(sf.sat_name) + " " +
            std::string(sf.description) + " " +
            format_freq(sf.freq_hz));

        // Update scan progress
        text_scan_prog.set(
            to_string_dec_uint(next + 1) + "/" +
            to_string_dec_uint(SAT_FREQ_COUNT) +
            " — " + sf.sat_name);
        return;
    }
}

void SatPassView::on_statistics_update(
    const RSSIStatisticsMessage& message) {

    if (!watching_) return;

    const int32_t rssi = message.statistics.max;
    const auto&   sf   = SAT_FREQS[current_freq_idx_];

    check_detection(current_freq_idx_, rssi);

    // Update peak for this frequency
    if (rssi > peak_rssi_[current_freq_idx_])
        peak_rssi_[current_freq_idx_] = rssi;

    dwell_counter_++;
    if (dwell_counter_ >= DWELL_TICKS) {
        // If signal dropped, clear detection
        if (rssi < sf.threshold_dbm &&
            detected_[current_freq_idx_]) {
            detected_[current_freq_idx_] = false;
        }
        advance_scan();
    }
}

void SatPassView::check_detection(
    int freq_idx, int32_t rssi) {

    const auto& sf = SAT_FREQS[freq_idx];

    if (rssi > sf.threshold_dbm &&
        !detected_[freq_idx]) {
        detected_[freq_idx] = true;
        trigger_pass(freq_idx, rssi);
    }
}

void SatPassView::trigger_pass(
    int freq_idx, int32_t rssi) {

    const auto& sf = SAT_FREQS[freq_idx];

    // Beep alert — two tones
    baseband::set_beep(1200, 150);

    // Show alert banner
    text_alert.set(
        "!! PASS: " +
        std::string(sf.sat_name) + " " +
        std::string(sf.description) + " " +
        to_string_dec_int(rssi) + "dBm !!");
    text_alert.set_style(ui::Theme::getInstance()->fg_light);

    // Log the event
    PassEvent evt;
    evt.sat_name    = sf.sat_name;
    evt.description = sf.description;
    evt.freq_hz     = sf.freq_hz;
    evt.peak_rssi   = rssi;
    evt.active      = true;

    // Prepend to events list (newest first)
    events_.insert(events_.begin(), evt);
    if ((int)events_.size() > MAX_EVENTS)
        events_.pop_back();

    text_stats.set(
        to_string_dec_uint((uint32_t)events_.size()) +
        " pass(es) detected");

    draw_event_log();
}

void SatPassView::draw_event_log() {
    Text* rows[8] = {
        &text_log_0, &text_log_1, &text_log_2,
        &text_log_3, &text_log_4, &text_log_5,
        &text_log_6, &text_log_7};

    for (int i = 0; i < 8; i++) {
        if (i < (int)events_.size()) {
            const auto& e = events_[i];

            // Pad satellite name to 10 chars
            std::string name = e.sat_name;
            while (name.size() < 10) name += " ";

            // Format frequency
            std::string freq = format_freq(e.freq_hz);
            while (freq.size() < 9) freq += " ";

            const std::string row =
                "--:--:--  " +
                name + "  " +
                freq + "  " +
                to_string_dec_int(e.peak_rssi) +
                "dBm";

            rows[i]->set(row);
            rows[i]->set_style(ui::Theme::getInstance()->fg_light);
        } else {
            rows[i]->set("");
        }
    }
}

std::string SatPassView::format_freq(uint64_t f) const {
    if (f >= 1'000'000'000) {
        return to_string_dec_uint(
            (uint32_t)(f / 1'000'000)) + "MHz";
    } else if (f >= 1'000'000) {
        const uint32_t mhz = f / 1'000'000;
        const uint32_t khz = (f % 1'000'000) / 1000;
        return to_string_dec_uint(mhz) + "." +
               (khz < 100 ? "0" : "") +
               (khz < 10  ? "0" : "") +
               to_string_dec_uint(khz) + "MHz";
    }
    return to_string_dec_uint(f / 1000) + "kHz";
}

std::string SatPassView::format_time(
    uint32_t t) const {
    const uint32_t h = t / 3600;
    const uint32_t m = (t % 3600) / 60;
    const uint32_t s = t % 60;
    return (h < 10 ? "0" : "") +
           to_string_dec_uint(h) + ":" +
           (m < 10 ? "0" : "") +
           to_string_dec_uint(m) + ":" +
           (s < 10 ? "0" : "") +
           to_string_dec_uint(s);
}

}  // namespace ui
