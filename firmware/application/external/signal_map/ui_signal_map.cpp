/*
 * Signal Map — External App (stub)
 * File: firmware/application/external/signal_map/ui_signal_map.cpp
 */
#include "ui_signal_map.hpp"
namespace ui {
SignalMapView::SignalMapView(NavigationView& nav) : nav_(nav) {
    add_children({&label_title, &text_info, &text_status, &button_back});
    button_back.on_select = [this](Button&) { nav_.pop(); };
}
void SignalMapView::focus() { button_back.focus(); }
}  // namespace ui
