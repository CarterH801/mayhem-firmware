/*
 * Satellite Pass Detector (stub)
 * File: firmware/application/apps/ui_sat_pass.hpp
 */
#ifndef __UI_SAT_PASS_H__
#define __UI_SAT_PASS_H__
#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
namespace ui {
class SatPassView : public View {
   public:
    SatPassView(NavigationView& nav);
    void focus() override;
    std::string title() const override { return "Sat Pass"; }
   private:
    NavigationView& nav_;
    Labels label_title{{{5 * 8, 3 * 16}, "Sat Pass Detect", Color::white()}};
    Text text_status{{2 * 8, 6 * 16, 26 * 8, 16}, "Coming soon..."};
    Button button_back{{9 * 8, 10 * 16, 12 * 8, 2 * 16}, "Back"};
};
}  // namespace ui
#endif
