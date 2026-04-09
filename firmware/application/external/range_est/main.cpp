#include "ui.hpp"
#include "ui_range_est.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::range_est {
void initialize_app(ui::NavigationView& nav) {
    nav.push<RangeEstView>();
}
}

extern "C" {
__attribute__((section(".external_app.app_range_est.application_information"), used)) application_information_t _application_information_range_est = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::range_est::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,
    /*.app_name = */ "Range Est",
    /*.bitmap_data = */ {0x00,0x00,0x80,0x01,0x80,0x01,0xC0,0x03,0xC0,0x03,0xE0,0x07,0xE0,0x07,0xF0,0x0F,0xF0,0x0F,0xF8,0x1F,0x00,0x00,0x24,0x24,0x24,0x24,0x00,0x00,0x00,0x00,0x00,0x00},
    /*.icon_color = */ ui::Color::cyan().v,
    /*.menu_location = */ app_location_t::RX,
    /*.desired_menu_position = */ -1,
    /*.m4_app_tag = */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
