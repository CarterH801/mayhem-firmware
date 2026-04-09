#include "ui.hpp"
#include "ui_sigfinder.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::sigfinder {
void initialize_app(ui::NavigationView& nav) {
    nav.push<SigFinderView>();
}
}

extern "C" {
__attribute__((section(".external_app.app_sigfinder.application_information"), used)) application_information_t _application_information_sigfinder = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::sigfinder::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,
    /*.app_name = */ "Sig Finder",
    /*.bitmap_data = */ {0x00,0x00,0x00,0x00,0xE0,0x07,0x10,0x08,0x08,0x10,0x08,0x10,0x08,0x10,0x10,0x08,0xE0,0x07,0x00,0x01,0x80,0x02,0x40,0x04,0x20,0x08,0x10,0x10,0x00,0x00,0x00,0x00},
    /*.icon_color = */ ui::Color::yellow().v,
    /*.menu_location = */ app_location_t::RX,
    /*.desired_menu_position = */ -1,
    /*.m4_app_tag = */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
