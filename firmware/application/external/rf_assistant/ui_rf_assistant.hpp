/*
 * RF Assistant — Offline Reference System
 * File: firmware/application/apps/ui_rf_assistant.hpp
 *
 * A built-in reference tool with 5 modules:
 *
 *  1. ERROR DECODER  — Paste a Mayhem error code,
 *                      get a plain English fix
 *
 *  2. FREQ LOOKUP    — Type any frequency, get a full
 *                      description of what it is
 *
 *  3. SIGNAL GUIDE   — Step-through decision tree to
 *                      identify unknown signals
 *
 *  4. APP HELP       — Quick reference for every app
 *                      in our suite
 *
 *  5. TROUBLESHOOT   — Top 25 HackRF/PortaPack fixes
 *
 * Everything works completely offline — no internet,
 * no cloud, no external connection needed.
 * All data is compiled into the firmware.
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_RF_ASSISTANT_H__
#define __UI_RF_ASSISTANT_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "string_format.hpp"
#include "portapack.hpp"
#include "ui_bandplan.hpp"

#include <string>
#include <vector>
#include <cstdint>

namespace ui {

// ─────────────────────────────────────────────────────────
// Generic key/value lookup entry
// ─────────────────────────────────────────────────────────
struct RefEntry {
    const char* key;
    const char* title;
    const char* body;        // Can be long — shown paginated
};

// ─────────────────────────────────────────────────────────
// Signal identification decision tree node
// ─────────────────────────────────────────────────────────
struct GuideNode {
    const char* question;
    const char* answer_yes;   // Next node key or "RESULT:xxx"
    const char* answer_no;    // Next node key or "RESULT:xxx"
    const char* result_text;  // Populated if this is a leaf
};

// ─────────────────────────────────────────────────────────
// Main RF Assistant View
// ─────────────────────────────────────────────────────────
class RFAssistantView : public View {
   public:
    RFAssistantView(NavigationView& nav);

    void focus() override;
    std::string title() const override {
        return "RF Assistant";
    }

   private:
    NavigationView& nav_;

    // Current module: 0=home, 1=error, 2=freq,
    //                 3=guide, 4=help, 5=trouble
    int  module_{0};
    int  result_scroll_{0};    // Scroll offset for results
    int  guide_node_{0};       // Current guide tree node
    bool result_showing_{false};

    // Current displayed content
    std::string result_title_{""};
    std::vector<std::string> result_lines_{};

    // ── Data tables ───────────────────────────────────────

    // Error code database
    static const RefEntry ERROR_DB[];
    static const int      ERROR_DB_COUNT;

    // App help database
    static const RefEntry HELP_DB[];
    static const int      HELP_DB_COUNT;

    // Troubleshooting database
    static const RefEntry TROUBLE_DB[];
    static const int      TROUBLE_DB_COUNT;

    // Signal identification guide nodes
    static const GuideNode GUIDE_NODES[];
    static const int       GUIDE_NODE_COUNT;

    // ── Module functions ──────────────────────────────────
    void show_home();
    void show_module(int mod);

    // Error decoder
    void search_error(const std::string& query);

    // Frequency lookup
    void lookup_frequency(uint64_t freq_hz);

    // Signal guide
    void guide_start();
    void guide_answer(bool yes);
    void guide_show_node(int node_idx);

    // App help
    void show_app_help(int idx);

    // Troubleshooting
    void show_trouble(int idx);

    // Generic result display
    void display_result(const std::string& title,
                        const std::string& body);
    void scroll_result(int delta);
    void wrap_text(const std::string& text,
                   int width,
                   std::vector<std::string>& lines) const;
    void update_result_display();

    // ── UI State ──────────────────────────────────────────

    // Home screen — module selector
    // (visible when module_ == 0)

    // Query input (used by error + freq modules)
    std::string input_buf_{""};

    // ── Widgets ───────────────────────────────────────────

    // ── HOME SCREEN ──────────────────────────────────────
    Labels label_title{
        {{0, 0}, "RF ASSISTANT", Color::green()}};
    Labels label_sub{
        {{0, 14}, "Offline Reference System", Color::light_grey()}};

    Button button_error{
        {10, 32, 220, 22}, "1. Error Code Decoder"};
    Button button_freq{
        {10, 58, 220, 22}, "2. Frequency Lookup"};
    Button button_guide{
        {10, 84, 220, 22}, "3. Signal ID Guide"};
    Button button_help{
        {10, 110, 220, 22}, "4. App Help"};
    Button button_trouble{
        {10, 136, 220, 22}, "5. Troubleshooting"};

    Labels label_hint_home{
        {{10, 166},
         "Select a module to get started",
         Color::light_grey()}};

    // ── RESULT SCREEN ─────────────────────────────────────
    // (overlaps home widgets — shown/hidden by module_)

    Text text_result_title{
        {0, 0, 240, 16}, ""};

    // 10 lines of result text
    Text text_r0{{0,  18, 240, 13}, ""};
    Text text_r1{{0,  31, 240, 13}, ""};
    Text text_r2{{0,  44, 240, 13}, ""};
    Text text_r3{{0,  57, 240, 13}, ""};
    Text text_r4{{0,  70, 240, 13}, ""};
    Text text_r5{{0,  83, 240, 13}, ""};
    Text text_r6{{0,  96, 240, 13}, ""};
    Text text_r7{{0, 109, 240, 13}, ""};
    Text text_r8{{0, 122, 240, 13}, ""};
    Text text_r9{{0, 135, 240, 13}, ""};
    Text text_r10{{0, 148, 240, 13}, ""};
    Text text_r11{{0, 161, 240, 13}, ""};

    // Scroll indicator
    Text text_scroll_hint{
        {0, 176, 200, 12}, ""};

    // ── INPUT AREA ────────────────────────────────────────
    Text text_input_label{
        {0, 192, 240, 14}, ""};

    // Frequency entry fields
    NumberField field_freq_mhz{
        {0, 208}, 5, {0, 99999}, 1, ' ', false};
    Labels label_dot{
        {{42, 208}, ".", Color::white()}};
    NumberField field_freq_khz{
        {50, 208}, 3, {0, 999}, 1, '0', false};
    Labels label_mhz_unit{
        {{77, 208}, "MHz", Color::light_grey()}};

    // Guide YES/NO buttons
    Button button_yes{
        {0, 208, 8 * 8, 20}, "YES →"};
    Button button_no{
        {9 * 8, 208, 8 * 8, 20}, "NO →"};

    // List selector for help/trouble modules
    NumberField field_list_idx{
        {0, 208}, 2, {1, 30}, 1, ' ', false};
    Button button_show{
        {3 * 8, 208, 6 * 8, 20}, "SHOW"};

    // Navigation buttons
    Button button_lookup{
        {17 * 8, 208, 7 * 8, 20}, "LOOK UP"};
    Button button_back_result{
        {0, 230, 8 * 8, 16}, "BACK"};
    Button button_scroll_up{
        {9 * 8, 230, 4 * 8, 16}, " ^"};
    Button button_scroll_dn{
        {14 * 8, 230, 4 * 8, 16}, " v"};
    Button button_home{
        {19 * 8, 230, 5 * 8, 16}, "HOME"};
};

}  // namespace ui

#endif /*__UI_RF_ASSISTANT_H__*/
