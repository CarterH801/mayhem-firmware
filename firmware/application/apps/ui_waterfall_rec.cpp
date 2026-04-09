/*
 * Spectrum Waterfall Recorder (stub)
 * File: firmware/application/apps/ui_waterfall_rec.cpp
 */
#include "ui_waterfall_rec.hpp"
namespace ui {
WaterfallRecView::WaterfallRecView(NavigationView& nav) : nav_(nav) {
    add_children({&label_title, &text_status, &button_back});
    button_back.on_select = [this](Button&) { nav_.pop(); };
}
void WaterfallRecView::focus() { button_back.focus(); }
}  // namespace ui
