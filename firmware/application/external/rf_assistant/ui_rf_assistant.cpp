/*
 * RF Assistant — Offline Reference System
 * File: firmware/application/apps/ui_rf_assistant.cpp
 *
 * Contains all 5 reference databases compiled in.
 */

#include "ui_rf_assistant.hpp"
#include "portapack.hpp"
#include "string_format.hpp"
#include "ui_bandplan.hpp"
#include <cstring>

namespace ui {

// ═════════════════════════════════════════════════════════
//  DATABASE 1 — MAYHEM ERROR CODES
//  Common errors, Guru Meditation codes, and fixes
// ═════════════════════════════════════════════════════════
const RefEntry RFAssistantView::ERROR_DB[] = {

    {"GURU", "Guru Meditation Error",
     "WHAT IT IS: A hard fault crash in the firmware.\n\n"
     "COMMON CAUSES:\n"
     "- Null pointer dereference in an app\n"
     "- Stack overflow (too many nested calls)\n"
     "- Memory corruption\n"
     "- Bad SD card file being read\n\n"
     "FIXES TO TRY:\n"
     "1. Press the reset button on the back\n"
     "2. Remove SD card and boot — if it boots OK,\n"
     "   your SD card has a corrupt file\n"
     "3. Settings > Factory Defaults to reset all\n"
     "4. Re-flash firmware from scratch\n"
     "5. If in a custom app: check for null pointers\n"
     "   and array bounds in your code"},

    {"NOBOOT", "Won't Boot / Black Screen",
     "WHAT IT IS: PortaPack fails to start.\n\n"
     "FIXES IN ORDER:\n"
     "1. Check battery — plug into USB power\n"
     "2. Remove SD card — boot without it\n"
     "3. Hold down the encoder while powering on\n"
     "   to enter DFU recovery mode\n"
     "4. In DFU mode, re-flash firmware via\n"
     "   hackrf.app in Chrome browser\n"
     "5. Dead coin cell battery on the HackRF board\n"
     "   — replace the CR1220 battery inside\n"
     "6. Check the USB cable — try a different one\n"
     "   (many USB cables are charge-only, not data)"},

    {"SDCARD", "SD Card Not Recognized",
     "WHAT IT IS: PortaPack can't read the SD card.\n\n"
     "FIXES:\n"
     "1. Format SD card as FAT32\n"
     "   (NOT exFAT, NOT NTFS — must be FAT32)\n"
     "2. Use a card 32GB or smaller\n"
     "3. Try a different SD card brand\n"
     "   (SanDisk Ultra works best)\n"
     "4. Copy SD files fresh from the Mayhem\n"
     "   release zip: mayhem_vX.X_COPY_TO_SDCARD.zip\n"
     "5. If using SD over USB: disconnect USB first\n"
     "   then reinsert SD card normally\n"
     "6. Check for bent/dirty contacts on SD slot"},

    {"NORX", "No Signal Being Received",
     "WHAT IT IS: HackRF tunes but gets no signal.\n\n"
     "FIXES IN ORDER:\n"
     "1. Check antenna is firmly connected to SMA port\n"
     "2. Make sure you are in the right frequency range\n"
     "   for the signal you are looking for\n"
     "3. Lower the squelch (SQ) value\n"
     "   Try SQ:-90 or lower\n"
     "4. Check gain settings in the title bar\n"
     "   Try LNA=32, VGA=40, AMP=on\n"
     "5. Try the Level app to see raw RSSI\n"
     "6. Some HackRF clones have poor sensitivity\n"
     "   below 300 MHz — try a known strong signal\n"
     "   like an FM radio station first (88-108 MHz)"},

    {"NOTX", "TX Not Working / Carrier Only",
     "WHAT IT IS: HackRF transmits but no modulation.\n\n"
     "CAUSES:\n"
     "- Baseband processor not running correctly\n"
     "- Wrong sample rate for the app\n"
     "- Audio input not connected/configured\n\n"
     "FIXES:\n"
     "1. Restart the app from the menu\n"
     "2. Check your TX power setting is not zero\n"
     "3. Re-flash firmware — corrupted baseband image\n"
     "4. Make sure you have a ham license for the\n"
     "   frequency you are trying to transmit on"},

    {"FLASH", "Firmware Update Failed",
     "WHAT IT IS: Flashing new firmware failed or\n"
     "device got stuck during update.\n\n"
     "RECOVERY:\n"
     "1. Do NOT panic — the device is not bricked\n"
     "2. Hold encoder button while plugging in USB\n"
     "   to force DFU bootloader mode\n"
     "3. Open Chrome and go to hackrf.app\n"
     "4. Use the DFU unbrick option\n"
     "5. Flash the firmware fresh\n"
     "6. If hackrf.app does not work on your OS:\n"
     "   Use the mayhem_flasher.bat script from the\n"
     "   Mayhem GitHub releases page"},

    {"FREEZE", "App Freezes or Hangs",
     "WHAT IT IS: An app stops responding.\n\n"
     "CAUSES:\n"
     "- Infinite loop in the app code\n"
     "- Waiting for a message that never arrives\n"
     "- SD card write blocking the main thread\n\n"
     "FIXES:\n"
     "1. Press the reset button (back of device)\n"
     "2. If it happens in a custom app: check for\n"
     "   blocking loops in message handlers\n"
     "3. If logging to SD: use async writes\n"
     "4. Try with SD card removed — if stable,\n"
     "   your SD card is slow (causes write blocks)"},

    {"APPICON", "App Icon Wrong Color / Missing",
     "WHAT IT IS: App shows wrong color in menu\n"
     "or icon does not display.\n\n"
     "FIX:\n"
     "1. Make sure you set the app icon correctly\n"
     "   in ui_navigation.cpp\n"
     "2. Each app needs an icon from the\n"
     "   existing Mayhem icon set\n"
     "3. Use an existing icon like:\n"
     "   &bitmap_icon_scanner\n"
     "   &bitmap_icon_receiver\n"
     "   &bitmap_icon_tools\n"
     "4. Check the icon colors match what Mayhem\n"
     "   expects for your app category"},

    {"MEMFULL", "Out of Memory / Memory Error",
     "WHAT IT IS: The app uses too much RAM.\n\n"
     "THE LIMITS:\n"
     "- PortaPack has ~200KB RAM total\n"
     "- Firmware uses ~100KB\n"
     "- Each app gets ~50-80KB to work with\n\n"
     "FIXES:\n"
     "1. Reduce buffer sizes in your app\n"
     "2. Use static arrays instead of vectors\n"
     "   where possible\n"
     "3. Free resources in the destructor\n"
     "4. Check the Debug > Memory Usage app\n"
     "   to see how much you are using\n"
     "5. Consider moving large data to SD card"},

    {"BUILDCMAKE", "CMake Build Error",
     "WHAT IT IS: The firmware fails to configure\n"
     "during the cmake step.\n\n"
     "COMMON MESSAGES AND FIXES:\n\n"
     "'file not found' or 'No such file':\n"
     "- Check your .cpp file is in the right folder\n"
     "- Check the filename matches EXACTLY\n"
     "  (case sensitive on Linux/Docker)\n\n"
     "'undefined reference to':\n"
     "- Your .cpp file was not added to CMakeLists.txt\n"
     "- Add it to the apps source list\n\n"
     "'implicit declaration of function':\n"
     "- Missing #include in your .hpp file\n"
     "- Check all includes are present"},

    {"BUILDLINK", "Linker Error During Build",
     "WHAT IT IS: The build compiles but fails\n"
     "when linking everything together.\n\n"
     "COMMON MESSAGES:\n\n"
     "'multiple definition of':\n"
     "- You defined a function in a .hpp file\n"
     "  instead of a .cpp file\n"
     "- Or included a .cpp file instead of .hpp\n\n"
     "'undefined reference to vtable':\n"
     "- A virtual function is declared but not\n"
     "  implemented in the .cpp file\n"
     "- Check all virtual functions have bodies\n\n"
     "'region is full' or 'overflow':\n"
     "- The firmware is too large for flash memory\n"
     "- Remove some apps or reduce data sizes"},

    {"GPS", "GPS Module Not Working",
     "WHAT IT IS: External GPS module attached\n"
     "but not getting a fix.\n\n"
     "SETUP:\n"
     "- Connect GPS to the PortaPack's expansion port\n"
     "- GPS needs a clear view of the sky\n"
     "- First fix can take 5-15 minutes (cold start)\n\n"
     "FIXES:\n"
     "1. Go outside with clear sky view\n"
     "2. Wait up to 15 min for cold start fix\n"
     "3. Check GPS module baud rate matches\n"
     "   PortaPack setting (usually 9600)\n"
     "4. Check wiring on the expansion connector\n"
     "5. Try the GPS Sim app to verify the\n"
     "   GPS receiver itself is working"},

    {"AUDIO", "No Audio / Audio Distorted",
     "WHAT IT IS: Can not hear received signals.\n\n"
     "FIXES:\n"
     "1. Check speaker/headphone is connected\n"
     "2. Settings > Audio > Volume — increase it\n"
     "3. In the receiver app, check the audio\n"
     "   mode is correct (WFM for FM radio)\n"
     "4. Check squelch is not too high (blocking audio)\n"
     "5. Try the Audio Test in Debug menu\n"
     "6. For distortion: lower the VGA gain setting\n"
     "   in the title bar (try VGA=20 first)"},
};
const int RFAssistantView::ERROR_DB_COUNT = 13;


// ═════════════════════════════════════════════════════════
//  DATABASE 2 — APP HELP REFERENCE
//  Quick guide for every app in our suite
// ═════════════════════════════════════════════════════════
const RefEntry RFAssistantView::HELP_DB[] = {

    {"scanner", "Band Scanner",
     "WHAT IT DOES:\n"
     "Sweeps a frequency band and shows you only\n"
     "the active signals — filters out static.\n\n"
     "MODES:\n"
     "FM   — Scans 87.5-108 MHz FM broadcast band\n"
     "AM   — Scans 1.0-1.71 MHz AM broadcast band\n"
     "BOTH — FM first then AM\n"
     "KFOB — Key fob bands: 315/433/868 MHz\n"
     "DRON — Drone bands: 433/868/915/2.4G/5.8G\n"
     "BT   — Bluetooth bands: 2402/2426/2480 MHz\n\n"
     "HOW IT WORKS:\n"
     "Phase 1: Measures noise floor across the band\n"
     "Phase 2: Flags signals above that baseline\n\n"
     "SQUELCH (SQ):\n"
     "Lower = more sensitive. Raise if too many\n"
     "false hits. Default -75 works for most cases.\n\n"
     "BUTTONS:\n"
     "TUNE  — Listen to selected station\n"
     "FIND  — Launch Signal Finder on this signal\n"
     "CLEAR — Wipe list and rescan"},

    {"finder", "Signal Finder",
     "WHAT IT DOES:\n"
     "Hot/cold proximity meter. Walk toward a signal\n"
     "source and the display heats up like a metal\n"
     "detector as you get closer.\n\n"
     "THE DISPLAY:\n"
     "Big % number — signal strength 0-100%\n"
     "HOT/WARM/COLD label — relative to baseline\n"
     "^ arrow — getting stronger (walk this way)\n"
     "v arrow — getting weaker (wrong direction)\n"
     "- arrow — holding steady\n"
     "Graph — last 30 readings as rolling history\n\n"
     "CAPTURE BUTTON:\n"
     "Records raw IQ data to SD card.\n"
     "Saves as /CAPTURES/FREQ_LABEL.C16\n"
     "Replayable in Mayhem's Replay app.\n\n"
     "BEEP BUTTON:\n"
     "Toggles metal detector audio.\n"
     "Pitch rises as signal gets stronger.\n"
     "Rate speeds up too — like a real detector."},

    {"range", "Range Estimator",
     "WHAT IT DOES:\n"
     "Estimates how far away a transmitter is\n"
     "using the Free Space Path Loss formula.\n\n"
     "DISTANCE SHOWN IN: Miles and Meters\n\n"
     "HOW TO USE:\n"
     "1. Select the device type from the list\n"
     "   (TX power auto-fills for known devices)\n"
     "2. Watch the distance update in real time\n"
     "3. Move around — distance changes as RSSI does\n\n"
     "CONFIDENCE BAR:\n"
     "Green  = strong signal, reliable estimate\n"
     "Yellow = moderate, rough estimate\n"
     "Red    = weak signal, low accuracy\n\n"
     "SMOOTH BUTTON:\n"
     "Averages last 5 readings for stable numbers.\n\n"
     "IMPORTANT: Only accurate in open air with\n"
     "line of sight. Buildings cause big errors.\n\n"
     "DEVICE PRESETS INCLUDE:\n"
     "DJI drones, FPV VTX (all power levels),\n"
     "Key fobs, WiFi routers, FM towers,\n"
     "Ham radios, Walkie talkies, Cell towers"},

    {"map", "Signal Map",
     "WHAT IT DOES:\n"
     "Plots your distance measurements on the world\n"
     "map as circles. Where they intersect = the\n"
     "transmitter location.\n\n"
     "THIS TECHNIQUE IS CALLED: Trilateration\n\n"
     "HOW TO USE:\n"
     "1. Set your lat/lon in the fields below the map\n"
     "   (or use external GPS for automatic position)\n"
     "2. Stand at position A, press MARK\n"
     "   Yellow circle appears on map\n"
     "3. Walk 50-100m to position B, press MARK\n"
     "   Cyan circle appears\n"
     "4. Walk to position C, press MARK\n"
     "   Green circle appears\n"
     "5. Press FIND\n"
     "   Red X appears where circles intersect\n\n"
     "REQUIRES:\n"
     "- world_map.bin on SD card (from Mayhem zip)\n"
     "- At least 3 measurements from different spots\n"
     "- Better accuracy = more spread out positions"},

    {"watch", "Frequency Watch",
     "WHAT IT DOES:\n"
     "Silent background RF tripwire. Monitors up to\n"
     "8 frequencies. Beeps and flashes red when any\n"
     "of them go active above your threshold.\n\n"
     "PRE-LOADED SLOTS:\n"
     "#1 — Key fob 315 MHz (US cars)\n"
     "#2 — Key fob 433 MHz (EU cars)\n"
     "#3 — NOAA Weather Radio\n"
     "#4 — Aviation Emergency 121.5 MHz\n"
     "#5 — Drone control 915 MHz\n"
     "#6 — BLE advertising channel 37\n"
     "#7 — Empty (set your own)\n"
     "#8 — Empty (set your own)\n\n"
     "HOW TO SET A SLOT:\n"
     "1. Use </> buttons to select slot number\n"
     "2. Enter frequency in MHz.kHz fields\n"
     "3. Set squelch threshold (alert above this)\n"
     "4. Press SET\n"
     "5. Press WATCH to start monitoring\n\n"
     "TRIGGER COUNT:\n"
     "Shows how many times each slot fired.\n"
     "Currently triggering slots show in RED."},

    {"modid", "Modulation Identifier",
     "WHAT IT DOES:\n"
     "Analyzes a received signal and tells you what\n"
     "modulation type it uses.\n\n"
     "DETECTS:\n"
     "WFM  — Wideband FM (broadcast radio)\n"
     "NFM  — Narrow FM (police, business, ham)\n"
     "AM   — Amplitude Mod (aircraft, AM radio)\n"
     "OOK  — On-Off Keying (garage doors, sensors)\n"
     "FSK  — Frequency Shift Keying (key fobs, IoT)\n"
     "CW   — Continuous Wave (beacons, test tones)\n\n"
     "HOW TO USE:\n"
     "1. Tune to the signal you want to identify\n"
     "2. Make sure the signal is active\n"
     "3. Press ANALYZE\n"
     "4. Wait ~10 seconds for sample collection\n"
     "5. Results show modulation type, confidence\n"
     "   percentage, description, and likely device\n\n"
     "CONFIDENCE:\n"
     "Green  = HIGH (75%+) — reliable ID\n"
     "Yellow = MEDIUM (50-74%) — probable\n"
     "Red    = LOW (<50%) — uncertain"},

    {"tpms", "TPMS Vehicle Counter",
     "WHAT IT DOES:\n"
     "Every car made after 2008 broadcasts tire\n"
     "pressure data on 315 MHz (US) or 433 MHz (EU).\n"
     "Each sensor has a unique ID so we can count\n"
     "and identify individual vehicles.\n\n"
     "DISPLAY COLUMNS:\n"
     "ID       — Unique 8-digit sensor ID\n"
     "TIRE     — FL/FR/RL/RR (which tire)\n"
     "PSI      — Tire pressure in PSI\n"
     "TEMP     — Temperature in Fahrenheit\n"
     "SIG      — Signal strength in dBm\n"
     "HITS     — How many times detected\n\n"
     "COLORS:\n"
     "White  — Normal pressure (32+ PSI)\n"
     "Yellow — Slightly low (28-31 PSI)\n"
     "Red    — Low pressure (<28 PSI)\n\n"
     "BAND SELECTOR:\n"
     "315MHz for US market vehicles\n"
     "433MHz for European market vehicles\n\n"
     "NOTE: TPMS only transmits when the car is\n"
     "moving or shortly after parking."},

    {"noaa", "NOAA Weather Satellite",
     "WHAT IT DOES:\n"
     "Receives actual weather satellite images from\n"
     "NOAA-15, NOAA-18, NOAA-19 orbiting at 850km.\n"
     "You see real cloud cover photos of your region.\n\n"
     "FREQUENCIES:\n"
     "NOAA-15 : 137.620 MHz\n"
     "NOAA-18 : 137.912 MHz\n"
     "NOAA-19 : 137.100 MHz\n\n"
     "WHAT YOU NEED:\n"
     "- A V-dipole or turnstile antenna\n"
     "  (NOT a simple whip — must be wideband)\n"
     "- Clear view of the sky\n"
     "- 12-minute window when sat is overhead\n\n"
     "HOW TO FIND PASS TIMES:\n"
     "Use the Satellite Pass Detector app\n"
     "Or check: heavens-above.com\n"
     "Or app: 'Look4Sat' on Android\n\n"
     "SAVED TO: /NOAA/noaa_YYYYMMDD_HH.pgm\n"
     "Open .pgm files in any image viewer\n\n"
     "TIP: Best results 10-30 degrees above horizon"},

    {"satpass", "Satellite Pass Detector",
     "WHAT IT DOES:\n"
     "Continuously scans 15 known satellite frequencies\n"
     "and alerts when any of them go active —\n"
     "indicating a satellite is passing overhead.\n\n"
     "SATELLITES MONITORED:\n"
     "ISS      — 4 frequencies (APRS + voice)\n"
     "AO-91    — FM amateur satellite\n"
     "AO-92    — FM amateur satellite\n"
     "SO-50    — FM amateur satellite\n"
     "LilacSat2— FM amateur satellite\n"
     "TEVEL    — Cluster of 6 sats\n"
     "CAS-4A   — Chinese amateur sat\n"
     "XW-2A    — Chinese amateur sat\n"
     "NOAA-15  — Weather satellite\n"
     "NOAA-18  — Weather satellite\n"
     "NOAA-19  — Weather satellite\n"
     "METEOR M2— Russian weather sat\n\n"
     "WHEN TRIGGERED:\n"
     "- Beep alert (double tone)\n"
     "- Green banner with sat name + frequency\n"
     "- Event logged with timestamp\n\n"
     "TIP: Point antenna toward the sky.\n"
     "Satellites are weak — use a quality antenna."},

    {"waterfall", "Waterfall Recorder",
     "WHAT IT DOES:\n"
     "Sweeps a frequency band repeatedly and records\n"
     "every sweep to CSV on SD card, creating a\n"
     "time-lapse of all RF activity over hours.\n\n"
     "LIVE DISPLAY:\n"
     "Heatmap shows signal strength by color:\n"
     "Black   = No signal\n"
     "Blue    = Very weak\n"
     "Cyan    = Weak\n"
     "Green   = Moderate\n"
     "Yellow  = Strong\n"
     "Orange  = Very strong\n"
     "Red     = Extremely strong\n\n"
     "PRESET BANDS:\n"
     "FM 87-108 MHz — Find broadcast stations\n"
     "VHF 136-175   — Aviation + public safety\n"
     "KFOB 313-317  — Key fob activity\n"
     "UHF 430-440   — Ham + key fob + drone\n"
     "900-928 MHz   — Drone control band\n\n"
     "SAVED TO: /WATERFALL/wfall_YYYYMMDD_HH.csv\n"
     "Open in Excel/Google Sheets to analyze\n"
     "Each row = one sweep with timestamp"},
};
const int RFAssistantView::HELP_DB_COUNT = 10;


// ═════════════════════════════════════════════════════════
//  DATABASE 3 — TROUBLESHOOTING GUIDE
//  Top problems and their solutions
// ═════════════════════════════════════════════════════════
const RefEntry RFAssistantView::TROUBLE_DB[] = {

    {"1", "HackRF Not Detected by PC",
     "SYMPTOMS: PC doesn't see the device,\n"
     "hackrf.app shows 'no device found'\n\n"
     "STEP 1: Try a different USB cable\n"
     "  Most USB cables are charge-only — not data.\n"
     "  Use the cable that came with the device.\n\n"
     "STEP 2: Try a USB 2.0 port (not USB 3.0)\n"
     "  Blue ports can sometimes cause issues.\n\n"
     "STEP 3: Check device manager (Windows)\n"
     "  Look for 'HackRF One' or 'Unknown Device'\n"
     "  If Unknown: install Zadig drivers\n"
     "  zadig.akeo.ie → install WinUSB driver\n\n"
     "STEP 4: Make sure PortaPack is in Normal Mode\n"
     "  NOT in HackRF Mode (Settings > Config)"},

    {"2", "Terrible Signal Reception",
     "SYMPTOMS: Everything looks noisy, weak signals,\n"
     "can't receive things nearby.\n\n"
     "STEP 1: Check your antenna\n"
     "  The stock stubby antenna is terrible.\n"
     "  Get a telescoping whip antenna — huge difference.\n\n"
     "STEP 2: Set gain properly\n"
     "  In the title bar: LNA=32, VGA=40, AMP=ON\n"
     "  AMP adds 14dB — toggle it on/off\n\n"
     "STEP 3: Check frequency\n"
     "  HackRF is weakest below 100 MHz\n"
     "  Best performance: 300 MHz - 3 GHz\n\n"
     "STEP 4: Reduce interference\n"
     "  Keep away from PC, USB cables, monitors\n"
     "  Use a USB extension cable to distance the\n"
     "  HackRF from your laptop"},

    {"3", "Battery Drains Very Fast",
     "SYMPTOMS: PortaPack H2 battery dies quickly\n\n"
     "NORMAL BATTERY LIFE: 3-5 hours with display on\n\n"
     "FIXES:\n"
     "1. Reduce display brightness\n"
     "   Settings > Brightness\n\n"
     "2. Enable sleep mode when idle\n"
     "   Settings > Sleep Mode\n\n"
     "3. TX operations drain battery fast\n"
     "   Only transmit when needed\n\n"
     "4. Disable amp when not needed\n"
     "   Amp adds ~200mA draw\n\n"
     "5. If battery is old it may need replacement\n"
     "   H2 uses a standard 3.7V LiPo"},

    {"4", "Apps Crashing on Open",
     "SYMPTOMS: App crashes immediately when opened,\n"
     "or after a few seconds.\n\n"
     "MOST COMMON CAUSE: Missing SD card file\n"
     "Some apps require files on the SD card.\n\n"
     "FIX:\n"
     "1. Make sure SD card is inserted\n"
     "2. Copy fresh SD card files from Mayhem release:\n"
     "   mayhem_vX.X_COPY_TO_SDCARD.zip\n"
     "3. Check /ADSB/world_map.bin exists for map apps\n\n"
     "FOR CUSTOM APPS:\n"
     "- Check add_children() lists all widgets\n"
     "- Check no null pointers in constructor\n"
     "- Check array bounds\n"
     "- Use Debug > Memory Usage to check RAM"},

    {"5", "Can't Hear Audio",
     "SYMPTOMS: Receiving signal but no audio output\n\n"
     "CHECK THESE IN ORDER:\n"
     "1. Volume: Settings > Audio > Volume > turn up\n\n"
     "2. Squelch too high: lower the SQ value\n"
     "   In the app, set SQ to -90 or lower\n\n"
     "3. Wrong modulation mode:\n"
     "   FM stations need WFM (Wideband FM)\n"
     "   Police/business need NFM (Narrow FM)\n"
     "   Aircraft need AM\n\n"
     "4. Bandwidth too narrow:\n"
     "   Increase BW setting in the app\n\n"
     "5. Check headphone jack or speaker connection\n"
     "   Test with Debug > Audio Test"},

    {"6", "Signal Map No World Map Shown",
     "SYMPTOMS: Signal Map opens but shows a black\n"
     "or blank map area instead of the world map.\n\n"
     "FIX:\n"
     "The world_map.bin file is missing from SD card.\n\n"
     "1. Download latest Mayhem release from GitHub\n"
     "2. Find: mayhem_vX.X_COPY_TO_SDCARD.zip\n"
     "3. Extract with 7-Zip\n"
     "4. Copy all files to SD card root\n"
     "5. The map file will be at: /ADSB/world_map.bin\n\n"
     "NOTE: The math and circles still work even\n"
     "without the map. The coordinate output at the\n"
     "bottom still shows estimated location."},

    {"7", "TPMS Counter Shows Nothing",
     "SYMPTOMS: TPMS Counter running but no vehicles\n"
     "appearing in the list.\n\n"
     "REASONS:\n"
     "1. No cars nearby\n"
     "   TPMS sensors only transmit when moving\n"
     "   or shortly after parking. Park near a\n"
     "   busy road for best results.\n\n"
     "2. Wrong band selected\n"
     "   US vehicles: 315 MHz\n"
     "   European vehicles: 433 MHz\n\n"
     "3. Antenna not suited for 315/433 MHz\n"
     "   Use a 1/4 wave whip (~23cm for 315MHz)\n\n"
     "4. Squelch too high — lower it to -90"},

    {"8", "NOAA Satellite Just Static",
     "SYMPTOMS: NOAA app receiving but image is\n"
     "just static, no cloud picture.\n\n"
     "MOST LIKELY: Wrong antenna\n"
     "You MUST use a wideband antenna:\n"
     "  V-dipole (2 elements at 120 degrees)\n"
     "  Turnstile (circular polarization — best)\n"
     "  QFH (quadrifilar helix — very good)\n"
     "A simple whip WILL NOT WORK for satellites.\n\n"
     "ALSO CHECK:\n"
     "1. Satellite must be above ~10 degrees elevation\n"
     "   Use Look4Sat app to find pass times\n\n"
     "2. Must be outdoors with clear sky view\n\n"
     "3. Check frequency — NOAA-19: 137.100 MHz\n"
     "   NOAA-18: 137.912 MHz\n"
     "   NOAA-15: 137.620 MHz"},

    {"9", "Waterfall File Not Saving",
     "SYMPTOMS: Waterfall recorder running but\n"
     "no CSV file appearing on SD card.\n\n"
     "FIXES:\n"
     "1. Create the /WATERFALL folder manually\n"
     "   on the SD card (the app should auto-create\n"
     "   it but sometimes fails)\n\n"
     "2. Make sure SD card is not full\n\n"
     "3. Check SD card is FAT32 formatted\n\n"
     "4. Try a faster SD card\n"
     "   Slow cards block on writes and cause issues\n"
     "   Use a Class 10 / UHS-1 rated card\n\n"
     "5. The /LOGS and /WATERFALL folders need to\n"
     "   exist before the apps will write to them"},

    {"10", "Build Fails: undefined reference",
     "SYMPTOMS: During make, error says:\n"
     "'undefined reference to SomeFunction::method'\n\n"
     "CAUSE: A .cpp file was not added to build\n\n"
     "FIX:\n"
     "1. Open: firmware/application/CMakeLists.txt\n"
     "2. Find the list of apps/*.cpp files\n"
     "3. Add the missing file to that list\n"
     "   Example: apps/ui_my_new_app.cpp\n"
     "4. Save the file\n"
     "5. Re-run: make -j2 firmware\n\n"
     "NOTE: Header-only files (.hpp only) do NOT\n"
     "need to be in CMakeLists.txt — only .cpp files"},
};
const int RFAssistantView::TROUBLE_DB_COUNT = 10;


// ═════════════════════════════════════════════════════════
//  DATABASE 4 — SIGNAL ID GUIDE TREE
//  Decision tree to identify unknown signals
// ═════════════════════════════════════════════════════════
const GuideNode RFAssistantView::GUIDE_NODES[] = {
    // Node 0: Start
    {"Is the signal continuous (always on)\n"
     "or pulsed/bursty (comes and goes)?",
     "1",   // yes = continuous → node 1
     "2",   // no  = pulsed → node 2
     nullptr},

    // Node 1: Continuous signal
    {"Does the signal have audio content?\n"
     "(tune to it — can you hear voice or music?)",
     "3",   // yes = has audio
     "4",   // no  = no audio
     nullptr},

    // Node 2: Pulsed/bursty signal
    {"Are the bursts very short — less than\n"
     "1 second each?",
     "5",   // yes = very short bursts
     "6",   // no  = longer bursts
     nullptr},

    // Node 3: Continuous with audio
    {"Is it music or broadcast quality audio?\n"
     "(wide, full-sounding audio)",
     "RESULT:0", nullptr, nullptr},

    // Node 4: Continuous no audio
    {"Is the signal below 200 MHz?",
     "RESULT:1", "RESULT:2", nullptr},

    // Node 5: Very short bursts
    {"Is the frequency near 315 MHz, 433 MHz,\n"
     "or 868 MHz?",
     "RESULT:3", "7", nullptr},

    // Node 6: Longer bursts
    {"Is the frequency between 2.4-2.5 GHz\n"
     "or 5.7-5.9 GHz?",
     "RESULT:4", "RESULT:5", nullptr},

    // Node 7: Short bursts, not key fob band
    {"Is the frequency between 902-928 MHz\n"
     "or near 433 MHz?",
     "RESULT:6", "RESULT:7", nullptr},
};
const int RFAssistantView::GUIDE_NODE_COUNT = 8;

// Result text for guide leaves
static const char* GUIDE_RESULTS[] = {
    // 0
    "LIKELY: FM BROADCAST RADIO\n\n"
    "Wide, high quality audio on a continuous\n"
    "carrier is the signature of FM broadcast.\n\n"
    "Confirm: frequency between 87.5-108 MHz\n"
    "Modulation: WFM (Wideband FM)\n"
    "Bandwidth: ~200 kHz\n"
    "What to do: Use Band Scanner in FM mode\n"
    "to find and label all stations in range.",

    // 1
    "LIKELY: BEACON, CARRIER, OR JAMMER\n\n"
    "A continuous signal below 200 MHz with\n"
    "no audio is often a beacon transmitter,\n"
    "a test carrier, or interference source.\n\n"
    "Could also be: CW Morse code beacon\n"
    "              Navigation aid (VOR/NDB)\n"
    "              Paging system carrier\n"
    "What to do: Use Modulation Identifier\n"
    "to analyze it further.",

    // 2
    "LIKELY: RADAR OR MILITARY SIGNAL\n\n"
    "A continuous signal above 200 MHz with\n"
    "no decodable audio may be:\n"
    "- Radar return signal\n"
    "- Military data link\n"
    "- Satellite beacon\n"
    "- WiFi/cellular infrastructure\n\n"
    "What to do: Check band plan for that\n"
    "frequency to identify the service.",

    // 3
    "LIKELY: KEY FOB / TPMS / GARAGE REMOTE\n\n"
    "Very short bursts near 315/433/868 MHz\n"
    "are almost certainly remote keyless entry.\n\n"
    "315 MHz = US car key fobs, TPMS sensors\n"
    "433 MHz = EU car key fobs, garage doors\n"
    "868 MHz = EU newer key fobs\n\n"
    "What to do:\n"
    "Use Signal Finder to locate the source.\n"
    "Use TPMS Counter if on 315/433 MHz.\n"
    "Use Modulation ID — will show FSK/OOK.",

    // 4
    "LIKELY: DRONE CONTROL OR WIFI\n\n"
    "Bursts in the 2.4 GHz or 5.8 GHz band:\n\n"
    "2.4 GHz: Drone RC control, WiFi, Bluetooth\n"
    "5.8 GHz: Drone video downlink, WiFi 5GHz\n\n"
    "Drone signals frequency-hop rapidly so\n"
    "you see burst activity across the band.\n\n"
    "What to do: Use Band Scanner in DRONE\n"
    "mode to survey the full drone bands.\n"
    "Use Signal Finder to locate the drone.",

    // 5
    "LIKELY: WALKIE TALKIE / LAND MOBILE\n\n"
    "Longer bursts outside the ISM bands\n"
    "are usually voice transmissions:\n\n"
    "VHF (136-174 MHz): Police, fire, business\n"
    "UHF (400-512 MHz): Public safety, GMRS\n"
    "FRS (462-468 MHz): Walkie talkies\n\n"
    "What to do: Use Modulation Identifier\n"
    "— will confirm NFM voice modulation.\n"
    "Try listening with audio enabled.",

    // 6
    "LIKELY: LONG RANGE DRONE LINK\n\n"
    "Bursts on 900-928 MHz or 433 MHz band\n"
    "matching drone protocol patterns:\n\n"
    "915 MHz: ELRS, TBS Crossfire, RFD900\n"
    "433 MHz: ELRS 433, DIY drone links\n\n"
    "These are long-range RC control links\n"
    "used by serious FPV/racing drone pilots.\n\n"
    "What to do: Use Band Scanner DRONE mode\n"
    "then Signal Finder to track the source.",

    // 7
    "LIKELY: IoT SENSOR OR UNKNOWN DEVICE\n\n"
    "Short bursts outside typical key fob\n"
    "or drone bands could be:\n\n"
    "- Smart home sensor (motion, door, water)\n"
    "- Weather station transmitter\n"
    "- Wireless alarm system\n"
    "- Industrial sensor\n"
    "- Baby monitor\n\n"
    "What to do: Note the exact frequency.\n"
    "Use Modulation Identifier to classify.\n"
    "Use Frequency Lookup to check band plan.",
};


// ═════════════════════════════════════════════════════════
//  CONSTRUCTOR
// ═════════════════════════════════════════════════════════
RFAssistantView::RFAssistantView(NavigationView& nav)
    : nav_(nav) {

    add_children({
        // Home screen
        &label_title, &label_sub,
        &button_error, &button_freq,
        &button_guide, &button_help,
        &button_trouble, &label_hint_home,
        // Result screen
        &text_result_title,
        &text_r0, &text_r1, &text_r2, &text_r3,
        &text_r4, &text_r5, &text_r6, &text_r7,
        &text_r8, &text_r9, &text_r10, &text_r11,
        &text_scroll_hint,
        // Input area
        &text_input_label,
        &field_freq_mhz, &label_dot,
        &field_freq_khz, &label_mhz_unit,
        &button_yes, &button_no,
        &field_list_idx, &button_show,
        &button_lookup,
        // Navigation
        &button_back_result,
        &button_scroll_up,
        &button_scroll_dn,
        &button_home,
    });

    show_home();

    // ── Home screen buttons ───────────────────────────────
    button_error.on_select = [this](Button&) {
        show_module(1);
    };
    button_freq.on_select = [this](Button&) {
        show_module(2);
    };
    button_guide.on_select = [this](Button&) {
        show_module(3);
    };
    button_help.on_select = [this](Button&) {
        show_module(4);
    };
    button_trouble.on_select = [this](Button&) {
        show_module(5);
    };

    // ── Frequency lookup ──────────────────────────────────
    button_lookup.on_select = [this](Button&) {
        if (module_ == 2) {
            const uint64_t freq =
                (uint64_t)field_freq_mhz.value() *
                1'000'000ULL +
                (uint64_t)field_freq_khz.value() * 1000ULL;
            lookup_frequency(freq);
        }
    };

    // ── Error search (use freq fields for index) ──────────
    // We reuse field_list_idx for error code selection

    // ── List index show button ────────────────────────────
    button_show.on_select = [this](Button&) {
        const int idx = field_list_idx.value() - 1;
        if (module_ == 1) {
            // Error codes — search by number
            if (idx < ERROR_DB_COUNT)
                display_result(
                    ERROR_DB[idx].title,
                    ERROR_DB[idx].body);
        } else if (module_ == 4) {
            // App help
            show_app_help(idx);
        } else if (module_ == 5) {
            // Troubleshooting
            show_trouble(idx);
        }
    };

    // ── Guide YES/NO ──────────────────────────────────────
    button_yes.on_select = [this](Button&) {
        guide_answer(true);
    };
    button_no.on_select = [this](Button&) {
        guide_answer(false);
    };

    // ── Navigation ────────────────────────────────────────
    button_home.on_select = [this](Button&) {
        show_home();
    };
    button_back_result.on_select = [this](Button&) {
        if (result_showing_) {
            result_showing_ = false;
            show_module(module_);
        } else {
            show_home();
        }
    };
    button_scroll_up.on_select = [this](Button&) {
        scroll_result(-3);
    };
    button_scroll_dn.on_select = [this](Button&) {
        scroll_result(3);
    };
}

void RFAssistantView::focus() {
    button_error.focus();
}

// ─────────────────────────────────────────
// Show home screen — hide result widgets
// ─────────────────────────────────────────
void RFAssistantView::show_home() {
    module_ = 0;
    result_showing_ = false;

    // Show home widgets
    label_title.hidden(false);
    label_sub.hidden(false);
    button_error.hidden(false);
    button_freq.hidden(false);
    button_guide.hidden(false);
    button_help.hidden(false);
    button_trouble.hidden(false);
    label_hint_home.hidden(false);

    // Hide result/input widgets
    text_result_title.hidden(true);
    text_r0.hidden(true);  text_r1.hidden(true);
    text_r2.hidden(true);  text_r3.hidden(true);
    text_r4.hidden(true);  text_r5.hidden(true);
    text_r6.hidden(true);  text_r7.hidden(true);
    text_r8.hidden(true);  text_r9.hidden(true);
    text_r10.hidden(true); text_r11.hidden(true);
    text_scroll_hint.hidden(true);
    text_input_label.hidden(true);
    field_freq_mhz.hidden(true);
    label_dot.hidden(true);
    field_freq_khz.hidden(true);
    label_mhz_unit.hidden(true);
    button_yes.hidden(true);
    button_no.hidden(true);
    field_list_idx.hidden(true);
    button_show.hidden(true);
    button_lookup.hidden(true);
    button_back_result.hidden(true);
    button_scroll_up.hidden(true);
    button_scroll_dn.hidden(true);
    button_home.hidden(true);

    set_dirty();
}

// ─────────────────────────────────────────
// Show a module's input screen
// ─────────────────────────────────────────
void RFAssistantView::show_module(int mod) {
    module_ = mod;
    result_showing_ = false;

    // Hide home
    label_title.hidden(true);
    label_sub.hidden(true);
    button_error.hidden(true);
    button_freq.hidden(true);
    button_guide.hidden(true);
    button_help.hidden(true);
    button_trouble.hidden(true);
    label_hint_home.hidden(true);

    // Show result area (initially empty)
    text_result_title.hidden(false);
    text_r0.hidden(false);  text_r1.hidden(false);
    text_r2.hidden(false);  text_r3.hidden(false);
    text_r4.hidden(false);  text_r5.hidden(false);
    text_r6.hidden(false);  text_r7.hidden(false);
    text_r8.hidden(false);  text_r9.hidden(false);
    text_r10.hidden(false); text_r11.hidden(false);
    text_scroll_hint.hidden(false);
    text_input_label.hidden(false);
    button_back_result.hidden(false);
    button_scroll_up.hidden(false);
    button_scroll_dn.hidden(false);
    button_home.hidden(false);

    // Clear result area
    text_result_title.set("");
    text_r0.set(""); text_r1.set(""); text_r2.set("");
    text_r3.set(""); text_r4.set(""); text_r5.set("");
    text_r6.set(""); text_r7.set(""); text_r8.set("");
    text_r9.set(""); text_r10.set(""); text_r11.set("");

    // Show module-specific input widgets
    field_freq_mhz.hidden(mod != 2);
    label_dot.hidden(mod != 2);
    field_freq_khz.hidden(mod != 2);
    label_mhz_unit.hidden(mod != 2);
    button_lookup.hidden(mod != 2);

    button_yes.hidden(mod != 3);
    button_no.hidden(mod != 3);

    field_list_idx.hidden(
        mod != 1 && mod != 4 && mod != 5);
    button_show.hidden(
        mod != 1 && mod != 4 && mod != 5);

    // Set input label
    switch (mod) {
        case 1:
            text_input_label.set(
                "Error # (1-" +
                to_string_dec_uint(ERROR_DB_COUNT) +
                "): use dial + SHOW");
            text_result_title.set(
                "ERROR DECODER — select # to look up");
            field_list_idx.set_range(1, ERROR_DB_COUNT);
            // Show error list summary
            for (int i = 0; i < 10 && i < ERROR_DB_COUNT;
                 i++) {
                Text* rows[10] = {
                    &text_r0,&text_r1,&text_r2,&text_r3,
                    &text_r4,&text_r5,&text_r6,&text_r7,
                    &text_r8,&text_r9};
                rows[i]->set(
                    to_string_dec_uint(i+1) + ". " +
                    ERROR_DB[i].title);
            }
            break;
        case 2:
            text_input_label.set(
                "Enter frequency in MHz.kHz:");
            text_result_title.set(
                "FREQUENCY LOOKUP — enter freq below");
            field_freq_mhz.set_value(315);
            field_freq_khz.set_value(0);
            break;
        case 3:
            text_input_label.set(
                "Answer YES or NO:");
            guide_start();
            break;
        case 4:
            text_input_label.set(
                "App # (1-" +
                to_string_dec_uint(HELP_DB_COUNT) +
                "): use dial + SHOW");
            text_result_title.set(
                "APP HELP — select app # below");
            field_list_idx.set_range(1, HELP_DB_COUNT);
            for (int i = 0; i < 10 && i < HELP_DB_COUNT;
                 i++) {
                Text* rows[10] = {
                    &text_r0,&text_r1,&text_r2,&text_r3,
                    &text_r4,&text_r5,&text_r6,&text_r7,
                    &text_r8,&text_r9};
                rows[i]->set(
                    to_string_dec_uint(i+1) + ". " +
                    HELP_DB[i].title);
            }
            break;
        case 5:
            text_input_label.set(
                "Issue # (1-" +
                to_string_dec_uint(TROUBLE_DB_COUNT) +
                "): use dial + SHOW");
            text_result_title.set(
                "TROUBLESHOOTING — select issue # below");
            field_list_idx.set_range(1, TROUBLE_DB_COUNT);
            for (int i = 0; i < 10 &&
                            i < TROUBLE_DB_COUNT; i++) {
                Text* rows[10] = {
                    &text_r0,&text_r1,&text_r2,&text_r3,
                    &text_r4,&text_r5,&text_r6,&text_r7,
                    &text_r8,&text_r9};
                rows[i]->set(
                    to_string_dec_uint(i+1) + ". " +
                    TROUBLE_DB[i].title);
            }
            break;
    }
    set_dirty();
}

// ─────────────────────────────────────────
// Frequency lookup
// ─────────────────────────────────────────
void RFAssistantView::lookup_frequency(uint64_t freq_hz) {
    const std::string label =
        bandplan::get_band_label(freq_hz);
    const std::string cat =
        bandplan::get_band_category(freq_hz);

    // Build freq string
    std::string freq_str;
    if (freq_hz >= 1'000'000'000) {
        const uint32_t ghz = freq_hz / 1'000'000'000;
        const uint32_t mhz =
            (freq_hz % 1'000'000'000) / 1'000'000;
        freq_str = to_string_dec_uint(ghz) + "." +
                   to_string_dec_uint(mhz) + " GHz";
    } else if (freq_hz >= 1'000'000) {
        const uint32_t mhz = freq_hz / 1'000'000;
        const uint32_t khz = (freq_hz % 1'000'000) / 1000;
        freq_str = to_string_dec_uint(mhz) + "." +
                   to_string_dec_uint(khz) + " MHz";
    } else {
        freq_str = to_string_dec_uint(
            freq_hz / 1000) + " kHz";
    }

    const std::string body =
        "FREQUENCY: " + freq_str + "\n\n"
        "SERVICE: " + label + "\n\n"
        "CATEGORY: " + cat + "\n\n"
        "DETAILS:\n" +
        get_freq_details(freq_hz, label, cat);

    display_result("Frequency: " + freq_str, body);
}

// ─────────────────────────────────────────
// Get detailed description for a frequency
// ─────────────────────────────────────────
// (Inline helper — returns context-sensitive info)
std::string get_freq_details_impl(
    uint64_t f,
    const std::string& label,
    const std::string& cat) {

    if (cat == "BROADCAST") {
        if (f < 30'000'000)
            return "AM broadcast radio band.\n"
                   "Used for talk radio, news, sports.\n"
                   "Modulation: AM (DSB)\n"
                   "Receive with: AM mode";
        return "FM broadcast radio band.\n"
               "Music, news, talk radio stations.\n"
               "Modulation: WFM\n"
               "Bandwidth: 200 kHz per station\n"
               "Receive with: WFM mode";
    }
    if (cat == "AVIATION")
        return "Aviation communications band.\n"
               "Used by pilots and air traffic control.\n"
               "Modulation: AM (required by law)\n"
               "Channel spacing: 25 kHz (8.33 in EU)\n"
               "Receive with: AM mode";
    if (cat == "EMERGENCY")
        return "EMERGENCY / DISTRESS FREQUENCY\n"
               "This is a monitored emergency channel.\n"
               "Do NOT transmit unless in distress.\n"
               "Always monitor — never block.";
    if (cat == "MARINE")
        return "Marine VHF radio band.\n"
               "Used by boats and coast guard.\n"
               "Modulation: NFM\n"
               "CH16 (156.8 MHz) = distress channel\n"
               "Receive with: NFM mode";
    if (cat == "WEATHER")
        return "NOAA Weather Radio.\n"
               "Continuous weather broadcasts 24/7.\n"
               "Covers local forecast, warnings, alerts.\n"
               "Modulation: NFM\n"
               "Receive with: NFM mode, audio on";
    if (cat == "HAM")
        return "Amateur (Ham) Radio band.\n"
               "Licensed operators only to transmit.\n"
               "Can receive freely.\n"
               "Common modes: NFM, AM, SSB, CW, digital\n"
               "License required: Technician or higher";
    if (cat == "DRONE")
        return "Drone / UAV frequency band.\n"
               "Uses frequency hopping spread spectrum.\n"
               "HackRF can detect but not follow hops.\n"
               "Use Band Scanner DRONE mode to survey.\n"
               "Use Signal Finder to locate drone.";
    if (cat == "KEYFOB")
        return "Remote Keyless Entry (key fob) band.\n"
               "Car remotes, garage doors, TPMS sensors.\n"
               "Modulation: FSK or OOK\n"
               "Modern cars use rolling codes (encrypted)\n"
               "Use TPMS Counter for vehicle detection.";
    if (cat == "BLUETOOTH")
        return "Bluetooth 2.4 GHz band.\n"
               "Classic BT: 79 channels, 1 MHz each\n"
               "BLE: 40 channels, 2 MHz each\n"
               "Hops 1600 times/sec (Classic)\n"
               "BLE advertising on 2402/2426/2480 MHz\n"
               "Use Band Scanner BT mode to survey.";
    if (cat == "MILITARY")
        return "Military frequency band.\n"
               "Used for military aviation and comms.\n"
               "Encrypted and frequency hopping likely.\n"
               "Can detect signal presence only.\n"
               "Receive with: AM or NFM mode";
    if (cat == "CELLULAR")
        return "Cellular / LTE band.\n"
               "Used by mobile phones and towers.\n"
               "Heavily encrypted — data not decodable.\n"
               "Can detect signal presence and strength.\n"
               "Useful for finding tower locations.";

    return "This frequency falls within: " + label + "\n"
           "Check the full band plan for more detail.\n"
           "Try the Modulation Identifier app to\n"
           "analyze any active signal here.";
}

std::string RFAssistantView::get_freq_details(
    uint64_t f,
    const std::string& label,
    const std::string& cat) const {
    return get_freq_details_impl(f, label, cat);
}

// ─────────────────────────────────────────
// Signal ID guide
// ─────────────────────────────────────────
void RFAssistantView::guide_start() {
    guide_node_ = 0;
    guide_show_node(0);
}

void RFAssistantView::guide_answer(bool yes) {
    if (guide_node_ < 0 ||
        guide_node_ >= GUIDE_NODE_COUNT) return;

    const auto& node = GUIDE_NODES[guide_node_];
    const char* next = yes ? node.answer_yes
                           : node.answer_no;

    if (!next) return;

    // Check if it's a result
    if (strncmp(next, "RESULT:", 7) == 0) {
        const int result_idx = atoi(next + 7);
        const int num_results =
            (int)(sizeof(GUIDE_RESULTS) /
                  sizeof(GUIDE_RESULTS[0]));
        if (result_idx < num_results) {
            display_result(
                "Signal Identified!",
                GUIDE_RESULTS[result_idx]);
        }
        return;
    }

    // Move to next node
    const int next_node = atoi(next);
    guide_node_ = next_node;
    guide_show_node(next_node);
}

void RFAssistantView::guide_show_node(int idx) {
    if (idx < 0 || idx >= GUIDE_NODE_COUNT) return;
    const auto& node = GUIDE_NODES[idx];

    text_result_title.set(
        "SIGNAL ID — Q" +
        to_string_dec_uint(idx + 1) + "/" +
        to_string_dec_uint(GUIDE_NODE_COUNT));

    // Clear result lines
    text_r0.set(""); text_r1.set(""); text_r2.set("");
    text_r3.set(""); text_r4.set(""); text_r5.set("");
    text_r6.set(""); text_r7.set(""); text_r8.set("");
    text_r9.set(""); text_r10.set(""); text_r11.set("");

    // Wrap question across result lines
    std::vector<std::string> lines;
    wrap_text(node.question, 28, lines);

    Text* rows[12] = {
        &text_r0, &text_r1, &text_r2, &text_r3,
        &text_r4, &text_r5, &text_r6, &text_r7,
        &text_r8, &text_r9, &text_r10, &text_r11};

    for (int i = 0; i < (int)lines.size() && i < 12; i++)
        rows[i]->set(lines[i]);

    text_input_label.set("Answer YES or NO:");
    set_dirty();
}

// ─────────────────────────────────────────
// App help
// ─────────────────────────────────────────
void RFAssistantView::show_app_help(int idx) {
    if (idx < 0 || idx >= HELP_DB_COUNT) return;
    display_result(HELP_DB[idx].title,
                   HELP_DB[idx].body);
}

void RFAssistantView::show_trouble(int idx) {
    if (idx < 0 || idx >= TROUBLE_DB_COUNT) return;
    display_result(TROUBLE_DB[idx].title,
                   TROUBLE_DB[idx].body);
}

// ─────────────────────────────────────────
// Generic result display with pagination
// ─────────────────────────────────────────
void RFAssistantView::display_result(
    const std::string& title,
    const std::string& body) {

    result_title_ = title;
    result_lines_.clear();
    result_scroll_ = 0;
    result_showing_ = true;

    // Wrap body text to 28 char width
    wrap_text(body, 28, result_lines_);

    text_result_title.set(title);
    text_input_label.set(
        "^ v to scroll | " +
        to_string_dec_uint((uint32_t)result_lines_.size()) +
        " lines total");

    // Hide input-specific widgets during result view
    button_yes.hidden(true);
    button_no.hidden(true);
    field_list_idx.hidden(true);
    button_show.hidden(true);
    field_freq_mhz.hidden(true);
    label_dot.hidden(true);
    field_freq_khz.hidden(true);
    label_mhz_unit.hidden(true);
    button_lookup.hidden(true);

    update_result_display();
}

void RFAssistantView::scroll_result(int delta) {
    const int max_scroll =
        std::max(0, (int)result_lines_.size() - 12);
    result_scroll_ = std::max(0,
        std::min(max_scroll, result_scroll_ + delta));
    update_result_display();
}

void RFAssistantView::update_result_display() {
    Text* rows[12] = {
        &text_r0, &text_r1, &text_r2, &text_r3,
        &text_r4, &text_r5, &text_r6, &text_r7,
        &text_r8, &text_r9, &text_r10, &text_r11};

    for (int i = 0; i < 12; i++) {
        const int idx = result_scroll_ + i;
        if (idx < (int)result_lines_.size())
            rows[i]->set(result_lines_[idx]);
        else
            rows[i]->set("");
    }

    const int total = (int)result_lines_.size();
    const int remaining = total - result_scroll_ - 12;
    if (remaining > 0) {
        text_scroll_hint.set(
            "v " + to_string_dec_uint(remaining) +
            " more lines");
    } else {
        text_scroll_hint.set(
            "-- end -- (" +
            to_string_dec_uint(total) + " lines)");
    }
    set_dirty();
}

// ─────────────────────────────────────────
// Word wrap text to N characters per line
// ─────────────────────────────────────────
void RFAssistantView::wrap_text(
    const std::string& text,
    int width,
    std::vector<std::string>& lines) const {

    std::string current_line;

    for (size_t i = 0; i < text.size(); i++) {
        const char c = text[i];

        if (c == '\n') {
            lines.push_back(current_line);
            current_line = "";
        } else if ((int)current_line.size() >= width) {
            // Try to break at last space
            const size_t last_space =
                current_line.rfind(' ');
            if (last_space != std::string::npos) {
                lines.push_back(
                    current_line.substr(0, last_space));
                current_line =
                    current_line.substr(last_space + 1) + c;
            } else {
                lines.push_back(current_line);
                current_line = std::string(1, c);
            }
        } else {
            current_line += c;
        }
    }

    if (!current_line.empty())
        lines.push_back(current_line);
}

}  // namespace ui
