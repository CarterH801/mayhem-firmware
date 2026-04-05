/*
 * Range Estimator — PortaPack Mayhem
 * File: firmware/application/apps/ui_range_est.hpp
 *
 * Estimates transmitter distance using Free Space
 * Path Loss (FSPL) formula from live RSSI readings.
 *
 * Displays distance in MILES and METERS.
 * Updates live as signal strength changes.
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_RANGE_EST_H__
#define __UI_RANGE_EST_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "radio_state.hpp"
#include "portapack.hpp"
#include "ui_bandplan.hpp"

#include <cstdint>
#include <string>
#include <cmath>

namespace ui {

// ─────────────────────────────────────────────────────────
// Known device types with typical TX power in dBm
// User selects from this list
// ─────────────────────────────────────────────────────────
struct DevicePreset {
    const char* name;       // Display name
    int32_t     tx_power;   // dBm
    uint64_t    freq_hint;  // Hz — 0 means any freq
};

static const DevicePreset DEVICE_PRESETS[] = {
    // ── Drones ───────────────────────────────────────────
    { "DJI Mavic/Mini/Air",   26,  2'440'000'000 },
    { "DJI Phantom",          26,  2'440'000'000 },
    { "DJI FPV / Avata",      26,  5'800'000'000 },
    { "Autel EVO/Nano",       26,  2'440'000'000 },
    { "FPV Drone VTX 25mW",   14,  5'800'000'000 },
    { "FPV Drone VTX 200mW",  23,  5'800'000'000 },
    { "FPV Drone VTX 600mW",  28,  5'800'000'000 },
    { "FPV Drone VTX 1W",     30,  5'800'000'000 },
    { "ELRS 915MHz",          27,    915'000'000 },
    { "ELRS 2.4GHz",          27,  2'440'000'000 },
    { "TBS Crossfire",        27,    915'000'000 },
    // ── Key Fobs / Cars ──────────────────────────────────
    { "Car Key Fob (US)",     10,    315'000'000 },
    { "Car Key Fob (EU)",     10,    433'920'000 },
    { "TPMS Tire Sensor",      6,    315'000'000 },
    // ── WiFi ─────────────────────────────────────────────
    { "WiFi Router 2.4GHz",   20,  2'440'000'000 },
    { "WiFi Router 5GHz",     20,  5'200'000'000 },
    { "WiFi Hotspot/Phone",   17,  2'440'000'000 },
    // ── Broadcast ────────────────────────────────────────
    { "FM Radio Station",     57,     98'000'000 },
    { "AM Radio Station",     53,      1'000'000 },
    { "NOAA Weather Radio",   47,    162'450'000 },
    // ── Personal Radio ───────────────────────────────────
    { "FRS Walkie Talkie",    17,    462'000'000 },
    { "GMRS Radio",           27,    462'000'000 },
    { "Ham HT (5W)",          37,    146'000'000 },
    { "Ham Mobile (50W)",     47,    146'000'000 },
    // ── Cellular ─────────────────────────────────────────
    { "Cell Tower LTE",       46,    850'000'000 },
    { "Cell Phone Uplink",    23,    850'000'000 },
    // ── Custom ───────────────────────────────────────────
    { "Custom (manual)",       0,              0 },
};

static const int DEVICE_PRESET_COUNT =
    sizeof(DEVICE_PRESETS) / sizeof(DEVICE_PRESETS[0]);

// ─────────────────────────────────────────────────────────
// Range Estimator View
// ─────────────────────────────────────────────────────────
class RangeEstView : public View {
   public:
    RangeEstView(NavigationView& nav);

    // Launch pre-tuned from Signal Finder or Band Scanner
    RangeEstView(NavigationView& nav,
                 rf::Frequency  freq,
                 const std::string& label);

    ~RangeEstView();

    void focus() override;
    std::string title() const override {
        return "Range Estimator";
    }

   private:
    void init(rf::Frequency freq, const std::string& label);

    NavigationView& nav_;

    rf::Frequency  target_freq_{315'000'000};
    std::string    band_label_{"---"};
    int32_t        current_rssi_{-120};
    int32_t        tx_power_dbm_{20};
    bool           smoothing_enabled_{true};

    // Smoothed RSSI (rolling average of last 5 readings)
    static constexpr int SMOOTH_LEN = 5;
    int32_t  rssi_buf_[SMOOTH_LEN]{};
    int      rssi_buf_idx_{0};
    bool     rssi_buf_full_{false};

    // ── Distance calculation ──────────────────────────────
    // Returns distance in METERS using FSPL formula
    // Returns -1 if signal too weak to estimate reliably
    float calc_distance_meters(int32_t rssi_dbm,
                               int32_t tx_dbm,
                               uint64_t freq_hz) const;

    // Confidence 0-100% based on signal strength
    int calc_confidence(int32_t rssi_dbm) const;

    std::string format_meters(float m) const;
    std::string format_miles(float m) const;
    std::string format_freq(rf::Frequency f) const;

    int32_t smoothed_rssi() const;

    void on_statistics_update(
        const RSSIStatisticsMessage& message);

    void update_display();
    void update_device_preset(int idx);

    // ── UI Widgets ────────────────────────────────────────

    // Row 0: Frequency
    Text text_freq{
        {0, 0, 240, 16}, "---"};

    // Row 1: Band label
    Text text_band{
        {0, 16, 240, 14}, "---"};

    // Row 2: Device selector label
    Labels label_device{
        {{0, 32}, "Device:", Color::light_grey()}};

    // Device type selector
    OptionsField options_device{
        {7 * 8, 32}, 18,
        {
            {"DJI Mavic/Mini/Air   ", 0},
            {"DJI Phantom          ", 1},
            {"DJI FPV / Avata      ", 2},
            {"Autel EVO/Nano       ", 3},
            {"FPV VTX 25mW         ", 4},
            {"FPV VTX 200mW        ", 5},
            {"FPV VTX 600mW        ", 6},
            {"FPV VTX 1W           ", 7},
            {"ELRS 915MHz          ", 8},
            {"ELRS 2.4GHz          ", 9},
            {"TBS Crossfire        ", 10},
            {"Car Key Fob (US)     ", 11},
            {"Car Key Fob (EU)     ", 12},
            {"TPMS Tire Sensor     ", 13},
            {"WiFi Router 2.4GHz   ", 14},
            {"WiFi Router 5GHz     ", 15},
            {"WiFi Hotspot/Phone   ", 16},
            {"FM Radio Station     ", 17},
            {"AM Radio Station     ", 18},
            {"NOAA Weather Radio   ", 19},
            {"FRS Walkie Talkie    ", 20},
            {"GMRS Radio           ", 21},
            {"Ham HT (5W)          ", 22},
            {"Ham Mobile (50W)     ", 23},
            {"Cell Tower LTE       ", 24},
            {"Cell Phone Uplink    ", 25},
            {"Custom (manual)      ", 26},
        }};

    // Row 3: TX Power (editable for custom mode)
    Labels label_txpow{
        {{0, 48}, "TX Power:", Color::light_grey()}};

    NumberField field_txpower{
        {10 * 8, 48}, 4, {-10, 60}, 1, ' ', false};

    Labels label_dbm{
        {{15 * 8, 48}, "dBm", Color::light_grey()}};

    // Row 4: Current RSSI
    Labels label_rssi_l{
        {{0, 64}, "Signal:", Color::light_grey()}};
    Text text_rssi{
        {8 * 8, 64, 80, 14}, "-120 dBm"};

    // Row 5: Divider
    Labels label_div{
        {{0, 80}, "──────── DISTANCE ────────", Color::grey()}};

    // ── Big distance display ──────────────────────────────

    // Miles — large
    Labels label_mi_unit{
        {{160, 98}, "miles", Color::light_grey()}};
    Text text_miles{
        {0, 94, 155, 24}, "---"};

    // Meters — large
    Labels label_m_unit{
        {{168, 126}, "meters", Color::light_grey()}};
    Text text_meters{
        {0, 122, 163, 24}, "---"};

    // Row 8: Confidence bar
    Labels label_conf_l{
        {{0, 150}, "Confidence:", Color::light_grey()}};
    ProgressBar bar_confidence{
        {12 * 8, 150, 108, 12}};
    Text text_conf_pct{
        {27 * 8, 150, 32, 12}, "  0%"};

    // Row 9: Accuracy note
    Text text_accuracy{
        {0, 166, 240, 14}, "Open air line-of-sight only"};

    // Row 10: Live update indicator
    Text text_live{
        {0, 182, 240, 14}, "Waiting for signal..."};

    // Row 11: Smoothing toggle
    Button button_smooth{
        {0, 198, 10 * 8, 16}, "SMOOTH:ON"};

    // Row 12: Buttons
    Button button_back{
        {0, 216, 8 * 8, 16}, "BACK"};

    Button button_reset{
        {9 * 8, 216, 8 * 8, 16}, "RESET"};

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

#endif /*__UI_RANGE_EST_H__*/
