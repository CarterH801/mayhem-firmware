/*
 * Range Estimator — PortaPack Mayhem
 * File: firmware/application/apps/ui_range_est.cpp
 *
 * Free Space Path Loss formula:
 *   FSPL(dB) = 20log10(d) + 20log10(f) + 32.44
 *
 * Solving for distance in km:
 *   d(km) = 10 ^ ((TxPow - RxRSSI - 20log10(f_MHz) - 32.44) / 20)
 *
 * Converted to meters: d_m = d_km * 1000
 * Converted to miles:  d_mi = d_m * 0.000621371
 *
 * Place in: firmware/application/apps/
 */

#include "ui_range_est.hpp"
#include "portapack.hpp"
#include "receiver_model.hpp"
#include "baseband_api.hpp"
#include "string_format.hpp"
#include "ui_bandplan.hpp"
#include <cmath>

using namespace portapack;

namespace ui::external_app::range_est {

// ─────────────────────────────────────────
// Standalone constructor
// ─────────────────────────────────────────
RangeEstView::RangeEstView(NavigationView& nav)
    : nav_(nav) {
    init(315'000'000, "KEY FOB 315MHz");
}

// ─────────────────────────────────────────
// Pre-tuned constructor (from scanner/finder)
// ─────────────────────────────────────────
RangeEstView::RangeEstView(NavigationView& nav,
                           rf::Frequency  freq,
                           const std::string& label)
    : nav_(nav) {
    init(freq, label);
}

// ─────────────────────────────────────────
// Destructor
// ─────────────────────────────────────────
RangeEstView::~RangeEstView() {
    receiver_model.disable();
    baseband::shutdown();
}

void RangeEstView::focus() {
    options_device.focus();
}

// ─────────────────────────────────────────
// Shared init
// ─────────────────────────────────────────
void RangeEstView::init(rf::Frequency freq,
                        const std::string& label) {
    target_freq_ = freq;
    band_label_  = label;

    // Zero the smoothing buffer
    for (int i = 0; i < SMOOTH_LEN; i++)
        rssi_buf_[i] = -120;

    add_children({
        &text_freq,
        &text_band,
        &label_device,
        &options_device,
        &label_txpow,
        &field_txpower,
        &label_dbm,
        &label_rssi_l,
        &text_rssi,
        &label_div,
        &label_mi_unit,
        &text_miles,
        &label_m_unit,
        &text_meters,
        &label_conf_l,
        &bar_confidence,
        &text_conf_pct,
        &text_accuracy,
        &text_live,
        &button_smooth,
        &button_back,
        &button_reset,
    });

    // Set static display
    text_freq.set(format_freq(target_freq_));
    text_band.set(band_label_);
    text_band.set_style(
        ui::Theme::getInstance()->fg_light);

    // Default TX power from first preset
    tx_power_dbm_ = DEVICE_PRESETS[0].tx_power;
    field_txpower.set_value(tx_power_dbm_);

    // Minimal baseband switch — disable/shutdown first so run_image is safe.
    receiver_model.disable();
    baseband::shutdown();

    if (target_freq_ >= 87'500'000 &&
        target_freq_ <= 108'000'000) {
        baseband::run_image(portapack::spi_flash::image_tag_wfm_audio);
        receiver_model.set_modulation(
            ReceiverModel::Mode::WidebandFMAudio);
    } else if (target_freq_ < 30'000'000) {
        baseband::run_image(portapack::spi_flash::image_tag_am_audio);
        receiver_model.set_modulation(
            ReceiverModel::Mode::AMAudio);
    } else {
        baseband::run_image(portapack::spi_flash::image_tag_nfm_audio);
        receiver_model.set_modulation(
            ReceiverModel::Mode::NarrowbandFMAudio);
    }

    // Tune radio
    receiver_model.set_target_frequency(target_freq_);
    receiver_model.set_sampling_rate(3072000);
    receiver_model.set_baseband_bandwidth(1750000);
    receiver_model.enable();

    // ── Button / field handlers ───────────────────────────

    options_device.on_change = [this](size_t idx,
                                      OptionsField::value_t) {
        update_device_preset((int)idx);
    };

    field_txpower.on_change = [this](int32_t v) {
        tx_power_dbm_ = v;
    };

    button_smooth.on_select = [this](Button&) {
        smoothing_enabled_ = !smoothing_enabled_;
        button_smooth.set_text(
            smoothing_enabled_ ? "SMOOTH:ON" : "SMOOTH:OFF");
    };

    button_reset.on_select = [this](Button&) {
        for (int i = 0; i < SMOOTH_LEN; i++)
            rssi_buf_[i] = -120;
        rssi_buf_idx_  = 0;
        rssi_buf_full_ = false;
        current_rssi_  = -120;
        text_live.set("Reset. Waiting for signal...");
        text_miles.set("---");
        text_meters.set("---");
    };

    button_back.on_select = [this](Button&) {
        receiver_model.disable();
        nav_.pop();
    };
}

// ─────────────────────────────────────────
// Load preset when device type changes
// ─────────────────────────────────────────
void RangeEstView::update_device_preset(int idx) {
    if (idx < 0 || idx >= DEVICE_PRESET_COUNT) return;

    const auto& p = DEVICE_PRESETS[idx];
    tx_power_dbm_ = p.tx_power;
    field_txpower.set_value(tx_power_dbm_);

    // If preset has a frequency hint, suggest it
    if (p.freq_hint > 0 && p.freq_hint != target_freq_) {
        // Don't auto-retune — just note it
        // User can manually change freq in Signal Finder
        text_accuracy.set(
            std::string("Typical freq: ") +
            format_freq(p.freq_hint));
    } else {
        text_accuracy.set("Open air line-of-sight only");
    }
}

// ─────────────────────────────────────────
// RSSI update handler
// ─────────────────────────────────────────
void RangeEstView::on_statistics_update(
    const RSSIStatisticsMessage& message) {

    const int32_t raw = message.statistics.max;

    // Feed smoothing buffer
    rssi_buf_[rssi_buf_idx_] = raw;
    rssi_buf_idx_ = (rssi_buf_idx_ + 1) % SMOOTH_LEN;
    if (rssi_buf_idx_ == 0) rssi_buf_full_ = true;

    current_rssi_ = smoothing_enabled_
                        ? smoothed_rssi()
                        : raw;

    update_display();
}

// ─────────────────────────────────────────
// Rolling average RSSI
// ─────────────────────────────────────────
int32_t RangeEstView::smoothed_rssi() const {
    const int count = rssi_buf_full_ ? SMOOTH_LEN
                                     : rssi_buf_idx_;
    if (count == 0) return -120;
    int32_t sum = 0;
    for (int i = 0; i < count; i++)
        sum += rssi_buf_[i];
    return sum / count;
}

// ─────────────────────────────────────────
// Update all display elements
// ─────────────────────────────────────────
void RangeEstView::update_display() {
    // Raw RSSI display
    text_rssi.set(
        to_string_dec_int(current_rssi_) + " dBm");

    // Color RSSI by strength
    if      (current_rssi_ > -60)
        text_rssi.set_style(ui::Theme::getInstance()->fg_light);
    else if (current_rssi_ > -80)
        text_rssi.set_style(ui::Theme::getInstance()->fg_light);
    else
        text_rssi.set_style(ui::Theme::getInstance()->fg_light);

    // Calculate distance
    const float dist_m = calc_distance_meters(
        current_rssi_,
        tx_power_dbm_,
        target_freq_);

    const int conf = calc_confidence(current_rssi_);

    // Confidence bar
    bar_confidence.set_value(conf);
    text_conf_pct.set(to_string_dec_uint(conf) + "%");

    if (dist_m < 0) {
        // Signal too weak / unreliable
        text_miles.set("--- ");
        text_meters.set("--- ");
        text_live.set("Signal too weak to estimate");
        text_live.set_style(ui::Theme::getInstance()->fg_light);
        return;
    }

    // Distance displays
    text_miles.set(format_miles(dist_m));
    text_meters.set(format_meters(dist_m));

    // Color code distance display by confidence
    if (conf >= 70) {
        text_miles.set_style(ui::Theme::getInstance()->fg_light);
        text_meters.set_style(ui::Theme::getInstance()->fg_light);
        text_live.set("Good estimate");
        text_live.set_style(ui::Theme::getInstance()->fg_light);
    } else if (conf >= 40) {
        text_miles.set_style(ui::Theme::getInstance()->fg_light);
        text_meters.set_style(ui::Theme::getInstance()->fg_light);
        text_live.set("Rough estimate - move closer");
        text_live.set_style(ui::Theme::getInstance()->fg_light);
    } else {
        text_miles.set_style(ui::Theme::getInstance()->fg_light);
        text_meters.set_style(ui::Theme::getInstance()->fg_light);
        text_live.set("Weak signal - low accuracy");
        text_live.set_style(ui::Theme::getInstance()->fg_light);
    }
}

// ─────────────────────────────────────────
// Free Space Path Loss distance calculation
//
// Formula:
//   d(km) = 10^((TxPow_dBm - RxRSSI_dBm
//                - 20*log10(f_MHz) - 32.44) / 20)
//   d(m)  = d(km) * 1000
//
// Returns distance in METERS, or -1 if unreliable
// ─────────────────────────────────────────
float RangeEstView::calc_distance_meters(
    int32_t  rssi_dbm,
    int32_t  tx_dbm,
    uint64_t freq_hz) const {

    // Reject readings that are too weak — math breaks down
    if (rssi_dbm < -105) return -1.0f;

    // Frequency in MHz
    const float freq_mhz =
        static_cast<float>(freq_hz) / 1'000'000.0f;

    if (freq_mhz <= 0.0f) return -1.0f;

    // FSPL rearranged for distance:
    // d_km = 10 ^ ((TxPow - RxRSSI - 20*log10(f_MHz) - 32.44) / 20)
    const float path_loss =
        static_cast<float>(tx_dbm - rssi_dbm);

    const float log_freq = 20.0f * log10f(freq_mhz);

    const float exponent =
        (path_loss - log_freq - 32.44f) / 20.0f;

    const float dist_km = powf(10.0f, exponent);

    // Convert km to meters
    const float dist_m = dist_km * 1000.0f;

    // Sanity clamp — reject nonsensical values
    if (dist_m < 0.01f) return 0.01f;   // 1cm minimum
    if (dist_m > 50'000'000.0f)          // 50,000 km max
        return -1.0f;

    return dist_m;
}

// ─────────────────────────────────────────
// Confidence score 0-100%
// Based on signal strength — stronger = more reliable
// ─────────────────────────────────────────
int RangeEstView::calc_confidence(int32_t rssi_dbm) const {
    // -40 dBm or stronger = 100% confidence
    // -105 dBm or weaker  = 0% confidence
    if (rssi_dbm >= -40)  return 100;
    if (rssi_dbm <= -105) return 0;
    return (int)(((float)(rssi_dbm + 105) / 65.0f) * 100.0f);
}

// ─────────────────────────────────────────
// Format meters — smart units
// < 1000m  → show as "XXX m"
// >= 1000m → show as "X,XXX m"
// ─────────────────────────────────────────
std::string RangeEstView::format_meters(float m) const {
    const uint32_t im = (uint32_t)(m + 0.5f);

    if (im < 1000) {
        return to_string_dec_uint(im) + " m";
    } else if (im < 10'000) {
        const uint32_t thou  = im / 1000;
        const uint32_t hunds = (im % 1000) / 10;
        return to_string_dec_uint(thou) + "," +
               (hunds < 10 ? "0" : "") +
               to_string_dec_uint(hunds) + " m";
    } else {
        return to_string_dec_uint(im / 1000) + "k m";
    }
}

// ─────────────────────────────────────────
// Format miles — 3 decimal places
// ─────────────────────────────────────────
std::string RangeEstView::format_miles(float m) const {
    // 1 meter = 0.000621371 miles
    const float miles = m * 0.000621371f;

    if (miles < 0.001f) {
        return "< 0.001 mi";
    } else if (miles < 1.0f) {
        // Show 3 decimal places
        const uint32_t thou =
            (uint32_t)(miles * 1000.0f + 0.5f);
        return std::string("0.") +
               (thou < 100 ? "0" : "") +
               (thou < 10  ? "0" : "") +
               to_string_dec_uint(thou) + " mi";
    } else if (miles < 10.0f) {
        // Show 2 decimal places
        const uint32_t whole = (uint32_t)miles;
        const uint32_t cents =
            (uint32_t)((miles - whole) * 100.0f + 0.5f);
        return to_string_dec_uint(whole) + "." +
               (cents < 10 ? "0" : "") +
               to_string_dec_uint(cents) + " mi";
    } else {
        // Show 1 decimal place
        const uint32_t whole = (uint32_t)miles;
        const uint32_t tenth =
            (uint32_t)((miles - whole) * 10.0f + 0.5f);
        return to_string_dec_uint(whole) + "." +
               to_string_dec_uint(tenth) + " mi";
    }
}

// ─────────────────────────────────────────
// Format frequency for display
// ─────────────────────────────────────────
std::string RangeEstView::format_freq(
    rf::Frequency f) const {

    if (f >= 1'000'000'000) {
        const uint32_t ghz = f / 1'000'000'000;
        const uint32_t mhz =
            (f % 1'000'000'000) / 1'000'000;
        return to_string_dec_uint(ghz) + "." +
               (mhz < 100 ? "0" : "") +
               (mhz < 10  ? "0" : "") +
               to_string_dec_uint(mhz) + " GHz";
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

}  // namespace ui::external_app::range_est
