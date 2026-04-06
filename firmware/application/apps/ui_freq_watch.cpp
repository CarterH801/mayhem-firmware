/*
 * Frequency Watch / Alert
 * File: firmware/application/apps/ui_freq_watch.cpp
 */

#include "ui_freq_watch.hpp"
#include "portapack.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "baseband_api.hpp"
#include "ui_bandplan.hpp"

using namespace portapack;

namespace ui {

FreqWatchView::FreqWatchView(NavigationView& nav)
    : nav_(nav) {

    add_children({
        &label_header,
        &text_e0, &text_e1, &text_e2, &text_e3,
        &text_e4, &text_e5, &text_e6, &text_e7,
        &text_alert,
        &text_status,
        &text_scanning,
        &label_freq,
        &field_freq_mhz,
        &label_dot,
        &field_freq_khz,
        &label_mhz,
        &label_thr,
        &field_threshold,
        &button_set,
        &button_clear_entry,
        &button_watch,
        &button_back,
        &button_prev,
        &text_sel,
        &button_next,
        &label_hint,
    });

    // Pre-populate with common useful watch frequencies
    entries_[0] = {315'000'000, -70,
                   "KEY FOB 315", true, false, 0, -120};
    entries_[1] = {433'920'000, -70,
                   "KEY FOB 433", true, false, 0, -120};
    entries_[2] = {162'475'000, -80,
                   "NOAA WEATHER", true, false, 0, -120};
    entries_[3] = {121'500'000, -85,
                   "AVIAT EMERG", true, false, 0, -120};
    entries_[4] = {915'000'000, -75,
                   "DRONE 915MHz", true, false, 0, -120};
    entries_[5] = {2'402'000'000, -75,
                   "BLE ADV CH37", true, false, 0, -120};
    // entries 6 and 7 left empty for user to fill

    // Set initial field values from entry 0
    field_freq_mhz.set_value(
        (int32_t)(entries_[0].freq / 1'000'000));
    field_freq_khz.set_value(
        (int32_t)((entries_[0].freq % 1'000'000) / 1000));
    field_threshold.set_value(entries_[0].threshold_dbm);

    update_all_rows();

    // ── Button handlers ───────────────────────────────────

    button_prev.on_select = [this](Button&) {
        selected_entry_ =
            (selected_entry_ - 1 + MAX_WATCH) % MAX_WATCH;
        text_sel.set(
            "#" + to_string_dec_uint(selected_entry_ + 1));
        const auto& e = entries_[selected_entry_];
        field_freq_mhz.set_value(
            (int32_t)(e.freq / 1'000'000));
        field_freq_khz.set_value(
            (int32_t)((e.freq % 1'000'000) / 1000));
        field_threshold.set_value(e.threshold_dbm);
    };

    button_next.on_select = [this](Button&) {
        selected_entry_ = (selected_entry_ + 1) % MAX_WATCH;
        text_sel.set(
            "#" + to_string_dec_uint(selected_entry_ + 1));
        const auto& e = entries_[selected_entry_];
        field_freq_mhz.set_value(
            (int32_t)(e.freq / 1'000'000));
        field_freq_khz.set_value(
            (int32_t)((e.freq % 1'000'000) / 1000));
        field_threshold.set_value(e.threshold_dbm);
    };

    button_set.on_select = [this](Button&) {
        auto& e = entries_[selected_entry_];
        e.freq =
            (rf::Frequency)field_freq_mhz.value() *
            1'000'000 +
            (rf::Frequency)field_freq_khz.value() * 1000;
        e.threshold_dbm = field_threshold.value();
        e.label = bandplan::get_band_label(e.freq);
        // Truncate label
        if (e.label.size() > 12)
            e.label = e.label.substr(0, 12);
        e.enabled = true;
        e.trigger_count = 0;
        update_all_rows();
        text_status.set("Slot #" +
            to_string_dec_uint(selected_entry_ + 1) +
            " set.");
    };

    button_clear_entry.on_select = [this](Button&) {
        entries_[selected_entry_] = WatchEntry{};
        update_all_rows();
        text_status.set("Slot #" +
            to_string_dec_uint(selected_entry_ + 1) +
            " cleared.");
    };

    button_watch.on_select = [this](Button&) {
        if (!watching_) {
            start_watch();
        } else {
            stop_watch();
        }
    };

    button_back.on_select = [this](Button&) {
        stop_watch();
        nav_.pop();
    };
}

FreqWatchView::~FreqWatchView() {
    stop_watch();
}

void FreqWatchView::focus() {
    button_watch.focus();
}

// ─────────────────────────────────────────
// Start watching
// ─────────────────────────────────────────
void FreqWatchView::start_watch() {
    watching_          = true;
    current_scan_idx_  = 0;
    dwell_counter_     = 0;
    button_watch.set_text("STOP");
    button_watch.set_style(ui::Theme::getInstance()->fg_light);
    text_status.set("Watching...");

    // Set up receiver
    receiver_model.set_modulation(
        ReceiverModel::Mode::NarrowbandFMAudio);
    receiver_model.set_sampling_rate(3072000);
    receiver_model.set_baseband_bandwidth(1750000);
    receiver_model.enable();

    tune_to_entry(current_scan_idx_);
}

// ─────────────────────────────────────────
// Stop watching
// ─────────────────────────────────────────
void FreqWatchView::stop_watch() {
    if (!watching_) return;
    watching_ = false;
    clear_alert();
    receiver_model.disable();
    button_watch.set_text("WATCH");
    button_watch.set_style(ui::Theme::getInstance()->fg_light);
    text_status.set("Stopped. Press WATCH to resume.");
    text_scanning.set("");
}

// ─────────────────────────────────────────
// Tune to a specific watch entry
// ─────────────────────────────────────────
void FreqWatchView::tune_to_entry(int idx) {
    if (idx >= MAX_WATCH) return;
    const auto& e = entries_[idx];
    if (e.freq == 0 || !e.enabled) {
        advance_scan();
        return;
    }

    receiver_model.set_target_frequency(e.freq);

    // Set appropriate modulation
    if (e.freq >= 87'500'000 && e.freq <= 108'000'000) {
        receiver_model.set_modulation(
            ReceiverModel::Mode::WidebandFMAudio);
    } else if (e.freq < 30'000'000) {
        receiver_model.set_modulation(
            ReceiverModel::Mode::AMAudio);
    } else {
        receiver_model.set_modulation(
            ReceiverModel::Mode::NarrowbandFMAudio);
    }

    text_scanning.set(
        "Checking: " + format_freq(e.freq) +
        " [" + e.label + "]");
}

// ─────────────────────────────────────────
// Advance to next enabled entry
// ─────────────────────────────────────────
void FreqWatchView::advance_scan() {
    // Find next enabled entry
    for (int i = 1; i <= MAX_WATCH; i++) {
        const int next =
            (current_scan_idx_ + i) % MAX_WATCH;
        if (entries_[next].freq > 0 &&
            entries_[next].enabled) {
            current_scan_idx_ = next;
            dwell_counter_ = 0;
            tune_to_entry(current_scan_idx_);
            return;
        }
    }
}

// ─────────────────────────────────────────
// RSSI handler — called every 100ms
// ─────────────────────────────────────────
void FreqWatchView::on_statistics_update(
    const RSSIStatisticsMessage& message) {

    if (!watching_) return;

    const int32_t rssi = message.statistics.max;

    // Update last RSSI for current entry
    entries_[current_scan_idx_].last_rssi = rssi;

    // Check against threshold
    check_and_alert(current_scan_idx_, rssi);

    // Dwell for DWELL_TICKS readings then move on
    dwell_counter_++;
    if (dwell_counter_ >= DWELL_TICKS) {
        // Reset triggered state if signal dropped
        if (rssi < entries_[current_scan_idx_].threshold_dbm) {
            entries_[current_scan_idx_].triggered = false;
        }
        dwell_counter_ = 0;
        advance_scan();
    }

    // Update row display for current entry
    draw_entry_row(current_scan_idx_);

    // Flash alert if active
    if (alert_active_) {
        alert_flash_counter_++;
        if (alert_flash_counter_ % 3 == 0) {
            text_alert.set_style(
                alert_flash_counter_ % 6 < 3
                ? ui::Theme::getInstance()->fg_light
                : ui::Theme::getInstance()->fg_light);
        }
    }
}

// ─────────────────────────────────────────
// Check RSSI against threshold and alert
// ─────────────────────────────────────────
void FreqWatchView::check_and_alert(
    int entry_idx, int32_t rssi) {

    auto& e = entries_[entry_idx];
    if (e.freq == 0 || !e.enabled) return;

    if (rssi > e.threshold_dbm && !e.triggered) {
        e.triggered = true;
        e.trigger_count++;
        trigger_alert(entry_idx);
    }
}

// ─────────────────────────────────────────
// Trigger alert for an entry
// ─────────────────────────────────────────
void FreqWatchView::trigger_alert(int entry_idx) {
    alert_active_ = true;
    alert_entry_  = entry_idx;
    alert_flash_counter_ = 0;

    const auto& e = entries_[entry_idx];
    text_alert.set(
        "!! ALERT: " + format_freq(e.freq) +
        " [" + e.label + "] " +
        to_string_dec_int(e.last_rssi) + "dBm !!");
    text_alert.set_style(ui::Theme::getInstance()->fg_light);

    // Beep alert
    baseband::set_beep(880, 200);  // 880Hz for 200ms

    text_status.set(
        "#" + to_string_dec_uint(entry_idx + 1) +
        " triggered x" +
        to_string_dec_uint(e.trigger_count));
}

void FreqWatchView::clear_alert() {
    alert_active_ = false;
    alert_entry_  = -1;
    text_alert.set("");
}

// ─────────────────────────────────────────
// Draw one entry row
// Format: >1 315.000M  KEY FOB 315  -70  -68  x3
// ─────────────────────────────────────────
void FreqWatchView::draw_entry_row(int idx) {
    Text* rows[8] = {
        &text_e0, &text_e1, &text_e2, &text_e3,
        &text_e4, &text_e5, &text_e6, &text_e7};

    const auto& e = entries_[idx];

    if (e.freq == 0) {
        rows[idx]->set(" " +
            to_string_dec_uint(idx + 1) +
            " [empty slot]");
        rows[idx]->set_style(ui::Theme::getInstance()->fg_light);
        return;
    }

    // Selector
    const std::string sel =
        (idx == selected_entry_) ? ">" : " ";

    // Frequency — 8 chars
    std::string fs = format_freq(e.freq);
    while (fs.size() < 8) fs += " ";

    // Label — 12 chars
    std::string lbl = e.label;
    while (lbl.size() < 12) lbl += " ";
    if (lbl.size() > 12) lbl = lbl.substr(0, 12);

    // Threshold
    std::string thr =
        to_string_dec_int(e.threshold_dbm);
    while (thr.size() < 4) thr = " " + thr;

    // Last RSSI
    std::string rssi =
        to_string_dec_int(e.last_rssi);
    while (rssi.size() < 4) rssi = " " + rssi;

    // Trigger count
    const std::string cnt =
        "x" + to_string_dec_uint(e.trigger_count);

    const std::string row =
        sel +
        to_string_dec_uint(idx + 1) + " " +
        fs + " " + lbl + " " +
        thr + " " + rssi + " " + cnt;

    rows[idx]->set(row);

    // Color by state
    if (e.triggered) {
        rows[idx]->set_style(ui::Theme::getInstance()->fg_light);
    } else if (!e.enabled) {
        rows[idx]->set_style(ui::Theme::getInstance()->fg_light);
    } else if (idx == current_scan_idx_ && watching_) {
        rows[idx]->set_style(ui::Theme::getInstance()->fg_light);
    } else {
        rows[idx]->set_style(ui::Theme::getInstance()->fg_light);
    }
}

void FreqWatchView::update_all_rows() {
    for (int i = 0; i < MAX_WATCH; i++)
        draw_entry_row(i);
}

std::string FreqWatchView::format_freq(
    rf::Frequency f) const {

    if (f >= 1'000'000'000) {
        const uint32_t ghz = f / 1'000'000'000;
        const uint32_t mhz =
            (f % 1'000'000'000) / 1'000'000;
        return to_string_dec_uint(ghz) + "." +
               (mhz < 100 ? "0" : "") +
               (mhz < 10  ? "0" : "") +
               to_string_dec_uint(mhz) + "G";
    } else if (f >= 1'000'000) {
        const uint32_t mhz = f / 1'000'000;
        const uint32_t khz = (f % 1'000'000) / 1000;
        return to_string_dec_uint(mhz) + "." +
               (khz < 100 ? "0" : "") +
               (khz < 10  ? "0" : "") +
               to_string_dec_uint(khz) + "M";
    }
    return to_string_dec_uint(f / 1000) + "k";
}

}  // namespace ui
