#include "ui.hpp"
#include "ui_amfm_scanner.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::amfm_scanner {
void initialize_app(ui::NavigationView& nav) {
    nav.push<AMFMScannerView>();
}
}

extern "C" {
__attribute__((section(".external_app.app_amfm_scanner.application_information"), used)) application_information_t _application_information_amfm_scanner = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::amfm_scanner::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,
    /*.app_name = */ "RF Scanner",
    /*.bitmap_data = */ {0x00,0x00,0xFE,0x7F,0x03,0xC0,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x03,0xC0,0xFE,0x7F,0x00,0x00,0x18,0x18,0x3C,0x3C,0x7E,0x7E,0xFF,0xFF,0x00,0x00,0x00,0x00},
    /*.icon_color = */ ui::Color::green().v,
    /*.menu_location = */ app_location_t::RX,
    /*.desired_menu_position = */ -1,
    /*.m4_app_tag = */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
