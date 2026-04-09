/*
 * Modulation Identifier implementation
 * File: firmware/application/apps/ui_mod_ident.cpp
 */

#include "ui_mod_ident.hpp"
#include "portapack.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include <cmath>

using namespace portapack;

namespace ui {

ModIdentView::ModIdentView(NavigationView& nav)
    : nav_(nav) {
    init(315'000'000, "KEY FOB 315MHz");
}

ModIdentView::ModIdentView(NavigationView& nav,
                           rf::Frequency freq,
                           const std::string& label)
    : nav_(nav) {
    init(freq, label);
}

ModIdentView::~ModIdentView() {
    receiver_model.disable();
}

void ModIdentView::focus() {
    button_analyze.focus();
}

void ModIdentView::init(rf::Frequency freq,
                        const std::string& label) {
    target_freq_ = freq;
    band_label_  = label;

    add_children({
        &text_freq, &text_band,
        &label_progress, &bar_progress,
        &label_result, &text_mod_type,
        &label_conf, &text_confidence,
        &bar_confidence,
        &label_desc, &text_description,
        &label_device, &text_device,
        &label_stats_h, &text_stats,
        &button_analyze, &button_back,
    });

    text_freq.set(format_freq(target_freq_));
    text_band.set(band_label_);
    text_band.set_style(
        ui::Theme::getInstance()->fg_light);

    button_analyze.on_select = [this](Button&) {
        // Reset and start new collection
        sample_idx_   = 0;
        samples_full_ = false;
        analyzing_    = true;
        result_       = ModResult{};
        text_mod_type.set("Collecting...");
        text_confidence.set("");
        text_description.set("");
        text_device.set("");
        bar_progress.set_value(0);

        receiver_model.set_target_frequency(target_freq_);
        receiver_model.set_modulation(
            ReceiverModel::Mode::NarrowbandFMAudio);
        receiver_model.set_sampling_rate(3072000);
        receiver_model.set_baseband_bandwidth(1750000);
        receiver_model.enable();
    };

    button_back.on_select = [this](Button&) {
        receiver_model.disable();
        nav_.pop();
    };
}

void ModIdentView::on_statistics_update(
    const RSSIStatisticsMessage& message) {

    if (!analyzing_) return;

    collect_sample(message.statistics.max);
}

void ModIdentView::collect_sample(int32_t rssi) {
    if (samples_full_) return;

    rssi_samples_[sample_idx_++] = rssi;
    bar_progress.set_value(
        (sample_idx_ * 100) / SAMPLE_COUNT);

    if (sample_idx_ >= SAMPLE_COUNT) {
        samples_full_ = true;
        analyzing_    = false;
        result_       = analyze_samples();
        update_display();
        receiver_model.disable();
    }
}

// ─────────────────────────────────────────
// The core analysis algorithm
//
// Uses 3 key metrics:
// 1. Amplitude variance — high = FM/FSK, low = CW/carrier
// 2. Zero crossing rate — fast = high freq deviation
// 3. AM modulation index — ratio of variation to carrier
// ─────────────────────────────────────────
ModResult ModIdentView::analyze_samples() {
    ModResult r;

    const float mean = calc_mean(
        rssi_samples_.data(), SAMPLE_COUNT);
    const float variance = calc_variance(
        rssi_samples_.data(), SAMPLE_COUNT, mean);
    const int zc = calc_zero_crossings(
        rssi_samples_.data(), SAMPLE_COUNT, mean);
    const float am_idx = calc_am_index(
        rssi_samples_.data(), SAMPLE_COUNT);

    // Standard deviation
    const float stddev = sqrtf(variance);

    // ── Classification logic ──────────────────────────────

    // Very low variance — no modulation or pure carrier
    if (stddev < 2.0f) {
        r.mod_type      = "CW / CARRIER";
        r.description   = "Continuous unmodulated carrier";
        r.likely_device = "Beacon, test tone, or jammer";
        r.confidence_pct = 70;
        r.confidence    = "MEDIUM";
        // Build stats string
        text_stats.set(
            "Var:" + to_string_dec_uint((uint32_t)variance) +
            " ZC:" + to_string_dec_uint(zc) +
            " AM:" + to_string_dec_uint((uint32_t)(am_idx*100)));
        return r;
    }

    // OOK — On/Off Keying
    // Very bimodal — RSSI flips between ON and OFF
    // High variance but only 2 distinct levels
    if (stddev > 8.0f && am_idx > 0.7f && zc < 20) {
        r.mod_type      = "OOK";
        r.description   =
            "On-Off Keying — digital data bursts";
        r.likely_device =
            "Garage door, alarm sensor, weather station";
        r.confidence_pct = 80;
        r.confidence    = "HIGH";
        text_stats.set(
            "Var:" + to_string_dec_uint((uint32_t)variance) +
            " ZC:" + to_string_dec_uint(zc) +
            " AM:" + to_string_dec_uint((uint32_t)(am_idx*100)));
        return r;
    }

    // AM — Amplitude Modulation
    // Moderate variance, amplitude varies smoothly
    if (am_idx > 0.3f && am_idx < 0.7f && zc < 40) {
        r.mod_type      = "AM";
        r.description   =
            "Amplitude Modulation — voice or data";
        r.likely_device =
            "AM broadcast, aviation ATC, aircraft comms";
        r.confidence_pct = 75;
        r.confidence    = "MEDIUM-HIGH";
        text_stats.set(
            "Var:" + to_string_dec_uint((uint32_t)variance) +
            " ZC:" + to_string_dec_uint(zc) +
            " AM:" + to_string_dec_uint((uint32_t)(am_idx*100)));
        return r;
    }

    // NFM — Narrow FM
    // Moderate variance, higher zero crossing rate
    if (stddev > 2.0f && stddev < 8.0f &&
        zc >= 20 && zc < 60) {

        // Check frequency range for better labeling
        if (target_freq_ >= 136'000'000 &&
            target_freq_ <= 174'000'000) {
            r.likely_device =
                "Police, fire, EMS, business radio";
        } else if (target_freq_ >= 400'000'000 &&
                   target_freq_ <= 512'000'000) {
            r.likely_device =
                "UHF public safety, business, GMRS";
        } else {
            r.likely_device =
                "Land mobile, ham radio, repeater";
        }
        r.mod_type      = "NFM";
        r.description   =
            "Narrow FM — 12.5kHz or 25kHz voice";
        r.confidence_pct = 80;
        r.confidence    = "HIGH";
        text_stats.set(
            "Var:" + to_string_dec_uint((uint32_t)variance) +
            " ZC:" + to_string_dec_uint(zc) +
            " AM:" + to_string_dec_uint((uint32_t)(am_idx*100)));
        return r;
    }

    // WFM — Wideband FM
    // Higher variance, faster zero crossings
    if (stddev >= 8.0f && zc >= 40) {
        r.mod_type      = "WFM";
        r.description   = "Wideband FM — 200kHz broadcast";
        r.likely_device = "FM broadcast radio station";
        r.confidence_pct = 85;
        r.confidence    = "HIGH";
        text_stats.set(
            "Var:" + to_string_dec_uint((uint32_t)variance) +
            " ZC:" + to_string_dec_uint(zc) +
            " AM:" + to_string_dec_uint((uint32_t)(am_idx*100)));
        return r;
    }

    // FSK — Frequency Shift Keying
    // Bimodal in frequency domain
    // Two dominant RSSI clusters
    if (stddev > 3.0f && zc >= 15 && zc < 40 &&
        am_idx < 0.3f) {
        r.mod_type      = "FSK / GFSK";
        r.description   =
            "Frequency Shift Keying — digital data";
        r.likely_device =
            "Key fob, TPMS sensor, IoT device, drone RC";
        r.confidence_pct = 72;
        r.confidence    = "MEDIUM";
        text_stats.set(
            "Var:" + to_string_dec_uint((uint32_t)variance) +
            " ZC:" + to_string_dec_uint(zc) +
            " AM:" + to_string_dec_uint((uint32_t)(am_idx*100)));
        return r;
    }

    // Unknown
    r.mod_type      = "UNKNOWN";
    r.description   = "Could not classify modulation";
    r.likely_device = "Try adjusting frequency slightly";
    r.confidence_pct = 20;
    r.confidence    = "LOW";
    text_stats.set(
        "Var:" + to_string_dec_uint((uint32_t)variance) +
        " ZC:" + to_string_dec_uint(zc) +
        " AM:" + to_string_dec_uint((uint32_t)(am_idx*100)));
    return r;
}

void ModIdentView::update_display() {
    text_mod_type.set(result_.mod_type);
    text_confidence.set(result_.confidence + " (" +
        to_string_dec_uint(result_.confidence_pct) + "%)");
    text_description.set(result_.description);
    text_device.set(result_.likely_device);
    bar_confidence.set_value(result_.confidence_pct);

    // Color by confidence
    if (result_.confidence_pct >= 75) {
        text_mod_type.set_style(ui::Theme::getInstance()->fg_light);
    } else if (result_.confidence_pct >= 50) {
        text_mod_type.set_style(ui::Theme::getInstance()->fg_light);
    } else {
        text_mod_type.set_style(ui::Theme::getInstance()->fg_light);
    }
}

float ModIdentView::calc_mean(
    const int32_t* data, int n) const {
    float sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    return sum / n;
}

float ModIdentView::calc_variance(
    const int32_t* data, int n, float mean) const {
    float sum = 0;
    for (int i = 0; i < n; i++) {
        const float d = data[i] - mean;
        sum += d * d;
    }
    return sum / n;
}

int ModIdentView::calc_zero_crossings(
    const int32_t* data, int n, float mean) const {
    int count = 0;
    bool above = data[0] > mean;
    for (int i = 1; i < n; i++) {
        const bool now_above = data[i] > mean;
        if (now_above != above) {
            count++;
            above = now_above;
        }
    }
    return count;
}

float ModIdentView::calc_am_index(
    const int32_t* data, int n) const {
    int32_t min_v = data[0], max_v = data[0];
    for (int i = 1; i < n; i++) {
        if (data[i] < min_v) min_v = data[i];
        if (data[i] > max_v) max_v = data[i];
    }
    const float range = (float)(max_v - min_v);
    const float sum_f = (float)(max_v + min_v);
    if (sum_f <= 0) return 0;
    return range / sum_f;
}

std::string ModIdentView::format_freq(
    rf::Frequency f) const {
    if (f >= 1'000'000'000) {
        return to_string_dec_uint(
            (uint32_t)(f / 1'000'000)) + " MHz";
    } else if (f >= 1'000'000) {
        const uint32_t mhz = f / 1'000'000;
        const uint32_t khz = (f % 1'000'000) / 1000;
        return to_string_dec_uint(mhz) + "." +
               (khz < 100 ? "0" : "") +
               (khz < 10  ? "0" : "") +
               to_string_dec_uint(khz) + " MHz";
    }
    return to_string_dec_uint(f / 1000) + " kHz";
}

}  // namespace ui
