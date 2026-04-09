/*
 * RF Assistant (stub)
 * File: firmware/application/apps/ui_rf_assistant.cpp
 */

#include "ui_rf_assistant.hpp"

namespace ui {

RFAssistantView::RFAssistantView(NavigationView& nav)
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

void RFAssistantView::focus() {
    button_back.focus();
}

}  // namespace ui
