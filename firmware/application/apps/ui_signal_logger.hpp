/*
 * Auto Signal Logger
 * File: firmware/application/apps/ui_signal_logger.hpp
 *
 * Automatically logs every detected signal to CSV on SD card.
 * Format: timestamp, frequency_hz, rssi_dbm, band_label, category
 * File: /LOGS/signals_YYYYMMDD.csv
 *
 * Used as a module by the Band Scanner — logs every
 * add_station() call automatically when enabled.
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_SIGNAL_LOGGER_H__
#define __UI_SIGNAL_LOGGER_H__

#include "file.hpp"
#include "string_format.hpp"
#include "portapack.hpp"
#include "rtc_time.hpp"

#include <string>
#include <cstdint>

namespace ui {

class SignalLogger {
   public:

    // Open or create today's log file
    // Returns true if ready to log
    bool open() {
        if (is_open_) return true;

        // Build filename: /LOGS/signals_YYYYMMDD.csv
        const auto now = rtc_time::now();
        const std::string fname =
            "/LOGS/signals_" +
            to_string_dec_uint(now.year())  +
            pad2(now.month())  +
            pad2(now.day())    +
            ".csv";

        // Create /LOGS directory if needed
        make_new_directory("/LOGS");

        // Open file — append mode
        if (log_file_.open(fname, true, true)) {
            is_open_ = true;

            // Write header if file is new / empty
            if (log_file_.size() == 0) {
                const std::string header =
                    "timestamp,freq_hz,freq_label,"
                    "rssi_dbm,band_label,category\n";
                log_file_.write(
                    header.c_str(), header.size());
            }
            return true;
        }
        return false;
    }

    void close() {
        if (is_open_) {
            log_file_.close();
            is_open_ = false;
        }
    }

    // Log one signal entry
    void log(uint64_t    freq_hz,
             int32_t     rssi_dbm,
             const std::string& band_label,
             const std::string& category) {

        if (!is_open_) return;

        const auto now = rtc_time::now();

        // Timestamp: HH:MM:SS
        const std::string ts =
            pad2(now.hour())   + ":" +
            pad2(now.minute()) + ":" +
            pad2(now.second());

        // Frequency as readable string
        const std::string freq_str =
            format_freq_readable(freq_hz);

        // Build CSV row
        const std::string row =
            ts + "," +
            to_string_dec_uint((uint32_t)(freq_hz / 1000)) +
            "000," +     // freq in Hz
            freq_str + "," +
            to_string_dec_int(rssi_dbm) + "," +
            band_label + "," +
            category + "\n";

        log_file_.write(row.c_str(), row.size());
        entry_count_++;
    }

    bool     is_open()     const { return is_open_; }
    uint32_t entry_count() const { return entry_count_; }

   private:
    File     log_file_{};
    bool     is_open_{false};
    uint32_t entry_count_{0};

    std::string pad2(int n) const {
        return (n < 10 ? "0" : "") +
               to_string_dec_uint(n);
    }

    std::string format_freq_readable(uint64_t f) const {
        if (f >= 1'000'000'000) {
            const uint32_t ghz = f / 1'000'000'000;
            const uint32_t mhz =
                (f % 1'000'000'000) / 1'000'000;
            return to_string_dec_uint(ghz) + "." +
                   (mhz < 100 ? "0" : "") +
                   (mhz < 10  ? "0" : "") +
                   to_string_dec_uint(mhz) + "GHz";
        } else if (f >= 1'000'000) {
            const uint32_t mhz = f / 1'000'000;
            const uint32_t khz = (f % 1'000'000) / 1000;
            return to_string_dec_uint(mhz) + "." +
                   (khz < 100 ? "0" : "") +
                   (khz < 10  ? "0" : "") +
                   to_string_dec_uint(khz) + "MHz";
        } else {
            return to_string_dec_uint(f / 1000) + "kHz";
        }
    }
};

}  // namespace ui

#endif /*__UI_SIGNAL_LOGGER_H__*/
