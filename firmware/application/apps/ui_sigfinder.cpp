/*
 * Signal Finder (stub)
 * File: firmware/application/apps/ui_sigfinder.cpp
 */

#include "ui_sigfinder.hpp"

namespace ui {

SigFinderView::SigFinderView(NavigationView& nav)
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

void SigFinderView::focus() {
    button_back.focus();
}

}  // namespace ui
