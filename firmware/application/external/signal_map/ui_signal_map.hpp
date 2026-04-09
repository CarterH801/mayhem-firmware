/*
 * Signal Map — External App (stub)
 * File: firmware/application/external/signal_map/ui_signal_map.hpp
 */
#ifndef __UI_SIGNAL_MAP_H__
#define __UI_SIGNAL_MAP_H__
#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
namespace ui {
class SignalMapView : public View {
   public:
    SignalMapView(NavigationView& nav);
    void focus() override;
    std::string title() const override { return "Signal Map"; }
   private:
    NavigationView& nav_;
    Labels label_title{{{6 * 8, 3 * 16}, "Signal Map", Color::white()}};
    Text text_info{{1 * 8, 5 * 16, 28 * 8, 16}, "Requires GeoMap (internal)"};
    Text text_status{{2 * 8, 7 * 16, 26 * 8, 16}, "Coming soon as external app"};
    Button button_back{{9 * 8, 10 * 16, 12 * 8, 2 * 16}, "Back"};
};
}  // namespace ui
#endif
