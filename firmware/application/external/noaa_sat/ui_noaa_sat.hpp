/*
 * NOAA Weather Satellite Receiver
 * File: firmware/application/apps/ui_noaa_sat.hpp
 *
 * Receives APT (Automatic Picture Transmission) signals
 * from NOAA-15, NOAA-18, NOAA-19 weather satellites
 * orbiting at ~850km altitude.
 *
 * Frequencies:
 *   NOAA-15: 137.620 MHz
 *   NOAA-18: 137.9125 MHz
 *   NOAA-19: 137.100 MHz
 *
 * APT signal: NFM, 4.8 kHz bandwidth, 2400 baud
 * Audio rate: 11025 Hz
 * Line rate:  2 lines/sec
 * Resolution: 4 km/pixel
 *
 * The satellite transmits a 2400-line image taking
 * about 12 minutes — the pass overhead window.
 *
 * What you need:
 *  - HackRF + PortaPack
 *  - Wideband V-dipole or turnstile antenna
 *  - Clear sky view
 *  - ~12 minute satellite pass
 *
 * The decoded image is saved to /NOAA/noaa_YYYYMMDD_HH.pgm
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_NOAA_SAT_H__
#define __UI_NOAA_SAT_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "portapack.hpp"
#include "file.hpp"
#include "rtc_time.hpp"

#include <array>
#include <string>
#include <cstdint>
#include <cmath>

namespace ui {

// ─────────────────────────────────────────────────────────
// NOAA satellite definitions
// ─────────────────────────────────────────────────────────
struct NOAASat {
    const char* name;
    uint64_t    freq_hz;
    const char* status;
};

static const NOAASat NOAA_SATS[] = {
    {"NOAA-15", 137'620'000, "Operational"},
    {"NOAA-18", 137'912'500, "Operational"},
    {"NOAA-19", 137'100'000, "Operational"},
    {"METEOR M2-3", 137'900'000, "Russian LRPT"},
};
static const int NOAA_SAT_COUNT = 4;

// ─────────────────────────────────────────────────────────
// APT decoder state
// APT uses 2400 Hz sync tones and AM audio to encode image
// ─────────────────────────────────────────────────────────
class APTDecoder {
   public:
    static constexpr int  LINE_WIDTH  = 2080; // pixels/line
    static constexpr int  SYNC_A_LEN  = 39;  // sync A bits
    static constexpr int  SYNC_B_LEN  = 39;  // sync B bits
    static constexpr float SAMPLE_RATE = 11025.0f;
    static constexpr float CARRIER_FREQ = 2400.0f;

    void reset() {
        line_count_    = 0;
        pixel_count_   = 0;
        sync_found_    = false;
        am_level_      = 0;
    }

    // Feed one audio sample (normalized -1 to 1)
    // Returns true if a complete line was decoded
    bool feed_sample(float sample);

    // Get current line buffer
    const uint8_t* line_data() const {
        return line_buf_.data();
    }

    int line_count()  const { return line_count_; }
    int pixel_count() const { return pixel_count_; }

   private:
    std::array<uint8_t, LINE_WIDTH> line_buf_{};
    int    line_count_{0};
    int    pixel_count_{0};
    bool   sync_found_{false};
    float  am_level_{0};
    float  carrier_phase_{0};

    // AM envelope detector
    float detect_am(float sample);
};

// ─────────────────────────────────────────────────────────
// NOAA Satellite Receiver View
// ─────────────────────────────────────────────────────────
class NOAASatView : public View {
   public:
    NOAASatView(NavigationView& nav);
    ~NOAASatView();

    void focus() override;
    std::string title() const override {
        return "NOAA Sat RX";
    }

    void paint(Painter& painter) override;

   private:
    NavigationView& nav_;

    int     selected_sat_{0};
    bool    receiving_{false};
    bool    image_started_{false};

    APTDecoder decoder_{};

    // Image preview — scaled to fit 240x120 screen area
    // Full res saved to file
    static constexpr int PREVIEW_W = 240;
    static constexpr int PREVIEW_H = 120;
    static constexpr int IMG_TOP   = 60;

    // Preview buffer — 1 byte per pixel greyscale
    std::array<uint8_t, PREVIEW_W * PREVIEW_H> preview_{};
    int preview_line_{0};
    bool preview_dirty_{false};

    // Output file
    File image_file_{};
    std::string image_filename_{""};
    uint32_t lines_received_{0};

    void start_receive();
    void stop_receive();
    void process_audio_line(const uint8_t* line_data);
    void update_preview(const uint8_t* line_data,
                        int line_num);
    void draw_preview(Painter& painter);
    void open_image_file();
    void write_pgm_header();

    // ── Widgets ───────────────────────────────────────────

    // Satellite selector
    Labels label_sat{
        {{0, 0}, "Satellite:", Color::light_grey()}};
    OptionsField options_sat{
        {11 * 8, 0}, 14,
        {{"NOAA-15 137.62", 0},
         {"NOAA-18 137.91", 1},
         {"NOAA-19 137.10", 2},
         {"METEOR  137.90", 3}}};

    // Status
    Text text_status{
        {0, 16, 240, 14}, "Select satellite and press RX"};

    // Signal strength
    Text text_signal{
        {0, 32, 120, 14}, "Signal: ---"};
    Text text_lines{
        {120, 32, 120, 14}, "Lines: 0"};

    // Progress bar (0-2400 lines = full pass)
    ProgressBar bar_progress{
        {0, 48, 240, 8}};

    // Image preview area (rows 60-180)
    // Drawn directly in paint()

    // Bottom controls
    Text text_filename{
        {0, 182, 240, 12}, "File: ---"};

    Button button_rx{
        {0, 196, 5 * 8, 16}, "RX"};

    Button button_clear{
        {6 * 8, 196, 7 * 8, 16}, "CLR IMG"};

    Button button_back{
        {14 * 8, 196, 8 * 8, 16}, "BACK"};

    Labels label_hint{
        {{0, 214},
         "Need V-dipole/turnstile antenna",
         Color::grey()}};

    Labels label_hint2{
        {{0, 226},
         "Best during satellite overhead pass",
         Color::grey()}};

    // Audio/RSSI handler
    MessageHandlerRegistration message_handler_stats{
        Message::ID::RSSIStatistics,
        [this](const Message* const p) {
            const auto msg =
                *reinterpret_cast<
                    const RSSIStatisticsMessage*>(p);

            if (!receiving_) return;
            const int32_t rssi = msg.statistics.max;
            text_signal.set(
                "Signal:" +
                to_string_dec_int(rssi) + "dBm");

            if (rssi > -90 && !image_started_) {
                image_started_ = true;
                text_status.set(
                    "Receiving! Image building...");
            }
        }};
};

}  // namespace ui

#endif /*__UI_NOAA_SAT_H__*/
