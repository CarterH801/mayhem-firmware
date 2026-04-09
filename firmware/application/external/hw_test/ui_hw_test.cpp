/*
 * Hardware Diagnostic Test — External App (stub)
 * File: firmware/application/external/hw_test/ui_hw_test.cpp
 */
#include "ui_hw_test.hpp"
namespace ui {
HWTestView::HWTestView(NavigationView& nav) : nav_(nav) {
    add_children({&label_title, &text_info, &text_status, &button_back});
    button_back.on_select = [this](Button&) { nav_.pop(); };
}
void HWTestView::focus() { button_back.focus(); }
}  // namespace ui
