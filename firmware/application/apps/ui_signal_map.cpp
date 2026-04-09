/*
 * Signal Map (stub)
 * File: firmware/application/apps/ui_signal_map.cpp
 */
#include "ui_signal_map.hpp"
namespace ui {
SignalMapView::SignalMapView(NavigationView& nav) : nav_(nav) {
    add_children({&label_title, &text_status, &button_back});
    button_back.on_select = [this](Button&) { nav_.pop(); };
}
void SignalMapView::focus() { button_back.focus(); }
}  // namespace ui
