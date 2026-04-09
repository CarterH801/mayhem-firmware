/*
 * NOAA Weather Satellite Receiver (stub)
 * File: firmware/application/apps/ui_noaa_sat.cpp
 */

#include "ui_noaa_sat.hpp"

namespace ui {

NOAASatView::NOAASatView(NavigationView& nav)
    : nav_(nav) {

    add_children({
        &label_title,
        &label_info,
        &label_info2,
        &label_info3,
        &text_status,
        &button_back,
    });

    button_back.on_select = [this](Button&) {
        nav_.pop();
    };
}

void NOAASatView::focus() {
    button_back.focus();
}

}  // namespace ui
