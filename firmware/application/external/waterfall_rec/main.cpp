#include "ui.hpp"
#include "ui_waterfall_rec.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::waterfall_rec {
void initialize_app(ui::NavigationView& nav) {
    nav.push<WaterfallRecView>();
}
}  // namespace ui::external_app::waterfall_rec

extern "C" {

__attribute__((section(".external_app.app_waterfall_rec.application_information"), used)) application_information_t _application_information_waterfall_rec = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::waterfall_rec::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,

    /*.app_name = */ "Waterfall",
    /*.bitmap_data = */ {
        0x00, 0x00, 0xFF, 0xFF, 0x01, 0x80, 0xFD, 0xBF,
        0x05, 0xA0, 0xF5, 0xAF, 0x15, 0xA8, 0xD5, 0xAB,
        0x55, 0xAA, 0xD5, 0xAB, 0x15, 0xA8, 0xF5, 0xAF,
        0x05, 0xA0, 0xFD, 0xBF, 0x01, 0x80, 0xFF, 0xFF,
    },
    /*.icon_color = */ ui::Color::green().v,
    /*.menu_location = */ app_location_t::RX,
    /*.desired_menu_position = */ -1,

    /*.m4_app_tag = portapack::spi_flash::image_tag_none */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
