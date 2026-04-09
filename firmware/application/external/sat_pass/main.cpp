#include "ui.hpp"
#include "ui_sat_pass.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::sat_pass {
void initialize_app(ui::NavigationView& nav) {
    nav.push<SatPassView>();
}
}  // namespace ui::external_app::sat_pass

extern "C" {

__attribute__((section(".external_app.app_sat_pass.application_information"), used)) application_information_t _application_information_sat_pass = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::sat_pass::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,

    /*.app_name = */ "Sat Pass",
    /*.bitmap_data = */ {
        0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0xC0, 0x03,
        0xE0, 0x07, 0xF0, 0x0F, 0xF8, 0x1F, 0xFC, 0x3F,
        0x00, 0x00, 0x18, 0x18, 0x3C, 0x3C, 0x18, 0x18,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    },
    /*.icon_color = */ ui::Color::blue().v,
    /*.menu_location = */ app_location_t::RX,
    /*.desired_menu_position = */ -1,

    /*.m4_app_tag = portapack::spi_flash::image_tag_none */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
