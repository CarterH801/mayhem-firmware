/*
 * Spectrum Waterfall Recorder
 * File: firmware/application/apps/ui_waterfall_rec.hpp
 *
 * Records a full spectrum sweep to SD card every N seconds
 * for hours, creating a time-lapse of RF activity.
 *
 * Saved as: /WATERFALL/wfall_YYYYMMDD_HH.csv
 * Format:   timestamp, freq_start_mhz, freq_end_mhz,
 *           rssi_0, rssi_1, ... rssi_N (one per step)
 *
 * Play back on PC using included Python script or
 * import into any spreadsheet for analysis.
 *
 * Use cases:
 *  - See which frequencies are busy at which times of day
 *  - Track drone activity patterns
 *  - Spot interference sources
 *  - Monitor key fob usage in a parking lot
 *  - Visualize RF environment changes over time
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_WATERFALL_REC_H__
#define __UI_WATERFALL_REC_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "portapack.hpp"
#include "file.hpp"
#include "rtc_time.hpp"
#include "ui_bandplan.hpp"

#include <vector>
#include <string>
#include <cstdint>

namespace ui {

class WaterfallRecView : public View {
   public:
    WaterfallRecView(NavigationView& nav);
    ~WaterfallRecView();

    void focus() override;
    std::string title() const override {
        return "Waterfall Recorder";
    }

    // Custom paint for waterfall display
    void paint(Painter& painter) override;

   private:
    NavigationView& nav_;

    // ── Scan parameters ───────────────────────────────────
    uint64_t scan_start_{87'500'000};   // Hz
    uint64_t scan_end_{108'000'000};    // Hz
    uint32_t scan_step_{200'000};       // Hz
    uint32_t interval_sec_{30};         // Record every N sec

    // ── Current scan state ────────────────────────────────
    uint64_t current_freq_{0};
    bool     recording_{false};
    bool     scan_active_{false};
    uint32_t sweep_count_{0};
    uint32_t last_sweep_time_{0};

    // ── RSSI samples for current sweep ───────────────────
    static constexpr int MAX_STEPS = 120;
    int32_t  sweep_buf_[MAX_STEPS]{};
    int      sweep_idx_{0};
    int      sweep_total_steps_{0};

    // ── Waterfall display buffer (last 60 sweeps) ─────────
    // Each row = one sweep, each column = one frequency step
    static constexpr int WFALL_ROWS = 60;
    static constexpr int WFALL_COLS = 120;
    int32_t  wfall_buf_[WFALL_ROWS][WFALL_COLS]{};
    int      wfall_row_{0};
    bool     wfall_dirty_{false};

    // ── File output ───────────────────────────────────────
    File  log_file_{};
    std::string log_filename_{""};
    bool  file_open_{false};

    // ── Preset bands ─────────────────────────────────────
    struct BandPreset {
        const char* name;
        uint64_t    start;
        uint64_t    end;
        uint32_t    step;
    };

    static const BandPreset PRESETS[];
    static const int        PRESET_COUNT;

    void start_recording();
    void stop_recording();
    void start_sweep();
    void next_step();
    void finish_sweep();
    void write_sweep_to_file();
    void open_log_file();
    void draw_waterfall(Painter& painter);
    Color rssi_to_color(int32_t rssi) const;

    void on_statistics_update(
        const RSSIStatisticsMessage& message);

    // ── Widgets ───────────────────────────────────────────

    // Band preset selector
    Labels label_band{
        {{0, 0}, "Band:", Color::light_grey()}};
    OptionsField options_band{
        {6 * 8, 0}, 14,
        {{"FM 87-108MHz  ", 0},
         {"VHF 136-175MHz", 1},
         {"KFOB 313-317M ", 2},
         {"UHF 430-440MHz", 3},
         {"DRONE 900-928M", 4},
         {"Custom        ", 5}}};

    // Interval selector
    Labels label_interval{
        {{0, 16}, "Every:", Color::light_grey()}};
    OptionsField options_interval{
        {7 * 8, 16}, 8,
        {{"10 sec ", 0},
         {"30 sec ", 1},
         {"1 min  ", 2},
         {"5 min  ", 3}}};

    // Status
    Text text_status{
        {0, 32, 240, 14},
        "Press REC to start recording"};

    // Sweep stats
    Text text_sweeps{
        {0, 48, 120, 12}, "Sweeps: 0"};
    Text text_file_size{
        {120, 48, 120, 12}, "File: ---"};

    // Waterfall display area (rows 62-182)
    // Drawn directly in paint()
    Labels label_wfall{
        {{0, 62}, "LIVE WATERFALL:", Color::light_grey()}};

    // Scale (bottom of waterfall)
    Text text_scale_l{
        {0, 184, 80, 12}, ""};
    Text text_scale_r{
        {160, 184, 80, 12}, ""};

    // Filename display
    Text text_filename{
        {0, 198, 240, 12}, "File: not recording"};

    // Buttons
    Button button_rec{
        {0, 212, 6 * 8, 16}, "REC"};
    Button button_clear{
        {7 * 8, 212, 7 * 8, 16}, "CLR VIEW"};
    Button button_back{
        {15 * 8, 212, 8 * 8, 16}, "BACK"};

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

#endif /*__UI_WATERFALL_REC_H__*/
