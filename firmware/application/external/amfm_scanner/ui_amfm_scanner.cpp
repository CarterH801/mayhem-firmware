/*
 * AM/FM/Drone Band Scanner — v3
 * File: firmware/application/apps/ui_amfm_scanner.cpp
 *
 * Scan modes: FM / AM / BOTH / KFOB / DRONE
 */

#include "ui_amfm_scanner.hpp"
#include "portapack.hpp"
#include "receiver_model.hpp"
#include "baseband_api.hpp"
#include "string_format.hpp"
#include "ui_bandplan.hpp"

using namespace portapack;

namespace ui::external_app::amfm_scanner {

// ─────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────
AMFMScannerView::AMFMScannerView(NavigationView& nav)
    : nav_(nav) {

    add_children({
        &options_mode,
        &label_sq,
        &field_squelch,
        &button_scan,
        &text_status,
        &progress_bar,
        &label_legend,
        &label_legend2,
        &label_headers,
        &text_s0, &text_s1, &text_s2, &text_s3,
        &text_s4, &text_s5, &text_s6, &text_s7,
        &text_scroll,
        &button_tune,
        &button_up,
        &button_down,
        &button_clear,
        &button_back,
    });

    field_squelch.set_value(squelch_db_);
    field_squelch.on_change = [this](int32_t v) {
        squelch_db_ = v;
    };

    button_scan.on_select = [this](Button&) {
        if (scan_state_ == ScanState::IDLE ||
            scan_state_ == ScanState::DONE) {
            start_scan();
        } else {
            stop_scan();
        }
    };

    button_tune.on_select = [this](Button&) {
        if (selected_station_ >= 0 &&
            selected_station_ < (int16_t)found_stations_.size()) {
            tune_to(found_stations_[selected_station_].frequency);
        }
    };

    button_up.on_select = [this](Button&) {
        scroll_list(-1);
    };
    button_down.on_select = [this](Button&) {
        scroll_list(1);
    };

    button_clear.on_select = [this](Button&) {
        found_stations_.clear();
        list_offset_      = 0;
        selected_station_ = -1;
        draw_station_list();
        text_status.set("List cleared.");
    };

    button_back.on_select = [this](Button&) {
        stop_scan();
        nav_.pop();
    };
}

AMFMScannerView::~AMFMScannerView() {
    stop_scan();
}

void AMFMScannerView::focus() {
    button_scan.focus();
}

// ─────────────────────────────────────────
// Start scan — choose band based on mode
// ─────────────────────────────────────────
void AMFMScannerView::start_scan() {
    found_stations_.clear();
    list_offset_      = 0;
    selected_station_ = -1;
    noise_floor_db_   = -100;
    noise_samples_    = 0;
    noise_sum_        = 0;
    confirm_count_    = 0;

    scan_mode_ = options_mode.selected_index();

    switch (scan_mode_) {
        case 0:  // FM
            scan_start_ = FM_START;
            scan_end_   = FM_END;
            scan_step_  = FM_STEP;
            break;
        case 1:  // AM
            scan_start_ = AM_START;
            scan_end_   = AM_END;
            scan_step_  = AM_STEP;
            break;
        case 2:  // BOTH — start with FM
            scan_start_ = FM_START;
            scan_end_   = FM_END;
            scan_step_  = FM_STEP;
            break;
        case 3:  // KFOB — disabled (feature removed)
        case 4:  // DRONE — disabled (feature removed)
            scan_start_ = FM_START;
            scan_end_   = FM_END;
            scan_step_  = FM_STEP;
            break;
        case 5:  // BT — scan all 3 BLE advertising channels
            // Start at BLE adv ch.37 = 2402 MHz
            scan_start_ = 2'402'000'000;
            scan_end_   = 2'402'000'000;
            scan_step_  = 500'000;
            break;
    }

    current_freq_ = scan_start_;
    scan_state_   = ScanState::CALIBRATING;

    button_scan.set_text("STOP");
    text_status.set("Phase 1: Measuring noise floor...");
    progress_bar.set_value(0);

    set_modulation_for_band();
    tune_to(current_freq_);
    draw_station_list();
}

// ─────────────────────────────────────────
// Stop scan
// ─────────────────────────────────────────
void AMFMScannerView::stop_scan() {
    if (scan_state_ != ScanState::IDLE) {
        scan_state_ = ScanState::DONE;
        button_scan.set_text("SCAN");
        update_status_text();
        receiver_model.disable();
    }
}

// ─────────────────────────────────────────
// Set modulation for current frequency
// ─────────────────────────────────────────
void AMFMScannerView::set_modulation_for_band() {
    if (current_freq_ >= 87'500'000 &&
        current_freq_ <= 108'000'000) {
        // FM broadcast
        receiver_model.set_modulation(
            ReceiverModel::Mode::WidebandFMAudio);
        receiver_model.set_nbfm_configuration(2);
    } else if (current_freq_ < 30'000'000) {
        // AM
        receiver_model.set_modulation(
            ReceiverModel::Mode::AMAudio);
        receiver_model.set_am_configuration(0);
    } else {
        // Everything else: NFM
        // (key fobs, drones, VHF, UHF)
        receiver_model.set_modulation(
            ReceiverModel::Mode::NarrowbandFMAudio);
        receiver_model.set_nbfm_configuration(0);
    }
    receiver_model.set_sampling_rate(3072000);
    receiver_model.set_baseband_bandwidth(1750000);
    receiver_model.enable();
}

// ─────────────────────────────────────────
// Tune radio
// ─────────────────────────────────────────
void AMFMScannerView::tune_to(rf::Frequency freq) {
    receiver_model.set_target_frequency(freq);
}

// ─────────────────────────────────────────
// Scroll station list
// ─────────────────────────────────────────
void AMFMScannerView::scroll_list(int16_t delta) {
    const int16_t total = (int16_t)found_stations_.size();
    list_offset_ = std::max(
        (int16_t)0,
        std::min((int16_t)(total - 8),
                 (int16_t)(list_offset_ + delta)));
    selected_station_ = list_offset_;
    draw_station_list();
}

// ─────────────────────────────────────────
// Advance to next frequency
// ─────────────────────────────────────────
void AMFMScannerView::next_frequency() {
    current_freq_ += scan_step_;

    if (current_freq_ > scan_end_) {

        if (scan_state_ == ScanState::CALIBRATING) {
            // Calibration done
            if (noise_samples_ > 0)
                noise_floor_db_ = noise_sum_ / noise_samples_;
            current_freq_ = scan_start_;
            scan_state_   = ScanState::SCANNING;
            text_status.set("Phase 2: Scanning...");
            tune_to(current_freq_);
            return;
        }

        // ── Band transitions ──────────────────────────────

        // BOTH mode: FM done → do AM
        if (scan_mode_ == 2 && scan_start_ == FM_START) {
            scan_start_   = AM_START;
            scan_end_     = AM_END;
            scan_step_    = AM_STEP;
            current_freq_ = scan_start_;
            scan_state_   = ScanState::CALIBRATING;
            noise_samples_ = 0;
            noise_sum_     = 0;
            text_status.set("AM: Measuring noise floor...");
            set_modulation_for_band();
            tune_to(current_freq_);
            return;
        }

        // BT mode: hop through 3 BLE advertising channels
        // ch37=2402MHz → ch38=2426MHz → ch39=2480MHz
        if (scan_mode_ == 5) {
            if (scan_start_ == 2'402'000'000ULL) {
                // Move to ch38
                scan_start_   = 2'426'000'000;
                scan_end_     = 2'426'000'000;
                current_freq_ = scan_start_;
                scan_state_   = ScanState::CALIBRATING;
                noise_samples_ = 0;
                noise_sum_     = 0;
                text_status.set("BT: Scanning ch.38 (2426MHz)");
                tune_to(current_freq_);
                return;
            }
            if (scan_start_ == 2'426'000'000ULL) {
                // Move to ch39
                scan_start_   = 2'480'000'000;
                scan_end_     = 2'480'000'000;
                current_freq_ = scan_start_;
                scan_state_   = ScanState::CALIBRATING;
                noise_samples_ = 0;
                noise_sum_     = 0;
                text_status.set("BT: Scanning ch.39 (2480MHz)");
                tune_to(current_freq_);
                return;
            }
            // Also do a wide sweep of the full BT band
            if (scan_start_ == 2'480'000'000ULL) {
                scan_start_   = 2'402'000'000;
                scan_end_     = 2'480'000'000;
                scan_step_    = 2'000'000;  // 2MHz BLE spacing
                current_freq_ = scan_start_;
                scan_state_   = ScanState::CALIBRATING;
                noise_samples_ = 0;
                noise_sum_     = 0;
                text_status.set("BT: Wide sweep 2.4GHz band");
                tune_to(current_freq_);
                return;
            }
        }

        // All done
        stop_scan();
        return;
    }

    confirm_count_ = 0;
    tune_to(current_freq_);
    update_status_text();
}

// ─────────────────────────────────────────
// RSSI statistics handler
// ─────────────────────────────────────────
void AMFMScannerView::on_statistics_update(
    const RSSIStatisticsMessage& message) {

    if (scan_state_ == ScanState::IDLE ||
        scan_state_ == ScanState::DONE)
        return;

    const int32_t rssi = message.statistics.max;

    if (scan_state_ == ScanState::CALIBRATING) {
        noise_sum_ += rssi;
        noise_samples_++;
        next_frequency();
        return;
    }

    const int32_t threshold = std::max(
        squelch_db_,
        noise_floor_db_ + SIGNAL_THRESHOLD_ABOVE_NOISE);

    if (rssi > threshold) {
        confirm_count_++;
        if (confirm_count_ >= SQUELCH_SAMPLES) {
            add_station(current_freq_, rssi);
            confirm_count_ = 0;
            next_frequency();
        }
    } else {
        confirm_count_ = 0;
        next_frequency();
    }

    const int64_t total =
        (scan_end_ - scan_start_) / scan_step_;
    const int64_t done  =
        (current_freq_ - scan_start_) / scan_step_;
    if (total > 0)
        progress_bar.set_value(
            (int)((done * 100) / total));
}

// ─────────────────────────────────────────
// Add detected station with band label
// ─────────────────────────────────────────
void AMFMScannerView::add_station(
    rf::Frequency freq, int32_t rssi) {

    for (const auto& s : found_stations_) {
        if (std::abs((int64_t)s.frequency -
                     (int64_t)freq) <= scan_step_)
            return;
    }

    StationEntry entry;
    entry.frequency  = freq;
    entry.rssi_db    = rssi;
    entry.freq_str   = format_frequency(freq);
    entry.band_label = bandplan::get_band_label(freq);
    entry.category   = bandplan::get_band_category(freq);

    found_stations_.push_back(entry);
    draw_station_list();
}

// ─────────────────────────────────────────
// Format frequency
// ─────────────────────────────────────────
std::string AMFMScannerView::format_frequency(
    rf::Frequency freq) const {

    if (freq >= 1'000'000'000) {
        const uint32_t ghz = freq / 1'000'000'000;
        const uint32_t mhz = (freq % 1'000'000'000) / 1'000'000;
        return to_string_dec_uint(ghz) + "." +
               (mhz < 100 ? "0" : "") +
               (mhz < 10  ? "0" : "") +
               to_string_dec_uint(mhz) + "G";
    } else if (freq >= 1'000'000) {
        const uint32_t mhz = freq / 1'000'000;
        const uint32_t khz = (freq % 1'000'000) / 1000;
        return to_string_dec_uint(mhz) + "." +
               (khz < 100 ? "0" : "") +
               (khz < 10  ? "0" : "") +
               to_string_dec_uint(khz) + "M";
    } else {
        return to_string_dec_uint(freq / 1000) + "k";
    }
}

// ─────────────────────────────────────────
// Draw station list with color coding
// ─────────────────────────────────────────
void AMFMScannerView::draw_station_list() {
    Text* rows[8] = {
        &text_s0, &text_s1, &text_s2, &text_s3,
        &text_s4, &text_s5, &text_s6, &text_s7};

    const int16_t total = (int16_t)found_stations_.size();

    for (int i = 0; i < 8; i++) {
        const int16_t idx = list_offset_ + i;

        if (idx < total) {
            const auto& s = found_stations_[idx];

            const int32_t bar_len = std::min((int32_t)4,
                std::max((int32_t)0, (s.rssi_db + 100) / 10));
            std::string sig = "";
            for (int b = 0; b < bar_len; b++) sig += "*";
            for (int b = bar_len; b < 4; b++)  sig += ".";

            std::string label = s.band_label;
            if ((int)label.size() > 12)
                label = label.substr(0, 12);

            std::string fstr = s.freq_str;
            while ((int)fstr.size() < 8) fstr += " ";

            const std::string sel =
                (idx == selected_station_) ? ">" : " ";
            const std::string row =
                sel + fstr + "[" + sig + "] " + label;

            rows[i]->set(row);
            rows[i]->set_style(
                ui::Theme::getInstance()->fg_light);
        } else {
            rows[i]->set("");
            rows[i]->set_style(ui::Theme::getInstance()->fg_light);
        }
    }

    if (total > 8) {
        const int rem = total - list_offset_ - 8;
        text_scroll.set(
            rem > 0
            ? "v " + to_string_dec_uint(rem) + " more | " +
              to_string_dec_uint(total) + " total"
            : "-- end -- | " +
              to_string_dec_uint(total) + " total");
    } else {
        text_scroll.set(
            to_string_dec_uint(total) + " found");
    }
}

// ─────────────────────────────────────────
// Status text
// ─────────────────────────────────────────
void AMFMScannerView::update_status_text() {
    if (scan_state_ == ScanState::DONE ||
        scan_state_ == ScanState::IDLE) {
        text_status.set(
            "Done. " +
            to_string_dec_uint(
                (int)found_stations_.size()) +
            " active signals found.");
    } else if (scan_state_ == ScanState::SCANNING) {
        text_status.set(
            "Scanning: " + format_frequency(current_freq_));
    }
}

}  // namespace ui::external_app::amfm_scanner
