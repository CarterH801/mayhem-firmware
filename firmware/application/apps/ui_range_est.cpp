/*
 * Range Estimator (stub)
 * File: firmware/application/apps/ui_range_est.cpp
 */

#include "ui_range_est.hpp"

namespace ui {

RangeEstView::RangeEstView(NavigationView& nav)
    : nav_(nav) {

    add_children({
        &label_title,
        &text_status,
        &button_back,
    });

    button_back.on_select = [this](Button&) {
        nav_.pop();
    };
}

void RangeEstView::focus() {
    button_back.focus();
}

}  // namespace ui
