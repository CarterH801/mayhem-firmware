/*
 * Frequency Watch / Alert
 * File: firmware/application/apps/ui_freq_watch.hpp
 *
 * Watches up to 8 frequencies silently in the background.
 * Alerts (beep + flash) when any watched frequency
 * goes above its set threshold.
 *
 * Like a tripwire for RF signals.
 * Use cases:
 *   - Alert when a drone frequency goes active
 *   - Alert when a key fob is pressed nearby
 *   - Monitor a weather radio channel
 *   - Watch for any RF activity on a custom frequency
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_FREQ_WATCH_H__
#define __UI_FREQ_WATCH_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "portapack.hpp"
#include "ui_bandplan.hpp"
#include "baseband_api.hpp"

#include <array>
#include <string>
#include <cstdint>

using namespace ui;

namespace ui::external_app::freq_watch {

// ─────────────────────────────────────────────────────────
// One watched frequency entry
// ─────────────────────────────────────────────────────────
struct WatchEntry {
    rf::Frequency freq{0};
    int32_t       threshold_dbm{-75};  // Alert above this
    std::string   label{""};
    bool          enabled{true};
    bool          triggered{false};    // Currently active?
    uint32_t      trigger_count{0};    // Times triggered
    int32_t       last_rssi{-120};
};

class FreqWatchView : public View {
   public:
    FreqWatchView(NavigationView& nav);
    ~FreqWatchView();

    void focus() override;
    std::string title() const override {
        return "Freq Watch";
    }

   private:
    static constexpr int MAX_WATCH = 8;

    NavigationView& nav_;

    std::array<WatchEntry, MAX_WATCH> entries_{};
    int     selected_entry_{0};
    int     current_scan_idx_{0};  // Which entry we're listening to
    bool    watching_{false};
    uint32_t dwell_counter_{0};
    static constexpr uint32_t DWELL_TICKS = 3; // 300ms per freq

    // Alert state
    bool    alert_active_{false};
    int     alert_entry_{-1};
    uint32_t alert_flash_counter_{0};

    void start_watch();
    void stop_watch();
    void advance_scan();
    void check_and_alert(int entry_idx, int32_t rssi);
    void trigger_alert(int entry_idx);
    void clear_alert();
    void tune_to_entry(int idx);
    void draw_entry_row(int idx);
    void update_all_rows();
    std::string format_freq(rf::Frequency f) const;

    void on_statistics_update(
        const RSSIStatisticsMessage& message);

    // ── Widgets ───────────────────────────────────────────

    // Title row
    Labels label_header{
        {{0, 0}, "FREQ WATCH — RF TRIPWIRE", Color::green()}};

    // 8 watch entry rows
    // Each row: [EN] FREQ      LABEL     THR  RSSI  CNT
    Text text_e0{{0,  16, 240, 14}, ""};
    Text text_e1{{0,  30, 240, 14}, ""};
    Text text_e2{{0,  44, 240, 14}, ""};
    Text text_e3{{0,  58, 240, 14}, ""};
    Text text_e4{{0,  72, 240, 14}, ""};
    Text text_e5{{0,  86, 240, 14}, ""};
    Text text_e6{{0, 100, 240, 14}, ""};
    Text text_e7{{0, 114, 240, 14}, ""};

    // Alert banner (hidden until triggered)
    Text text_alert{
        {0, 130, 240, 20}, ""};

    // Status
    Text text_status{
        {0, 152, 240, 14}, "Press WATCH to start monitoring"};

    // Scan indicator — shows which freq is being checked
    Text text_scanning{
        {0, 168, 240, 14}, ""};

    // Frequency input for selected entry
    Labels label_freq{
        {{0, 184}, "Freq:", Color::light_grey()}};

    NumberField field_freq_mhz{
        {6 * 8, 184}, 4, {1, 5800}, 1, ' ', false};

    Labels label_dot{
        {{11 * 8, 184}, ".", Color::white()}};

    NumberField field_freq_khz{
        {12 * 8, 184}, 3, {0, 999}, 1, '0', false};

    Labels label_mhz{
        {{16 * 8, 184}, "MHz", Color::light_grey()}};

    // Threshold input
    Labels label_thr{
        {{20 * 8, 184}, "SQ:", Color::light_grey()}};

    NumberField field_threshold{
        {24 * 8, 184}, 4, {-120, -20}, 1, ' ', false};

    // Buttons
    Button button_set{
        {0, 200, 5 * 8, 16}, "SET"};

    Button button_clear_entry{
        {6 * 8, 200, 7 * 8, 16}, "CLR SLOT"};

    Button button_watch{
        {14 * 8, 200, 7 * 8, 16}, "WATCH"};

    Button button_back{
        {22 * 8, 200, 8 * 8, 16}, "BACK"};

    // Entry selector
    Button button_prev{
        {0, 216, 3 * 8, 16}, " <"};
    Text text_sel{
        {4 * 8, 216, 3 * 8, 14}, "#1"};
    Button button_next{
        {8 * 8, 216, 3 * 8, 16}, " >"};

    Labels label_hint{
        {{12 * 8, 218},
         "Use </> to select slot",
         Color::light_grey()}};

    // ── RSSI handler ──────────────────────────────────────
    MessageHandlerRegistration message_handler_stats{
        Message::ID::RSSIStatistics,
        [this](const Message* const p) {
            const auto msg =
                *reinterpret_cast<
                    const RSSIStatisticsMessage*>(p);
            this->on_statistics_update(msg);
        }};
};

}  // namespace ui::external_app::freq_watch

#endif /*__UI_FREQ_WATCH_H__*/
