/*
 * Modulation Identifier
 * File: firmware/application/apps/ui_mod_ident.hpp
 *
 * Analyzes a received signal and identifies its modulation:
 *   AM / FM / NFM / WFM / FSK / OOK / PSK / USB / LSB / CW
 *
 * Method: Analyzes amplitude variance, frequency deviation,
 * zero-crossing rate, and spectral shape to classify signal.
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_MOD_IDENT_H__
#define __UI_MOD_IDENT_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "portapack.hpp"
#include "ui_bandplan.hpp"

#include <array>
#include <string>
#include <cstdint>
#include <cmath>

namespace ui {

// ─────────────────────────────────────────────────────────
// Modulation analysis result
// ─────────────────────────────────────────────────────────
struct ModResult {
    std::string mod_type{""};       // "FM" "AM" "OOK" etc.
    std::string confidence{""};     // "HIGH" "MEDIUM" "LOW"
    std::string description{""};    // Human readable
    std::string likely_device{""};  // What device uses this
    int         confidence_pct{0};
};

class ModIdentView : public View {
   public:
    ModIdentView(NavigationView& nav);
    ModIdentView(NavigationView& nav,
                 rf::Frequency freq,
                 const std::string& band_label);
    ~ModIdentView();

    void focus() override;
    std::string title() const override {
        return "Mod Identifier";
    }

   private:
    void init(rf::Frequency freq,
              const std::string& band_label);

    NavigationView& nav_;
    rf::Frequency  target_freq_{315'000'000};
    std::string    band_label_{"---"};

    // ── Sample collection ─────────────────────────────────
    static constexpr int SAMPLE_COUNT = 100;
    std::array<int32_t, SAMPLE_COUNT> rssi_samples_{};
    int     sample_idx_{0};
    bool    samples_full_{false};
    bool    analyzing_{false};

    // ── Analysis state ────────────────────────────────────
    ModResult result_{};

    // ── Analysis functions ────────────────────────────────
    void collect_sample(int32_t rssi);
    ModResult analyze_samples() const;

    // Statistical helpers
    float calc_mean(const int32_t* data, int n) const;
    float calc_variance(const int32_t* data,
                        int n, float mean) const;
    int   calc_zero_crossings(const int32_t* data,
                               int n, float mean) const;
    float calc_am_index(const int32_t* data,
                        int n) const;

    void update_display();
    std::string format_freq(rf::Frequency f) const;

    void on_statistics_update(
        const RSSIStatisticsMessage& message);

    // ── Widgets ───────────────────────────────────────────

    Text text_freq{
        {0, 0, 240, 16}, "---"};
    Text text_band{
        {0, 16, 240, 14}, "---"};

    // Analysis progress
    Labels label_progress{
        {{0, 32}, "Collecting samples:", Color::light_grey()}};
    ProgressBar bar_progress{
        {0, 48, 240, 10}};

    // Result display
    Labels label_result{
        {{0, 64}, "Modulation:", Color::light_grey()}};
    Text text_mod_type{
        {12 * 8, 64, 120, 20}, "---"};

    Labels label_conf{
        {{0, 86}, "Confidence:", Color::light_grey()}};
    Text text_confidence{
        {12 * 8, 86, 100, 14}, "---"};
    ProgressBar bar_confidence{
        {0, 102, 240, 10}};

    Labels label_desc{
        {{0, 116}, "Description:", Color::light_grey()}};
    Text text_description{
        {0, 132, 240, 14}, "---"};

    Labels label_device{
        {{0, 148}, "Likely device:", Color::light_grey()}};
    Text text_device{
        {0, 164, 240, 14}, "---"};

    // Signal stats
    Labels label_stats_h{
        {{0, 180}, "Amplitude Var  Zero-Cross  AM-Idx",
         Color::grey()}};
    Text text_stats{
        {0, 194, 240, 14}, "---"};

    // Buttons
    Button button_analyze{
        {0, 212, 10 * 8, 16}, "ANALYZE"};
    Button button_back{
        {11 * 8, 212, 8 * 8, 16}, "BACK"};

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

#endif /*__UI_MOD_IDENT_H__*/
