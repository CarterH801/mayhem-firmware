/*
 * US Radio Frequency Band Plan — Type definition
 *
 * This header exposes only the BandEntry struct. The actual band plan
 * data and lookup functions live in ui_bandplan_data.inc, which must be
 * included INSIDE the using app's namespace so the data ends up in the
 * correct external app memory section.
 *
 * Usage pattern inside an external app's .cpp file:
 *
 *   #include "ui_bandplan.hpp"
 *   ...
 *   namespace ui::external_app::<appname> {
 *   namespace bandplan {
 *       using ::ui::bandplan::BandEntry;
 *       #include "ui_bandplan_data.inc"
 *   }
 *   // now use bandplan::get_band_label(freq) from within the app
 *   }
 */

#ifndef __UI_BANDPLAN_H__
#define __UI_BANDPLAN_H__

#include <cstdint>
#include <string>

namespace ui {
namespace bandplan {

// A single band entry: frequency range + service label
struct BandEntry {
    uint64_t freq_start;   // Hz
    uint64_t freq_end;     // Hz
    const char* label;     // Short label shown on screen
    const char* category;  // Category color hint
};

}  // namespace bandplan
}  // namespace ui

#endif /*__UI_BANDPLAN_H__*/
