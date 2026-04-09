/*
 * Hardware Diagnostic Test — External App (stub)
 * File: firmware/application/external/hw_test/ui_hw_test.hpp
 */
#ifndef __UI_HW_TEST_H__
#define __UI_HW_TEST_H__
#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
namespace ui {
class HWTestView : public View {
   public:
    HWTestView(NavigationView& nav);
    void focus() override;
    std::string title() const override { return "HW Diagnostics"; }
   private:
    NavigationView& nav_;
    Labels label_title{{{3 * 8, 3 * 16}, "HW Diagnostics", Color::white()}};
    Text text_info{{1 * 8, 5 * 16, 28 * 8, 16}, "Uses internal APIs only"};
    Text text_status{{2 * 8, 7 * 16, 26 * 8, 16}, "Use built-in Debug menu"};
    Button button_back{{9 * 8, 10 * 16, 12 * 8, 2 * 16}, "Back"};
};
}  // namespace ui
#endif
