/*
 * Modulation Identifier (stub)
 * File: firmware/application/apps/ui_mod_ident.hpp
 */
#ifndef __UI_MOD_IDENT_H__
#define __UI_MOD_IDENT_H__
#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
namespace ui {
class ModIdentView : public View {
   public:
    ModIdentView(NavigationView& nav);
    void focus() override;
    std::string title() const override { return "Mod Ident"; }
   private:
    NavigationView& nav_;
    Labels label_title{{{3 * 8, 3 * 16}, "Modulation Identifier", Color::white()}};
    Text text_status{{2 * 8, 6 * 16, 26 * 8, 16}, "Coming soon..."};
    Button button_back{{9 * 8, 10 * 16, 12 * 8, 2 * 16}, "Back"};
};
}  // namespace ui
#endif
