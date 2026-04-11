/*
 * Signal Finder — PortaPack Mayhem
 * File: firmware/application/apps/ui_sigfinder.hpp
 *
 * Hot/cold proximity meter for any frequency.
 * Shows signal strength like a metal detector.
 * Captures raw IQ data to SD card on demand.
 *
 * Launched from Band Scanner via FIND button,
 * or standalone from the menu.
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_SIGFINDER_H__
#define __UI_SIGFINDER_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "ui_receiver.hpp"
#include "ui_record_view.hpp"
#include "string_format.hpp"
#include "radio_state.hpp"
#include "receiver_model.hpp"
#include "portapack.hpp"
#include "ui_bandplan.hpp"
#include "file.hpp"

#include <cstdint>
#include <string>
#include <deque>

using namespace ui;

namespace ui::external_app::sigfinder {

class SigFinderView : public View {
   public:
    // Standalone constructor — opens at last used frequency
    SigFinderView(NavigationView& nav);

    // Constructor called from Band Scanner — pre-tuned
    SigFinderView(NavigationView& nav,
                  rf::Frequency freq,
                  const std::string& band_label,
                  const std::string& category);

    ~SigFinderView();

    void focus() override;
    std::string title() const override { return "Signal Finder"; }

   private:
    void init(rf::Frequency freq,
              const std::string& band_label,
              const std::string& category);

    NavigationView& nav_;

    // ── Signal tracking ───────────────────────────────────
    rf::Frequency target_freq_{315'000'000};
    std::string   band_label_{"UNKNOWN"};
    std::string   category_{"UNKNOWN"};

    int32_t  current_rssi_{-120};
    int32_t  peak_rssi_{-120};
    int32_t  prev_rssi_{-120};
    int32_t  baseline_rssi_{-120};    // Set on first reading
    bool     baseline_set_{false};
    uint32_t reading_count_{0};

    // Rolling history for graph (last 30 readings)
    std::deque<int32_t> rssi_history_{};
    static constexpr int HISTORY_LEN = 30;

    // Capture state
    bool capturing_{false};
    std::string capture_filename_{""};

    // Audio beep state
    bool  audio_enabled_{true};
    uint32_t beep_counter_{0};

    // ── Audio tone helpers ────────────────────────────────
    void update_audio_tone();
    void stop_audio();

    // ── Core logic ────────────────────────────────────────
    void on_statistics_update(const RSSIStatisticsMessage& message);
    void update_display();
    void update_trend_arrow();
    void update_strength_bar();
    void update_history_graph();
    void update_hot_cold_label();
    void start_capture();
    void stop_capture();
    std::string make_capture_filename() const;
    std::string format_freq(rf::Frequency f) const;
    int32_t rssi_to_percent(int32_t rssi) const;

    // ── Trend direction ───────────────────────────────────
    enum class Trend { UP, DOWN, STEADY };
    Trend current_trend_{Trend::STEADY};

    // ── UI Widgets ────────────────────────────────────────

    // ── Row 0: Frequency display ──────────────────────────
    Text text_freq{
        {0, 0, 200, 16}, "--- MHz"};

    // ── Row 1: Band label ─────────────────────────────────
    Text text_band{
        {0, 16, 240, 14}, "---"};

    // ── Row 2-3: Big signal strength percentage ───────────
    // Large number fill — 3 chars wide + % symbol
    Text text_percent_big{
        {60, 34, 120, 32}, "  0%"};

    // ── Row 4: HOT / COLD / WARM label ───────────────────
    Text text_hotcold{
        {80, 68, 80, 16}, "NO SIGNAL"};

    // ── Row 5: Trend arrow ────────────────────────────────
    Text text_trend{
        {108, 86, 24, 14}, " "};

    // ── Row 6: Signal strength bar ────────────────────────
    // Visual bar [████████░░░░░░░░░░░░]
    Labels label_bar_l{
        {{0, 104}, "[", Color::white()}};
    ProgressBar bar_signal{
        {8, 104, 204, 14}};
    Labels label_bar_r{
        {{212, 104}, "]", Color::white()}};

    // ── Row 7: dBm reading + peak ────────────────────────
    Text text_dbm{
        {0, 120, 120, 14}, "-120 dBm"};
    Text text_peak{
        {120, 120, 120, 14}, "Peak:-120"};

    // ── Row 8: COLD <────────────────────> HOT scale ──────
    Labels label_scale{
        {{0,   136}, "COLD",   Color::cyan()},
        {{100, 136}, "|",      Color::white()},
        {{196, 136}, "HOT",    Color::red()}};

    // ── Row 9-12: Rolling history graph (30 bars) ─────────
    // Drawn manually as a sequence of Text chars
    Text text_graph_0{{  0, 152, 8, 48}, " "};
    Text text_graph_1{{  8, 152, 8, 48}, " "};
    Text text_graph_2{{ 16, 152, 8, 48}, " "};
    Text text_graph_3{{ 24, 152, 8, 48}, " "};
    Text text_graph_4{{ 32, 152, 8, 48}, " "};
    Text text_graph_5{{ 40, 152, 8, 48}, " "};
    Text text_graph_6{{ 48, 152, 8, 48}, " "};
    Text text_graph_7{{ 56, 152, 8, 48}, " "};
    Text text_graph_8{{ 64, 152, 8, 48}, " "};
    Text text_graph_9{{ 72, 152, 8, 48}, " "};
    Text text_graph_10{{ 80, 152, 8, 48}, " "};
    Text text_graph_11{{ 88, 152, 8, 48}, " "};
    Text text_graph_12{{ 96, 152, 8, 48}, " "};
    Text text_graph_13{{104, 152, 8, 48}, " "};
    Text text_graph_14{{112, 152, 8, 48}, " "};
    Text text_graph_15{{120, 152, 8, 48}, " "};
    Text text_graph_16{{128, 152, 8, 48}, " "};
    Text text_graph_17{{136, 152, 8, 48}, " "};
    Text text_graph_18{{144, 152, 8, 48}, " "};
    Text text_graph_19{{152, 152, 8, 48}, " "};
    Text text_graph_20{{160, 152, 8, 48}, " "};
    Text text_graph_21{{168, 152, 8, 48}, " "};
    Text text_graph_22{{176, 152, 8, 48}, " "};
    Text text_graph_23{{184, 152, 8, 48}, " "};
    Text text_graph_24{{192, 152, 8, 48}, " "};
    Text text_graph_25{{200, 152, 8, 48}, " "};
    Text text_graph_26{{208, 152, 8, 48}, " "};
    Text text_graph_27{{216, 152, 8, 48}, " "};
    Text text_graph_28{{224, 152, 8, 48}, " "};
    Text text_graph_29{{232, 152, 8, 48}, " "};

    // ── Row 13: Capture status ────────────────────────────
    Text text_capture_status{
        {0, 202, 240, 14}, "Ready to capture"};

    // ── Row 14: Buttons ───────────────────────────────────
    Button button_capture{
        {0, 218, 8 * 8, 16}, "CAPTURE"};

    Button button_reset_peak{
        {9 * 8, 218, 8 * 8, 16}, "RST PEAK"};

    Button button_audio{
        {18 * 8, 218, 5 * 8, 16}, "BEEP"};

    Button button_back{
        {24 * 8, 218, 5 * 8, 16}, "BACK"};

    // ── RSSI message handler ──────────────────────────────
    MessageHandlerRegistration message_handler_stats{
        Message::ID::RSSIStatistics,
        [this](const Message* const p) {
            const auto msg =
                *reinterpret_cast<const RSSIStatisticsMessage*>(p);
            this->on_statistics_update(msg);
        }};
};

}  // namespace ui::external_app::sigfinder

#endif /*__UI_SIGFINDER_H__*/
