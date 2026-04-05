/*
 * Satellite Pass Detector
 * File: firmware/application/apps/ui_sat_pass.hpp
 *
 * Monitors known frequencies for the ISS and amateur
 * satellites. Alerts when a pass is detected overhead.
 *
 * Known frequencies monitored:
 *
 *  ISS (International Space Station):
 *    145.800 MHz — APRS / packet downlink
 *    145.825 MHz — ARISS voice downlink
 *    437.550 MHz — cross-band FM repeater
 *    437.800 MHz — ISS digipeater
 *
 *  Amateur Satellites:
 *    AO-91 (RadFxSat):   145.960 MHz downlink
 *    AO-92 (Fox-1D):     145.880 MHz downlink
 *    SO-50:              436.795 MHz downlink
 *    LilacSat-2:         437.200 MHz FM downlink
 *    TEVEL-2 through 7:  436.400 MHz FM
 *    CAS-4A/B:           145.835 MHz
 *    XW-2 series:        145.670-145.900 MHz range
 *
 *  NOAA Weather Satellites:
 *    NOAA-15: 137.620 MHz
 *    NOAA-18: 137.913 MHz
 *    NOAA-19: 137.100 MHz
 *
 * When a signal is detected above threshold on any of
 * these frequencies, the app alerts with a beep and
 * logs the detection time.
 *
 * For accurate pass prediction you need:
 *  - External GPS (for your location)
 *  - Current TLE data (Two-Line Elements) for each sat
 *  - The built-in orbital calculation (simplified here)
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_SAT_PASS_H__
#define __UI_SAT_PASS_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "portapack.hpp"
#include "rtc_time.hpp"
#include "baseband_api.hpp"

#include <array>
#include <vector>
#include <string>
#include <cstdint>

namespace ui {

// ─────────────────────────────────────────────────────────
// Satellite frequency entry
// ─────────────────────────────────────────────────────────
struct SatFreq {
    const char* sat_name;
    const char* description;
    uint64_t    freq_hz;
    const char* modulation;  // "NFM", "FM", "APRS"
    int32_t     threshold_dbm;
};

static const SatFreq SAT_FREQS[] = {
    // ── ISS ──────────────────────────────────────────────
    {"ISS",      "APRS/Packet",    145'800'000, "NFM", -100},
    {"ISS",      "ARISS Voice",    145'825'000, "NFM", -95 },
    {"ISS",      "UHF Repeater",   437'550'000, "NFM", -95 },
    {"ISS",      "Digipeater",     437'800'000, "NFM", -95 },
    // ── Amateur Satellites ────────────────────────────────
    {"AO-91",    "FM Downlink",    145'960'000, "NFM", -100},
    {"AO-92",    "FM Downlink",    145'880'000, "NFM", -100},
    {"SO-50",    "FM Downlink",    436'795'000, "NFM", -100},
    {"LilacSat2","FM Downlink",    437'200'000, "NFM", -100},
    {"TEVEL",    "FM Cluster",     436'400'000, "NFM", -100},
    {"CAS-4A",   "CW/SSB",         145'835'000, "NFM", -100},
    {"XW-2A",    "FM Downlink",    145'670'000, "NFM", -100},
    // ── Weather Satellites ────────────────────────────────
    {"NOAA-15",  "APT Image 137.6",137'620'000, "NFM", -95 },
    {"NOAA-18",  "APT Image 137.9",137'912'500, "NFM", -95 },
    {"NOAA-19",  "APT Image 137.1",137'100'000, "NFM", -95 },
    // ── Meteor Weather (Russian) ──────────────────────────
    {"METEOR M2","LRPT 137.9",     137'900'000, "NFM", -95 },
};
static const int SAT_FREQ_COUNT =
    sizeof(SAT_FREQS) / sizeof(SAT_FREQS[0]);

// ─────────────────────────────────────────────────────────
// Detected pass event
// ─────────────────────────────────────────────────────────
struct PassEvent {
    std::string sat_name{""};
    std::string description{""};
    uint64_t    freq_hz{0};
    int32_t     peak_rssi{-120};
    uint32_t    first_heard{0};  // RTC seconds
    uint32_t    duration_sec{0};
    bool        active{false};
};

// ─────────────────────────────────────────────────────────
// Satellite Pass Detector View
// ─────────────────────────────────────────────────────────
class SatPassView : public View {
   public:
    SatPassView(NavigationView& nav);
    ~SatPassView();

    void focus() override;
    std::string title() const override {
        return "Satellite Detector";
    }

   private:
    NavigationView& nav_;

    bool    watching_{false};
    int     current_freq_idx_{0};
    uint32_t dwell_counter_{0};
    static constexpr uint32_t DWELL_TICKS = 5; // 500ms

    // Pass log (last 10)
    static constexpr int MAX_EVENTS = 10;
    std::vector<PassEvent> events_{};

    // Active detection state per frequency
    std::array<bool, SAT_FREQ_COUNT> detected_{};
    std::array<int32_t, SAT_FREQ_COUNT> peak_rssi_{};

    void start_watch();
    void stop_watch();
    void advance_scan();
    void check_detection(int freq_idx, int32_t rssi);
    void trigger_pass(int freq_idx, int32_t rssi);
    void draw_event_log();
    std::string format_freq(uint64_t f) const;
    std::string format_time(uint32_t t) const;

    void on_statistics_update(
        const RSSIStatisticsMessage& message);

    // ── Widgets ───────────────────────────────────────────

    // Header
    Labels label_header{
        {{0, 0}, "SAT PASS DETECTOR", Color::cyan()}};

    // Current scan status
    Text text_scanning{
        {0, 16, 240, 14}, "Press WATCH to start"};

    // Active pass alert
    Text text_alert{
        {0, 32, 240, 20}, ""};

    // Scan progress
    Text text_scan_prog{
        {0, 54, 240, 12}, ""};

    // Column header for event log
    Labels label_log_hdr{
        {{0, 68},
         "TIME     SAT         FREQ      RSSI",
         Color::light_grey()}};

    // 8 event log rows
    Text text_log_0{{0,  82, 240, 12}, ""};
    Text text_log_1{{0,  94, 240, 12}, ""};
    Text text_log_2{{0, 106, 240, 12}, ""};
    Text text_log_3{{0, 118, 240, 12}, ""};
    Text text_log_4{{0, 130, 240, 12}, ""};
    Text text_log_5{{0, 142, 240, 12}, ""};
    Text text_log_6{{0, 154, 240, 12}, ""};
    Text text_log_7{{0, 166, 240, 12}, ""};

    // Stats
    Text text_stats{
        {0, 180, 240, 14}, "0 passes detected"};

    // Frequency being scanned
    Text text_freq_now{
        {0, 196, 240, 14}, ""};

    // Buttons
    Button button_watch{
        {0, 212, 7 * 8, 16}, "WATCH"};
    Button button_clear{
        {8 * 8, 212, 7 * 8, 16}, "CLEAR"};
    Button button_back{
        {16 * 8, 212, 8 * 8, 16}, "BACK"};

    Labels label_hint{
        {{0, 230},
         "Point antenna toward sky for best results",
         Color::grey()}};

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

#endif /*__UI_SAT_PASS_H__*/
