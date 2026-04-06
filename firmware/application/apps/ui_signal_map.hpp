/*
 * Signal Map — PortaPack Mayhem
 * File: firmware/application/apps/ui_signal_map.hpp
 *
 * Plots RSSI-based distance estimates on the world map.
 * Uses RF trilateration — take readings from 3+ positions
 * and where the circles intersect = transmitter location.
 *
 * Uses Mayhem's existing world_map.bin on SD card.
 * Works standalone or with external GPS module.
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_SIGNAL_MAP_H__
#define __UI_SIGNAL_MAP_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
          // Mayhem's existing map widget
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "portapack.hpp"
#include "ui_bandplan.hpp"

#include <vector>
#include <string>
#include <cmath>

namespace ui {

// ─────────────────────────────────────────────────────────
// One signal measurement taken from a known position
// ─────────────────────────────────────────────────────────
struct SignalMeasurement {
    float    lat;           // Observer lat (decimal degrees)
    float    lon;           // Observer lon (decimal degrees)
    float    dist_meters;   // Estimated distance to transmitter
    int32_t  rssi_dbm;      // Raw RSSI at time of reading
    int32_t  tx_power_dbm;  // TX power used for calculation
    uint64_t freq_hz;       // Frequency
    std::string label;      // Band label
};

// ─────────────────────────────────────────────────────────
// Result of trilateration from 3+ measurements
// ─────────────────────────────────────────────────────────
struct TrilatResult {
    float lat;              // Estimated transmitter lat
    float lon;              // Estimated transmitter lon
    float error_meters;     // Estimated error radius
    bool  valid{false};
};

// ─────────────────────────────────────────────────────────
// Signal Map View
// ─────────────────────────────────────────────────────────
class SignalMapView : public View {
   public:
    // Standalone — user sets own position manually
    SignalMapView(NavigationView& nav);

    // Pre-loaded with frequency from scanner/finder
    SignalMapView(NavigationView& nav,
                  uint64_t    freq_hz,
                  int32_t     tx_power_dbm,
                  const std::string& label);

    ~SignalMapView();

    void focus() override;
    std::string title() const override {
        return "Signal Map";
    }

    // Paint override — draw circles and markers on map

   private:
    void init(uint64_t freq_hz,
              int32_t  tx_power_dbm,
              const std::string& label);

    NavigationView& nav_;

    // ── Target signal ─────────────────────────────────────
    uint64_t    target_freq_{315'000'000};
    int32_t     tx_power_dbm_{20};
    std::string band_label_{"---"};

    // ── Current live RSSI ─────────────────────────────────
    int32_t current_rssi_{-120};

    // ── Observer position ─────────────────────────────────
    float  my_lat_{37.7749f};   // Default: San Francisco
    float  my_lon_{-122.4194f};
    bool   gps_locked_{false};

    // ── Measurements list (max 8) ─────────────────────────
    static constexpr int MAX_MEASUREMENTS = 8;
    std::vector<SignalMeasurement> measurements_{};

    // ── Trilateration result ──────────────────────────────
    TrilatResult trilat_result_{};

    // ── Mode: SET_POS or MEASURING ────────────────────────
    enum class Mode { SET_POS, MEASURING };
    Mode mode_{Mode::SET_POS};

    // ── Map view area (top portion of screen) ────────────
    // Map takes up rows 0-190, controls below

    // ── Math helpers ──────────────────────────────────────
    float calc_distance_meters(int32_t rssi,
                               int32_t tx_dbm,
                               uint64_t freq_hz) const;

    // Convert lat/lon to pixel on the 240x180 map area
    // Based on Mercator projection matching world_map.bin
    float    pixel_to_lat(int y) const;
    float    pixel_to_lon(int x) const;

    // Degrees of lat/lon per meter at given latitude
    float meters_to_lat_deg(float meters) const;
    float meters_to_lon_deg(float meters, float lat) const;

    // Radius in pixels for a given distance in meters

    // Trilateration from 3+ measurements
    TrilatResult trilaterate() const;

    // Draw everything on map canvas

    void take_measurement();
    void update_rssi_display();

    void on_statistics_update(
        const RSSIStatisticsMessage& message);

    // ── UI Widgets ────────────────────────────────────────


    // ── Row below map: live RSSI + distance ──────────────
    Text text_rssi_live{
        {0, 198, 120, 14}, "RSSI: --- dBm"};

    Text text_dist_live{
        {120, 198, 120, 14}, "Dist: ---"};

    // ── Row: position controls ────────────────────────────
    Labels label_lat{
        {{0, 214}, "Lat:", Color::light_grey()}};

    NumberField field_lat_deg{
        {4 * 8, 214}, 3, {-90, 90}, 1, ' ', false};

    Labels label_lon{
        {{9 * 8, 214}, "Lon:", Color::light_grey()}};

    NumberField field_lon_deg{
        {13 * 8, 214}, 4, {-180, 180}, 1, ' ', false};

    // ── Row: action buttons ───────────────────────────────
    Button button_mark{
        {0, 230, 7 * 8, 16}, "MARK"};

    Button button_clear{
        {8 * 8, 230, 7 * 8, 16}, "CLEAR"};

    Button button_trilat{
        {16 * 8, 230, 7 * 8, 16}, "FIND"};

    Button button_back{
        {24 * 8, 230, 5 * 8, 16}, "BACK"};

    // ── Measurement count display ─────────────────────────
    Text text_count{
        {0, 246, 240, 14}, "0 readings — need 3 to triangulate"};

    // ── RSSI message handler ──────────────────────────────
    MessageHandlerRegistration message_handler_stats{
        Message::ID::RSSIStatistics,
        [this](const Message* const p) {
            const auto msg =
                *reinterpret_cast<
                    const RSSIStatisticsMessage*>(p);
            this->on_statistics_update(msg);
        }};
};

}  // namespace ui

#endif /*__UI_SIGNAL_MAP_H__*/
