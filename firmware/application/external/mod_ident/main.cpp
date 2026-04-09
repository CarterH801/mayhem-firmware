#include "ui.hpp"
#include "ui_mod_ident.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::mod_ident {
void initialize_app(ui::NavigationView& nav) {
    nav.push<ModIdentView>();
}
}

extern "C" {
__attribute__((section(".external_app.app_mod_ident.application_information"), used)) application_information_t _application_information_mod_ident = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::mod_ident::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,
    /*.app_name = */ "Mod Ident",
    /*.bitmap_data = */ {0x00,0x00,0xFE,0x7F,0x02,0x40,0x02,0x40,0xF2,0x4F,0x12,0x48,0xF2,0x4F,0x02,0x40,0x02,0x40,0xF2,0x4F,0x12,0x48,0xF2,0x4F,0x02,0x40,0xFE,0x7F,0x00,0x00,0x00,0x00},
    /*.icon_color = */ ui::Color::orange().v,
    /*.menu_location = */ app_location_t::RX,
    /*.desired_menu_position = */ -1,
    /*.m4_app_tag = */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
