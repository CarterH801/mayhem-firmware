#include "ui.hpp"
#include "ui_hw_test.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::hw_test {
void initialize_app(ui::NavigationView& nav) {
    nav.push<HWTestView>();
}
}  // namespace ui::external_app::hw_test

extern "C" {

__attribute__((section(".external_app.app_hw_test.application_information"), used)) application_information_t _application_information_hw_test = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::hw_test::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,

    /*.app_name = */ "HW Test",
    /*.bitmap_data = */ {
        0x00, 0x00, 0xFE, 0x7F, 0x02, 0x40, 0xFA, 0x5F,
        0x0A, 0x50, 0xEA, 0x57, 0x2A, 0x54, 0xAA, 0x55,
        0x2A, 0x54, 0xEA, 0x57, 0x0A, 0x50, 0xFA, 0x5F,
        0x02, 0x40, 0xFE, 0x7F, 0x00, 0x00, 0x00, 0x00,
    },
    /*.icon_color = */ ui::Color::red().v,
    /*.menu_location = */ app_location_t::DEBUG,
    /*.desired_menu_position = */ -1,

    /*.m4_app_tag = portapack::spi_flash::image_tag_none */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
