#include "ui.hpp"
#include "ui_noaa_sat.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::noaa_sat {
void initialize_app(ui::NavigationView& nav) {
    nav.push<NOAASatView>();
}
}  // namespace ui::external_app::noaa_sat

extern "C" {

__attribute__((section(".external_app.app_noaa_sat.application_information"), used)) application_information_t _application_information_noaa_sat = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::noaa_sat::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,

    /*.app_name = */ "NOAA Sat RX",
    /*.bitmap_data = */ {
        0x00, 0x00, 0x80, 0x01, 0x40, 0x02, 0x20, 0x04,
        0x10, 0x08, 0xF8, 0x1F, 0x04, 0x20, 0xFE, 0x7F,
        0x04, 0x20, 0xF8, 0x1F, 0x10, 0x08, 0x20, 0x04,
        0x40, 0x02, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00,
    },
    /*.icon_color = */ ui::Color::cyan().v,
    /*.menu_location = */ app_location_t::RX,
    /*.desired_menu_position = */ -1,

    /*.m4_app_tag = portapack::spi_flash::image_tag_none */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
