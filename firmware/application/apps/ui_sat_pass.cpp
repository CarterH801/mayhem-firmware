/*
 * Satellite Pass Detector (stub)
 * File: firmware/application/apps/ui_sat_pass.cpp
 */
#include "ui_sat_pass.hpp"
namespace ui {
SatPassView::SatPassView(NavigationView& nav) : nav_(nav) {
    add_children({&label_title, &text_status, &button_back});
    button_back.on_select = [this](Button&) { nav_.pop(); };
}
void SatPassView::focus() { button_back.focus(); }
}  // namespace ui
