#include "ui.hpp"
#include "ui_tpms_counter.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::tpms_counter {
void initialize_app(ui::NavigationView& nav) {
    nav.push<TPMSCounterView>();
}
}

extern "C" {
__attribute__((section(".external_app.app_tpms_counter.application_information"), used)) application_information_t _application_information_tpms_counter = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::tpms_counter::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,
    /*.app_name = */ "TPMS Count",
    /*.bitmap_data = */ {0x00,0x00,0xE0,0x07,0x10,0x08,0x08,0x10,0xC4,0x23,0xE4,0x27,0xC4,0x23,0x08,0x10,0x10,0x08,0xE0,0x07,0x00,0x00,0xE0,0x07,0x10,0x08,0xE0,0x07,0x00,0x00,0x00,0x00},
    /*.icon_color = */ ui::Color::white().v,
    /*.menu_location = */ app_location_t::RX,
    /*.desired_menu_position = */ -1,
    /*.m4_app_tag = */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
