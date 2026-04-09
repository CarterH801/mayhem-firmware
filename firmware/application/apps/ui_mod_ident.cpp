/*
 * Modulation Identifier (stub)
 * File: firmware/application/apps/ui_mod_ident.cpp
 */
#include "ui_mod_ident.hpp"
namespace ui {
ModIdentView::ModIdentView(NavigationView& nav) : nav_(nav) {
    add_children({&label_title, &text_status, &button_back});
    button_back.on_select = [this](Button&) { nav_.pop(); };
}
void ModIdentView::focus() { button_back.focus(); }
}  // namespace ui
