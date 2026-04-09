/*
 * Frequency Watch (stub)
 * File: firmware/application/apps/ui_freq_watch.hpp
 */
#ifndef __UI_FREQ_WATCH_H__
#define __UI_FREQ_WATCH_H__
#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
namespace ui {
class FreqWatchView : public View {
   public:
    FreqWatchView(NavigationView& nav);
    void focus() override;
    std::string title() const override { return "Freq Watch"; }
   private:
    NavigationView& nav_;
    Labels label_title{{{4 * 8, 3 * 16}, "Frequency Watch", Color::white()}};
    Text text_status{{2 * 8, 6 * 16, 26 * 8, 16}, "Coming soon..."};
    Button button_back{{9 * 8, 10 * 16, 12 * 8, 2 * 16}, "Back"};
};
}  // namespace ui
#endif
