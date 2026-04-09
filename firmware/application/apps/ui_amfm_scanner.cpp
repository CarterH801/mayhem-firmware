/*
 * AM/FM Band Scanner (stub)
 * File: firmware/application/apps/ui_amfm_scanner.cpp
 */

#include "ui_amfm_scanner.hpp"

namespace ui {

AMFMScannerView::AMFMScannerView(NavigationView& nav)
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

void AMFMScannerView::focus() {
    button_back.focus();
}

}  // namespace ui
