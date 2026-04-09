/*
 * TPMS Vehicle Counter (stub)
 * File: firmware/application/apps/ui_tpms_counter.cpp
 */
#include "ui_tpms_counter.hpp"
namespace ui {
TPMSCounterView::TPMSCounterView(NavigationView& nav) : nav_(nav) {
    add_children({&label_title, &text_status, &button_back});
    button_back.on_select = [this](Button&) { nav_.pop(); };
}
void TPMSCounterView::focus() { button_back.focus(); }
}  // namespace ui
