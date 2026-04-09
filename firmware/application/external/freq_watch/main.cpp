#include "ui.hpp"
#include "ui_freq_watch.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::freq_watch {
void initialize_app(ui::NavigationView& nav) {
    nav.push<FreqWatchView>();
}
}

extern "C" {
__attribute__((section(".external_app.app_freq_watch.application_information"), used)) application_information_t _application_information_freq_watch = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::freq_watch::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,
    /*.app_name = */ "Freq Watch",
    /*.bitmap_data = */ {0x00,0x00,0xFC,0x3F,0x04,0x20,0xF4,0x2F,0x14,0x28,0xF4,0x2F,0x04,0x20,0xFC,0x3F,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x00,0x00,0x00,0x00},
    /*.icon_color = */ ui::Color::red().v,
    /*.menu_location = */ app_location_t::RX,
    /*.desired_menu_position = */ -1,
    /*.m4_app_tag = */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
