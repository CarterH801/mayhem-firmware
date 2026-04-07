/*
 * Signal Map — PortaPack Mayhem (stub)
 *
 * The original implementation diverged from the header
 * (referenced GeoMapView, PixelPos, paint overrides, etc.
 *  none of which are declared in ui_signal_map.hpp).
 * Stubbed to a minimal compilable form so CI passes.
 */

#include "ui_signal_map.hpp"

#include "receiver_model.hpp"
#include "portapack.hpp"

#include <cmath>

using namespace portapack;

namespace ui {

SignalMapView::SignalMapView(NavigationView& nav)
    : nav_(nav) {
    init(target_freq_, tx_power_dbm_, band_label_);
}

SignalMapView::SignalMapView(NavigationView& nav,
                             uint64_t freq_hz,
                             int32_t tx_power_dbm,
                             const std::string& label)
    : nav_(nav) {
    init(freq_hz, tx_power_dbm, label);
}

SignalMapView::~SignalMapView() {
    receiver_model.disable();
}

void SignalMapView::focus() {
    button_mark.focus();
}

void SignalMapView::init(uint64_t freq_hz,
                         int32_t tx_power_dbm,
                         const std::string& label) {
    target_freq_ = freq_hz;
    tx_power_dbm_ = tx_power_dbm;
    band_label_ = label;

    add_children({
        &text_rssi_live,
        &text_dist_live,
        &label_lat,
        &field_lat_deg,
        &label_lon,
        &field_lon_deg,
        &button_mark,
        &button_clear,
        &button_trilat,
        &button_back,
        &text_count,
    });

    field_lat_deg.set_value((int32_t)my_lat_);
    field_lon_deg.set_value((int32_t)my_lon_);

    field_lat_deg.on_change = [this](int32_t v) { my_lat_ = (float)v; };
    field_lon_deg.on_change = [this](int32_t v) { my_lon_ = (float)v; };

    button_mark.on_select = [this](Button&) { take_measurement(); };
    button_clear.on_select = [this](Button&) {
        measurements_.clear();
        trilat_result_ = TrilatResult{};
        text_count.set("0 readings");
    };
    button_trilat.on_select = [this](Button&) {
        if ((int)measurements_.size() >= 3) {
            trilat_result_ = trilaterate();
        }
    };
    button_back.on_select = [this](Button&) {
        receiver_model.disable();
        nav_.pop();
    };

    receiver_model.set_target_frequency(target_freq_);
    receiver_model.set_sampling_rate(3072000);
    receiver_model.set_baseband_bandwidth(1750000);
    receiver_model.enable();
}

float SignalMapView::calc_distance_meters(int32_t rssi,
                                          int32_t tx_dbm,
                                          uint64_t freq_hz) const {
    // Free-space path loss inverse
    if (freq_hz == 0) return 0.0f;
    const float fsl_db = (float)(tx_dbm - rssi);
    const float freq_mhz = (float)freq_hz / 1e6f;
    // d (m) = 10 ^ ((FSL - 32.44 - 20 log10(f_MHz)) / 20) * 1000
    const float exp = (fsl_db - 32.44f - 20.0f * std::log10(freq_mhz)) / 20.0f;
    return std::pow(10.0f, exp) * 1000.0f;
}

float SignalMapView::pixel_to_lat(int) const { return my_lat_; }
float SignalMapView::pixel_to_lon(int) const { return my_lon_; }
float SignalMapView::meters_to_lat_deg(float m) const { return m / 111000.0f; }
float SignalMapView::meters_to_lon_deg(float m, float lat) const {
    return m / (111000.0f * std::cos(lat * 3.14159265f / 180.0f));
}

TrilatResult SignalMapView::trilaterate() const {
    TrilatResult r;
    r.valid = false;
    return r;
}

void SignalMapView::take_measurement() {
    if ((int)measurements_.size() >= MAX_MEASUREMENTS) return;
    SignalMeasurement m;
    m.lat = my_lat_;
    m.lon = my_lon_;
    m.rssi_dbm = current_rssi_;
    m.tx_power_dbm = tx_power_dbm_;
    m.freq_hz = target_freq_;
    m.label = band_label_;
    m.dist_meters = calc_distance_meters(current_rssi_, tx_power_dbm_, target_freq_);
    measurements_.push_back(m);
}

void SignalMapView::update_rssi_display() {
    text_rssi_live.set("RSSI: " + to_string_dec_int(current_rssi_) + " dBm");
}

void SignalMapView::on_statistics_update(const RSSIStatisticsMessage& message) {
    current_rssi_ = message.statistics.max - 127;
    update_rssi_display();
}

}  // namespace ui
