/*
 * TPMS Vehicle Counter
 * File: firmware/application/apps/ui_tpms_counter.hpp
 *
 * Every post-2008 car broadcasts TPMS (Tire Pressure
 * Monitoring System) data on 315 MHz (US) or 433 MHz (EU).
 * Each sensor has a unique 32-bit ID — so we can count
 * unique vehicles passing by.
 *
 * What you see:
 *  - Total unique vehicles detected
 *  - Each vehicle's sensor ID, pressure, temperature
 *  - Signal strength (how far away)
 *  - First seen / last seen timestamps
 *  - US/EU band selector
 *
 * Practical uses:
 *  - Traffic counting
 *  - See how many cars are parked near you
 *  - Interesting RF activity demonstration
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_TPMS_COUNTER_H__
#define __UI_TPMS_COUNTER_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "portapack.hpp"
#include "rtc_time.hpp"

#include <vector>
#include <string>
#include <cstdint>

namespace ui {

// ─────────────────────────────────────────────────────────
// One detected vehicle's TPMS data
// ─────────────────────────────────────────────────────────
struct TPMSVehicle {
    uint32_t sensor_id{0};
    uint8_t  tire_id{0};         // 0=FL 1=FR 2=RL 3=RR
    uint8_t  pressure_psi{0};    // Tire pressure
    int8_t   temp_f{0};          // Temperature °F
    int32_t  rssi_dbm{-120};
    uint32_t first_seen{0};      // RTC timestamp
    uint32_t last_seen{0};
    uint32_t hit_count{0};
};

class TPMSCounterView : public View {
   public:
    TPMSCounterView(NavigationView& nav);
    ~TPMSCounterView();

    void focus() override;
    std::string title() const override {
        return "TPMS Vehicle Counter";
    }

   private:
    NavigationView& nav_;

    static constexpr int MAX_VEHICLES = 20;
    static constexpr uint64_t TPMS_US_FREQ = 315'000'000;
    static constexpr uint64_t TPMS_EU_FREQ = 433'920'000;
    static constexpr uint32_t SENSOR_TIMEOUT_SEC = 300; // 5 min

    std::vector<TPMSVehicle> vehicles_{};
    bool watching_{false};
    int  list_offset_{0};
    int  band_mode_{0};  // 0=US 315MHz, 1=EU 433MHz

    void start_watch();
    void stop_watch();
    void process_packet(uint64_t data, int32_t rssi);
    void add_or_update_vehicle(const TPMSVehicle& v);
    void cleanup_old_vehicles();
    void draw_list();
    std::string tire_name(uint8_t id) const;

    void on_statistics_update(
        const RSSIStatisticsMessage& message);

    // Raw bit buffer for packet decoding
    static constexpr int BUF_LEN = 128;
    uint8_t  bit_buf_[BUF_LEN]{};
    int      bit_idx_{0};
    uint32_t last_edge_time_{0};

    // ── Widgets ───────────────────────────────────────────

    // Header stats
    Text text_total{
        {0, 0, 120, 16}, "Vehicles: 0"};
    Text text_band{
        {120, 0, 120, 16}, "315 MHz (US)"};

    // Column headers
    Labels label_cols{
        {{0, 18},
         "ID        TIRE  PSI  TEMP  SIG  HITS",
         Color::light_grey()}};

    // 8 vehicle rows
    Text text_v0{{0,  32, 240, 12}, ""};
    Text text_v1{{0,  44, 240, 12}, ""};
    Text text_v2{{0,  56, 240, 12}, ""};
    Text text_v3{{0,  68, 240, 12}, ""};
    Text text_v4{{0,  80, 240, 12}, ""};
    Text text_v5{{0,  92, 240, 12}, ""};
    Text text_v6{{0, 104, 240, 12}, ""};
    Text text_v7{{0, 116, 240, 12}, ""};

    // Scroll
    Text text_scroll{
        {0, 130, 240, 12}, ""};

    // Live decode display
    Labels label_last{
        {{0, 144}, "Last packet:", Color::light_grey()}};
    Text text_last_packet{
        {0, 158, 240, 14}, "Waiting..."};

    // Status
    Text text_status{
        {0, 174, 240, 14},
        "Press WATCH to start"};

    // Buttons
    Button button_watch{
        {0, 190, 7 * 8, 16}, "WATCH"};

    OptionsField options_band{
        {8 * 8, 190}, 12,
        {{"315MHz (US)", 0},
         {"433MHz (EU)", 1}}};

    Button button_clear{
        {21 * 8, 190, 7 * 8, 16}, "CLEAR"};

    Button button_up{
        {0, 208, 3 * 8, 16}, " ^"};
    Button button_down{
        {4 * 8, 208, 3 * 8, 16}, " v"};
    Button button_back{
        {8 * 8, 208, 8 * 8, 16}, "BACK"};

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

#endif /*__UI_TPMS_COUNTER_H__*/
