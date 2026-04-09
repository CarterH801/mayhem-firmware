#include "ui.hpp"
#include "ui_signal_map.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::signal_map {
void initialize_app(ui::NavigationView& nav) {
    nav.push<SignalMapView>();
}
}  // namespace ui::external_app::signal_map

extern "C" {

__attribute__((section(".external_app.app_signal_map.application_information"), used)) application_information_t _application_information_signal_map = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::signal_map::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,

    /*.app_name = */ "Signal Map",
    /*.bitmap_data = */ {
        0x00, 0x00, 0x00, 0x00, 0xC0, 0x03, 0x20, 0x04,
        0x10, 0x08, 0x08, 0x10, 0x04, 0x20, 0x84, 0x21,
        0x84, 0x21, 0x04, 0x20, 0x08, 0x10, 0x10, 0x08,
        0x20, 0x04, 0xC0, 0x03, 0x00, 0x00, 0x00, 0x00,
    },
    /*.icon_color = */ ui::Color::blue().v,
    /*.menu_location = */ app_location_t::UTILITIES,
    /*.desired_menu_position = */ -1,

    /*.m4_app_tag = portapack::spi_flash::image_tag_none */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
