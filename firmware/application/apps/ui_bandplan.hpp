/*
 * US Radio Frequency Band Plan
 * File: firmware/application/apps/ui_bandplan.hpp
 *
 * Maps frequency ranges to human-readable service labels.
 * Used by the AM/FM Scanner app to auto-tag detected signals.
 *
 * Sources: FCC Table of Frequency Allocations, NTIA, FAA AC 90-50D
 *
 * Place this file in: firmware/application/apps/
 */

#ifndef __UI_BANDPLAN_H__
#define __UI_BANDPLAN_H__

#include <cstdint>
#include <string>

namespace ui {
namespace bandplan {

// ─────────────────────────────────────────────────────────
// A single band entry: frequency range + service label
// ─────────────────────────────────────────────────────────
struct BandEntry {
    uint64_t freq_start;   // Hz
    uint64_t freq_end;     // Hz
    const char* label;     // Short label shown on screen
    const char* category;  // Category color hint
};

// ─────────────────────────────────────────────────────────
// US Frequency Band Plan Table
// Ordered from lowest to highest frequency
// ─────────────────────────────────────────────────────────
static const BandEntry BAND_PLAN[] = {

    // ── AM Broadcast ──────────────────────────────────────
    {    530'000,   1'710'000,  "AM RADIO",          "BROADCAST"  },

    // ── HF Shortwave / Ham (partial, below HackRF range) ──
    // HackRF starts at ~1MHz so most HF is unreachable,
    // but included for reference/future hardware.

    // ── VHF Low Band ──────────────────────────────────────
    {  30'000'000,  50'000'000, "VHF LOW / HAM 6M",  "HAM"        },
    {  50'000'000,  54'000'000, "HAM 6M BAND",        "HAM"        },

    // ── FM Broadcast ──────────────────────────────────────
    {  87'500'000, 108'000'000, "FM RADIO",           "BROADCAST"  },

    // ── Aviation VOR/ILS Navigation ───────────────────────
    { 108'000'000, 117'950'000, "AVIATION NAV (VOR)", "AVIATION"   },

    // ── Aviation Voice (ATC) ──────────────────────────────
    { 118'000'000, 121'400'000, "ATC / AIR TRAFFIC",  "AVIATION"   },

    // ── Aviation Emergency ────────────────────────────────
    { 121'500'000, 121'500'000, "!! AVIATION EMERG",  "EMERGENCY"  },

    // ── Aviation Ground / Airport Ops ─────────────────────
    { 121'600'000, 121'925'000, "AIRPORT OPS / ELT",  "AVIATION"   },

    // ── Aviation Instructional ────────────────────────────
    { 121'950'000, 123'675'000, "AVIATION TRAINING",  "AVIATION"   },

    // ── Aviation UNICOM (Private/Uncontrolled) ────────────
    { 122'700'000, 122'975'000, "AVIATION UNICOM",    "AVIATION"   },

    // ── Aviation ATC (continued) ──────────────────────────
    { 123'675'000, 136'975'000, "ATC / AIR TRAFFIC",  "AVIATION"   },

    // ── VHF Land Mobile (Government/Military) ─────────────
    { 137'000'000, 144'000'000, "GOVT / SATELLITE",   "GOVERNMENT" },

    // ── Amateur 2M Ham Band ───────────────────────────────
    { 144'000'000, 148'000'000, "HAM 2M BAND",        "HAM"        },

    // ── VHF Land Mobile (Public Safety) ───────────────────
    { 148'000'000, 150'800'000, "LAND MOBILE / GOVT", "GOVERNMENT" },
    { 150'800'000, 162'000'000, "VHF LAND MOBILE",    "BUSINESS"   },

    // ── NOAA Weather Radio ────────────────────────────────
    { 162'400'000, 162'550'000, "NOAA WEATHER RADIO", "WEATHER"    },

    // ── VHF Land Mobile (continued) ───────────────────────
    { 162'550'000, 173'200'000, "VHF LAND MOBILE",    "BUSINESS"   },

    // ── VHF Public Safety Interop ─────────────────────────
    { 155'000'000, 158'000'000, "POLICE / FIRE VHF",  "EMERGENCY"  },

    // ── Marine VHF ────────────────────────────────────────
    { 156'000'000, 162'025'000, "MARINE VHF",         "MARINE"     },

    // ── Marine Distress / Hailing ─────────────────────────
    { 156'800'000, 156'800'000, "!! MARINE DISTRESS", "EMERGENCY"  },

    // ── Amateur 1.25M / 220 MHz ───────────────────────────
    { 219'000'000, 225'000'000, "HAM 1.25M BAND",     "HAM"        },

    // ── Military UHF Air ──────────────────────────────────
    { 225'000'000, 400'000'000, "MILITARY AIR UHF",   "MILITARY"   },

    // ── Amateur 70cm Band ─────────────────────────────────
    { 420'000'000, 450'000'000, "HAM 70CM BAND",      "HAM"        },

    // ── UHF Land Mobile (Public Safety) ───────────────────
    { 450'000'000, 470'000'000, "UHF PUBLIC SAFETY",  "EMERGENCY"  },

    // ── FRS / GMRS (Walkie Talkies) ───────────────────────
    { 462'000'000, 468'000'000, "FRS / GMRS",         "PERSONAL"   },

    // ── GMRS Emergency ────────────────────────────────────
    { 462'675'000, 462'675'000, "!! GMRS EMERG",      "EMERGENCY"  },

    // ── UHF Business / Land Mobile ────────────────────────
    { 470'000'000, 512'000'000, "UHF LAND MOBILE",    "BUSINESS"   },

    // ── UHF TV (Broadcast) ────────────────────────────────
    { 512'000'000, 698'000'000, "UHF TV BROADCAST",   "BROADCAST"  },

    // ── Cellular / LTE ────────────────────────────────────
    { 698'000'000, 960'000'000, "CELLULAR / LTE",     "CELLULAR"   },

    // ── GPS ───────────────────────────────────────────────
    {1'575'420'000,1'575'420'000,"GPS L1",            "NAVIGATION" },
    {1'227'600'000,1'227'600'000,"GPS L2",            "NAVIGATION" },

    // ── ISM / WiFi 2.4GHz ────────────────────────────────
    {2'400'000'000,2'500'000'000,"WiFi / ISM 2.4GHz", "DATA"       },

    // ── WiFi 5GHz ─────────────────────────────────────────
    {5'150'000'000,5'850'000'000,"WiFi 5GHz",         "DATA"       },
};

static const int BAND_PLAN_COUNT = sizeof(BAND_PLAN) / sizeof(BAND_PLAN[0]);

// ─────────────────────────────────────────────────────────
// Look up a frequency in the band plan.
// Returns the label string, or "UNKNOWN" if not found.
// ─────────────────────────────────────────────────────────
inline std::string get_band_label(uint64_t freq_hz) {
    for (int i = 0; i < BAND_PLAN_COUNT; i++) {
        const auto& b = BAND_PLAN[i];
        // For point frequencies (start == end), check within 25kHz
        if (b.freq_start == b.freq_end) {
            if (freq_hz >= b.freq_start - 25'000 &&
                freq_hz <= b.freq_end + 25'000) {
                return std::string(b.label);
            }
        } else {
            if (freq_hz >= b.freq_start && freq_hz <= b.freq_end) {
                return std::string(b.label);
            }
        }
    }
    return "UNKNOWN";
}

// ─────────────────────────────────────────────────────────
// Get category for color coding
// Returns: "EMERGENCY", "AVIATION", "BROADCAST",
//          "HAM", "WEATHER", "MARINE", "MILITARY",
//          "BUSINESS", "GOVERNMENT", "PERSONAL",
//          "CELLULAR", "NAVIGATION", "DATA", "UNKNOWN"
// ─────────────────────────────────────────────────────────
inline std::string get_band_category(uint64_t freq_hz) {
    for (int i = 0; i < BAND_PLAN_COUNT; i++) {
        const auto& b = BAND_PLAN[i];
        if (b.freq_start == b.freq_end) {
            if (freq_hz >= b.freq_start - 25'000 &&
                freq_hz <= b.freq_end + 25'000) {
                return std::string(b.category);
            }
        } else {
            if (freq_hz >= b.freq_start && freq_hz <= b.freq_end) {
                return std::string(b.category);
            }
        }
    }
    return "UNKNOWN";
}

// ─────────────────────────────────────────────────────────
// Check if a frequency is an emergency channel
// ─────────────────────────────────────────────────────────
inline bool is_emergency(uint64_t freq_hz) {
    return get_band_category(freq_hz) == "EMERGENCY";
}

}  // namespace bandplan
}  // namespace ui

#endif /*__UI_BANDPLAN_H__*/
