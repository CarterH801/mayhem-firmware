/*
 * AM/FM Band Scanner for PortaPack Mayhem — v2 with Band Plan
 * File: firmware/application/apps/ui_amfm_scanner.hpp
 *
 * Now includes automatic frequency identification via US band plan.
 * Active signals are labeled AND color-coded by service type.
 *
 * Place this file in: firmware/application/apps/
 */

#ifndef __UI_AMFM_SCANNER_H__
#define __UI_AMFM_SCANNER_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "ui_receiver.hpp"
#include "string_format.hpp"
#include "radio_state.hpp"
#include "receiver_model.hpp"
#include "portapack.hpp"
#include "ui_bandplan.hpp"   // <-- Our new band plan

#include <vector>
#include <string>

namespace ui {

// ─────────────────────────────────────────────────────────
// Color map for band categories
// ─────────────────────────────────────────────────────────
inline Color color_for_category(const std::string& cat) {
    if (cat == "EMERGENCY")   return Color::red();
    if (cat == "AVIATION")    return Color::cyan();
    if (cat == "BROADCAST")   return Color::green();
    if (cat == "WEATHER")     return Color::yellow();
    if (cat == "MARINE")      return Color::blue();
    if (cat == "HAM")         return Color::magenta();
    if (cat == "MILITARY")    return Color::orange();
    if (cat == "GOVERNMENT")  return Color::yellow();
    if (cat == "BUSINESS")    return Color::white();
    if (cat == "PERSONAL")    return Color::light_grey();
    if (cat == "CELLULAR")    return Color::grey();
    if (cat == "NAVIGATION")  return Color::cyan();
    if (cat == "DRONE")       return Color::orange();
    if (cat == "KEYFOB")      return Color::orange();
    if (cat == "BLUETOOTH")   return Color::blue();
    if (cat == "DATA")        return Color::light_grey();
    return Color::white();
}

// ─────────────────────────────────────────────────────────
// A detected active station with band label
// ─────────────────────────────────────────────────────────
struct StationEntry {
    rf::Frequency frequency;
    int32_t rssi_db;
    std::string freq_str;      // e.g. "98.100 MHz"
    std::string band_label;    // e.g. "FM RADIO"
    std::string category;      // e.g. "BROADCAST"
};

class AMFMScannerView : public View {
   public:
    AMFMScannerView(NavigationView& nav);
    ~AMFMScannerView();

    void focus() override;
    std::string title() const override { return "Band Scanner"; }

   private:
    enum class ScanState { IDLE, CALIBRATING, SCANNING, DONE };

    NavigationView& nav_;
    ScanState scan_state_{ScanState::IDLE};

    // ── Band definitions ──────────────────────────────────
    static constexpr rf::Frequency FM_START = 87'500'000;
    static constexpr rf::Frequency FM_END   = 108'000'000;
    static constexpr rf::Frequency FM_STEP  = 200'000;

    static constexpr rf::Frequency AM_START = 1'000'000;
    static constexpr rf::Frequency AM_END   = 1'710'000;
    static constexpr rf::Frequency AM_STEP  = 10'000;

    // Current scan position
    rf::Frequency current_freq_{0};
    rf::Frequency scan_start_{0};
    rf::Frequency scan_end_{0};
    rf::Frequency scan_step_{0};

    // Noise floor calibration
    int32_t noise_floor_db_{-100};
    int32_t noise_samples_{0};
    int32_t noise_sum_{0};
    static constexpr int32_t SIGNAL_THRESHOLD_ABOVE_NOISE = 12;
    static constexpr int32_t SQUELCH_SAMPLES = 3;

    int32_t confirm_count_{0};
    int8_t  scan_mode_{0};
    int32_t squelch_db_{-75};

    std::vector<StationEntry> found_stations_{};
    int16_t list_offset_{0};
    int16_t selected_station_{-1};

    // ── Helpers ───────────────────────────────────────────
    void start_scan();
    void stop_scan();
    void next_frequency();
    void tune_to(rf::Frequency freq);
    void set_modulation_for_band();
    void on_statistics_update(const RSSIStatisticsMessage& message);
    void add_station(rf::Frequency freq, int32_t rssi);
    std::string format_frequency(rf::Frequency freq) const;
    void draw_station_list();
    void update_status_text();
    void scroll_list(int16_t delta);

    // ── UI Widgets ────────────────────────────────────────

    // Row 0: Mode | Squelch | Scan button
    OptionsField options_mode{
        {0, 0}, 4,
        {{"FM  ", 0},
         {"AM  ", 1},
         {"BOTH", 2},
         {"KFOB", 3},
         {"DRON", 4},
         {"BT  ", 5}}};

    Labels label_sq{
        {{5 * 8, 0}, "SQ:", Color::light_grey()}};

    NumberField field_squelch{
        {8 * 8, 0}, 4, {-120, -20}, 1, ' ', false};

    Button button_scan{
        {14 * 8, 0, 6 * 8, 16}, "SCAN"};

    // Row 1: Status
    Text text_status{
        {0, 16, 240, 14}, "Select mode and press SCAN"};

    // Row 2: Progress bar
    ProgressBar progress_bar{
        {0, 32, 240, 8}};

    // Row 3: Legend / color key
    Labels label_legend{
        {{0, 42},      "RED=EMRG ",  Color::red()},
        {{9 * 8, 42},  "CYN=AIR ",   Color::cyan()},
        {{17 * 8, 42}, "GRN=BCAST",  Color::green()}};

    Labels label_legend2{
        {{0, 52},      "YLW=WX ",    Color::yellow()},
        {{7 * 8, 52},  "BLU=MAR ",   Color::blue()},
        {{14 * 8, 52}, "MAG=HAM",    Color::magenta()}};

    // Row 4: Column headers
    Labels label_headers{
        {{0, 64}, "FREQUENCY       SERVICE", Color::light_grey()}};

    // ── Station list rows (8 visible at a time) ──────────
    // Each is a Text widget; color is set dynamically
    Text text_s0{{0,  80, 240, 14}, ""};
    Text text_s1{{0,  96, 240, 14}, ""};
    Text text_s2{{0, 112, 240, 14}, ""};
    Text text_s3{{0, 128, 240, 14}, ""};
    Text text_s4{{0, 144, 240, 14}, ""};
    Text text_s5{{0, 160, 240, 14}, ""};
    Text text_s6{{0, 176, 240, 14}, ""};
    Text text_s7{{0, 192, 240, 14}, ""};

    // Scroll hint
    Text text_scroll{
        {0, 208, 240, 12}, ""};

    // ── Bottom buttons ────────────────────────────────────
    Button button_tune{
        {0, 222, 7 * 8, 16}, "TUNE"};

    Button button_up{
        {8 * 8, 222, 3 * 8, 16}, " ^"};

    Button button_down{
        {12 * 8, 222, 3 * 8, 16}, " v"};

    Button button_clear{
        {16 * 8, 222, 6 * 8, 16}, "CLEAR"};

    Button button_back{
        {23 * 8, 222, 5 * 8, 16}, "BACK"};

    // ── RSSI message handler ──────────────────────────────
    MessageHandlerRegistration message_handler_stats{
        Message::ID::RSSIStatistics,
        [this](const Message* const p) {
            const auto message =
                *reinterpret_cast<const RSSIStatisticsMessage*>(p);
            this->on_statistics_update(message);
        }};
};

}  // namespace ui

#endif /*__UI_AMFM_SCANNER_H__*/
