/*
 * Hardware Diagnostic Test
 * File: firmware/application/apps/ui_hw_test.hpp
 *
 * Tests all HackRF + PortaPack hardware:
 *
 *  RX PATH TESTS:
 *   - LNA (Low Noise Amplifier) gain stages
 *   - VGA (Variable Gain Amplifier) range
 *   - Built-in AMP (+14dB preamp) on/off
 *   - Noise floor baseline measurement
 *   - RSSI accuracy across gain settings
 *   - Receiver sensitivity estimate
 *
 *  TX PATH TESTS:
 *   - TX output present (loopback test)
 *   - TX power level verification
 *   - Carrier frequency accuracy
 *
 *  SIGNAL PATH:
 *   - SMA antenna port continuity
 *   - Baseband processor communication
 *   - Sample rate clock stability
 *
 *  AUDIO:
 *   - Speaker output test (tone sweep)
 *   - Headphone jack detection
 *   - Audio codec communication
 *   - Microphone input level
 *
 *  HARDWARE I/O:
 *   - SD card read/write speed test
 *   - Display pixel test
 *   - All buttons functional test
 *   - Encoder rotation test
 *   - Touch screen calibration check
 *   - Battery voltage reading
 *   - Temperature sensor reading
 *
 *  RESULTS:
 *   PASS = green checkmark
 *   FAIL = red X with description
 *   WARN = yellow — marginal but working
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_HW_TEST_H__
#define __UI_HW_TEST_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "receiver_model.hpp"
#include "transmitter_model.hpp"
#include "string_format.hpp"
#include "portapack.hpp"
#include "baseband_api.hpp"
#include "audio.hpp"
#include "file.hpp"
#include "rtc_time.hpp"

#include <array>
#include <vector>
#include <string>
#include <cstdint>

namespace ui {

// ─────────────────────────────────────────────────────────
// Test result status
// ─────────────────────────────────────────────────────────
enum class TestStatus {
    PENDING,   // Not run yet
    RUNNING,   // Currently running
    PASS,      // Passed
    WARN,      // Passed with warning
    FAIL,      // Failed
    SKIP       // Skipped (requires hardware not present)
};

// ─────────────────────────────────────────────────────────
// One test result entry
// ─────────────────────────────────────────────────────────
struct TestResult {
    std::string name{""};
    TestStatus  status{TestStatus::PENDING};
    std::string value{""};    // Measured value
    std::string detail{""};   // Pass/fail detail
};

// ─────────────────────────────────────────────────────────
// Hardware Test View
// ─────────────────────────────────────────────────────────
class HWTestView : public View {
   public:
    HWTestView(NavigationView& nav);
    ~HWTestView();

    void focus() override;
    std::string title() const override {
        return "HW Diagnostics";
    }

   private:
    NavigationView& nav_;

    // ── Test state machine ────────────────────────────────
    int     current_test_{-1};
    bool    running_{false};
    int     rssi_samples_{0};
    int32_t rssi_sum_{0};
    int32_t rssi_min_{0};
    int32_t rssi_max_{-120};

    // TX loopback state
    bool    tx_loopback_active_{false};
    int32_t tx_baseline_rssi_{-120};
    int32_t tx_on_rssi_{-120};

    // Button test state
    bool    btn_select_pressed_{false};
    bool    btn_encoder_moved_{false};

    // Audio test state
    int     audio_tone_idx_{0};
    uint32_t audio_counter_{0};

    // SD test state
    bool    sd_write_ok_{false};
    bool    sd_read_ok_{false};
    uint32_t sd_write_speed_{0};

    // ── Test definitions ──────────────────────────────────
    static constexpr int TOTAL_TESTS = 18;
    std::array<TestResult, TOTAL_TESTS> results_{};

    void init_tests();
    void run_all_tests();
    void run_next_test();
    void run_test(int idx);
    void finish_test(int idx,
                     TestStatus status,
                     const std::string& value,
                     const std::string& detail);

    // Individual test functions
    void test_baseband_comms();
    void test_lna_range();
    void test_vga_range();
    void test_amp_toggle();
    void test_noise_floor();
    void test_rssi_range();
    void test_rx_sensitivity();
    void test_tx_output();
    void test_tx_power();
    void test_audio_speaker();
    void test_audio_codec();
    void test_sd_write();
    void test_sd_read();
    void test_sd_speed();
    void test_battery_voltage();
    void test_temperature();
    void test_buttons_prompt();
    void test_encoder_prompt();

    // RSSI collection
    void collect_rssi_sample(int32_t rssi);
    int32_t get_avg_rssi() const;

    // Display
    void draw_results();
    void draw_test_row(int idx);
    std::string status_symbol(TestStatus s) const;
    Color       status_color(TestStatus s)  const;

    void on_statistics_update(
        const RSSIStatisticsMessage& message);

    // ── Widgets ───────────────────────────────────────────

    // Header
    Labels label_header{
        {{0, 0}, "HW DIAGNOSTICS", Color::green()}};

    Text text_progress{
        {120, 0, 120, 14}, "Ready"};

    // Overall progress bar
    ProgressBar bar_overall{
        {0, 14, 240, 6}};

    // 14 test result rows (2 columns of 7 each)
    // Left column (tests 0-8)
    Text text_t0 {{0,  22, 118, 13}, ""};
    Text text_t1 {{0,  35, 118, 13}, ""};
    Text text_t2 {{0,  48, 118, 13}, ""};
    Text text_t3 {{0,  61, 118, 13}, ""};
    Text text_t4 {{0,  74, 118, 13}, ""};
    Text text_t5 {{0,  87, 118, 13}, ""};
    Text text_t6 {{0, 100, 118, 13}, ""};
    Text text_t7 {{0, 113, 118, 13}, ""};
    Text text_t8 {{0, 126, 118, 13}, ""};

    // Right column (tests 9-17)
    Text text_t9  {{122,  22, 118, 13}, ""};
    Text text_t10 {{122,  35, 118, 13}, ""};
    Text text_t11 {{122,  48, 118, 13}, ""};
    Text text_t12 {{122,  61, 118, 13}, ""};
    Text text_t13 {{122,  74, 118, 13}, ""};
    Text text_t14 {{122,  87, 118, 13}, ""};
    Text text_t15 {{122, 100, 118, 13}, ""};
    Text text_t16 {{122, 113, 118, 13}, ""};
    Text text_t17 {{122, 126, 118, 13}, ""};

    // Current test detail
    Text text_current{
        {0, 142, 240, 14}, "Press RUN ALL to start"};

    // Detail line for current test value
    Text text_detail{
        {0, 158, 240, 14}, ""};

    // Summary line
    Text text_summary{
        {0, 174, 240, 14}, ""};

    // Warning note
    Text text_warn{
        {0, 190, 240, 12},
        "TX test: keep antenna connected"};

    // Buttons
    Button button_run_all{
        {0, 204, 10 * 8, 18}, "RUN ALL"};

    Button button_run_one{
        {11 * 8, 204, 10 * 8, 18}, "RUN ONE"};

    NumberField field_test_num{
        {22 * 8, 204}, 2, {1, TOTAL_TESTS}, 1, ' ', false};

    Button button_back{
        {0, 224, 8 * 8, 16}, "BACK"};

    Button button_export{
        {9 * 8, 224, 9 * 8, 16}, "SAVE LOG"};

    Labels label_legend{
        {{19 * 8, 226},
         "OK=grn WRN=yel FAIL=red",
         Color::light_grey()}};

    // RSSI handler
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

#endif /*__UI_HW_TEST_H__*/
