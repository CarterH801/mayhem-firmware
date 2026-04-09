/*
 * Signal Finder (stub)
 * File: firmware/application/apps/ui_sigfinder.hpp
 */

#ifndef __UI_SIGFINDER_H__
#define __UI_SIGFINDER_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"

namespace ui {

class SigFinderView : public View {
   public:
    SigFinderView(NavigationView& nav);

    void focus() override;
    std::string title() const override {
        return "Sig Finder";
    }

   private:
    NavigationView& nav_;

    Labels label_title{
        {{5 * 8, 3 * 16}, "Signal Finder", Color::white()}};

    Text text_status{
        {2 * 8, 6 * 16, 26 * 8, 16}, "Coming soon..."};

    Button button_back{
        {9 * 8, 10 * 16, 12 * 8, 2 * 16}, "Back"};
};

}  // namespace ui

#endif /*__UI_SIGFINDER_H__*/
