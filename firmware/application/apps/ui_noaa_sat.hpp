/*
 * NOAA Weather Satellite Receiver (stub)
 * File: firmware/application/apps/ui_noaa_sat.hpp
 */

#ifndef __UI_NOAA_SAT_H__
#define __UI_NOAA_SAT_H__

#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"

namespace ui {

class NOAASatView : public View {
   public:
    NOAASatView(NavigationView& nav);

    void focus() override;
    std::string title() const override {
        return "NOAA Sat RX";
    }

   private:
    NavigationView& nav_;

    Labels label_title{
        {{2 * 8, 3 * 16}, "NOAA Satellite Receiver", Color::white()}};

    Labels label_info{
        {{3 * 8, 5 * 16}, "NOAA-15  137.620 MHz", Color::light_grey()}};
    Labels label_info2{
        {{3 * 8, 6 * 16}, "NOAA-18  137.913 MHz", Color::light_grey()}};
    Labels label_info3{
        {{3 * 8, 7 * 16}, "NOAA-19  137.100 MHz", Color::light_grey()}};

    Text text_status{
        {2 * 8, 9 * 16, 26 * 8, 16}, "Coming soon..."};

    Button button_back{
        {9 * 8, 12 * 16, 12 * 8, 2 * 16}, "Back"};
};

}  // namespace ui

#endif /*__UI_NOAA_SAT_H__*/
