/*
 * TPMS Vehicle Counter (stub)
 * File: firmware/application/apps/ui_tpms_counter.hpp
 */
#ifndef __UI_TPMS_COUNTER_H__
#define __UI_TPMS_COUNTER_H__
#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
namespace ui {
class TPMSCounterView : public View {
   public:
    TPMSCounterView(NavigationView& nav);
    void focus() override;
    std::string title() const override { return "TPMS Count"; }
   private:
    NavigationView& nav_;
    Labels label_title{{{4 * 8, 3 * 16}, "TPMS Counter", Color::white()}};
    Text text_status{{2 * 8, 6 * 16, 26 * 8, 16}, "Coming soon..."};
    Button button_back{{9 * 8, 10 * 16, 12 * 8, 2 * 16}, "Back"};
};
}  // namespace ui
#endif
