/*
 * Hardware Diagnostic Test (stub)
 * File: firmware/application/apps/ui_hw_test.hpp
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
    std::string title() const override {
        return "HW Diagnostics";
    }

   private:
    NavigationView& nav_;

    Labels label_title{
        {{4 * 8, 3 * 16}, "HW Diagnostics", Color::white()}};

    Text text_status{
        {2 * 8, 6 * 16, 26 * 8, 16}, "Coming soon..."};

    Button button_back{
        {9 * 8, 10 * 16, 12 * 8, 2 * 16}, "Back"};
};

}  // namespace ui

#endif /*__UI_HW_TEST_H__*/
