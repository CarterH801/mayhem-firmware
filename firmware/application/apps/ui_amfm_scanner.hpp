/*
 * AM/FM Band Scanner (stub)
 * File: firmware/application/apps/ui_amfm_scanner.hpp
 */

#ifndef __UI_AMFM_SCANNER_H__
#define __UI_AMFM_SCANNER_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"

namespace ui {

class AMFMScannerView : public View {
   public:
    AMFMScannerView(NavigationView& nav);

    void focus() override;
    std::string title() const override {
        return "RF Scanner";
    }

   private:
    NavigationView& nav_;

    Labels label_title{
        {{5 * 8, 3 * 16}, "RF Scanner", Color::white()}};

    Text text_status{
        {2 * 8, 6 * 16, 26 * 8, 16}, "Coming soon..."};

    Button button_back{
        {9 * 8, 10 * 16, 12 * 8, 2 * 16}, "Back"};
};

}  // namespace ui

#endif /*__UI_AMFM_SCANNER_H__*/
