#include "ui.hpp"
#include "ui_rf_assistant.hpp"
#include "ui_navigation.hpp"
#include "external_app.hpp"

namespace ui::external_app::rf_assistant {
void initialize_app(ui::NavigationView& nav) {
    nav.push<RFAssistantView>();
}
}

extern "C" {
__attribute__((section(".external_app.app_rf_assistant.application_information"), used)) application_information_t _application_information_rf_assistant = {
    /*.memory_location = */ (uint8_t*)0x00000000,
    /*.externalAppEntry = */ ui::external_app::rf_assistant::initialize_app,
    /*.header_version = */ CURRENT_HEADER_VERSION,
    /*.app_version = */ VERSION_MD5,
    /*.app_name = */ "RF Assist",
    /*.bitmap_data = */ {0x00,0x00,0xFC,0x3F,0x04,0x20,0xE4,0x27,0x24,0x24,0xE4,0x27,0x04,0x20,0x04,0x20,0xE4,0x27,0x24,0x24,0xE4,0x27,0x04,0x20,0x04,0x20,0xFC,0x3F,0x00,0x00,0x00,0x00},
    /*.icon_color = */ ui::Color::white().v,
    /*.menu_location = */ app_location_t::UTILITIES,
    /*.desired_menu_position = */ -1,
    /*.m4_app_tag = */ {0, 0, 0, 0},
    /*.m4_app_offset = */ 0x00000000,
};
}
