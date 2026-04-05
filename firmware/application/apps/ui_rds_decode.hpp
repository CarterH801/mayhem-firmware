/*
 * RDS Decoder for FM Band Scanner
 * File: firmware/application/apps/ui_rds_decode.hpp
 *
 * Decodes Radio Data System (RDS) from FM stations.
 * RDS is a 57 kHz subcarrier on FM broadcasts carrying:
 *   - PS  (Programme Service) — Station name (8 chars)
 *   - RT  (RadioText)         — Song title / artist (64 chars)
 *   - PTY (Programme Type)    — Music/News/Talk/Sports etc.
 *   - PI  (Programme ID)      — Unique station ID code
 *   - TP/TA — Traffic Programme / Traffic Announcement
 *   - CT  (Clock Time)        — UTC time broadcast
 *
 * Hooks into the Band Scanner — when tuned to an FM station
 * this module parses the RDS data and returns readable text.
 *
 * Place in: firmware/application/apps/
 */

#ifndef __UI_RDS_DECODE_H__
#define __UI_RDS_DECODE_H__

#include <string>
#include <cstdint>
#include <array>

namespace ui {

// ─────────────────────────────────────────────────────────
// RDS Programme Type codes → readable labels
// ─────────────────────────────────────────────────────────
static const char* RDS_PTY_LABELS[] = {
    "None",        "News",       "Information", "Sports",
    "Talk",        "Rock",       "Classic Rock","Adult Hits",
    "Soft Rock",   "Top 40",     "Country",     "Oldies",
    "Soft",        "Nostalgia",  "Jazz",        "Classical",
    "R&B",         "Soft R&B",   "Language",    "Religious Music",
    "Religious Talk","Personality","Public",    "College",
    "Spanish Talk","Spanish Music","Hip Hop",  "Unassigned",
    "Unassigned",  "Weather",    "Emergency Test","Emergency"
};

// ─────────────────────────────────────────────────────────
// Decoded RDS data for one station
// ─────────────────────────────────────────────────────────
struct RDSData {
    std::string station_name{""};   // PS — "Z100    "
    std::string radio_text{""};     // RT — "Taylor Swift - Shake It Off"
    std::string pty_label{""};      // PTY — "Top 40"
    uint16_t    pi_code{0};         // PI — unique station ID
    bool        traffic_prog{false};// TP
    bool        traffic_ann{false}; // TA
    bool        has_ps{false};
    bool        has_rt{false};
    bool        has_pty{false};

    // Build one-line display string for scanner
    std::string to_display() const {
        if (!has_ps && !has_rt) return "";
        std::string s = "";
        if (has_ps) {
            // Trim trailing spaces from station name
            std::string ps = station_name;
            while (!ps.empty() && ps.back() == ' ')
                ps.pop_back();
            s += ps;
        }
        if (has_rt && !radio_text.empty()) {
            if (!s.empty()) s += " · ";
            // Truncate RT to 32 chars for display
            if (radio_text.size() > 32)
                s += radio_text.substr(0, 32) + "...";
            else
                s += radio_text;
        }
        if (has_pty && !pty_label.empty()) {
            s += " [" + pty_label + "]";
        }
        return s;
    }
};

// ─────────────────────────────────────────────────────────
// RDS Block / Group decoder
//
// RDS data arrives as 4 blocks × 26 bits each (= one group)
// Groups are typed 0A-15B, each carrying different content
//
// This class accumulates raw RDS groups from the baseband
// and assembles the complete PS, RT, PTY, PI strings.
// ─────────────────────────────────────────────────────────
class RDSDecoder {
   public:
    // Reset all decoded data
    void reset() {
        data_ = RDSData{};
        ps_buf_.fill(' ');
        rt_buf_.fill(' ');
        rt_ab_flag_ = false;
        ps_complete_segments_ = 0;
        rt_complete_segments_ = 0;
    }

    // Feed one RDS group (4 blocks = 64 bits total)
    // blocks[0] = Block A (PI code)
    // blocks[1] = Block B (group type + flags)
    // blocks[2] = Block C (data)
    // blocks[3] = Block D (data)
    void feed_group(const uint16_t blocks[4]) {
        // Block A always contains PI code
        data_.pi_code = blocks[0];

        // Block B: group type (upper 4 bits) + version (bit 11)
        const uint8_t  group_type =
            (blocks[1] >> 12) & 0x0F;
        const bool     version_b  =
            (blocks[1] >> 11) & 0x01;
        const bool     tp         =
            (blocks[1] >> 10) & 0x01;
        const uint8_t  pty        =
            (blocks[1] >> 5) & 0x1F;

        data_.traffic_prog = tp;

        // Decode PTY
        if (pty < 32) {
            data_.pty_label  = RDS_PTY_LABELS[pty];
            data_.has_pty    = true;
        }

        // Group 0A/0B — Programme Service name (PS)
        if (group_type == 0) {
            const uint8_t seg = blocks[1] & 0x03;
            // TA flag in group 0
            data_.traffic_ann = (blocks[1] >> 4) & 0x01;

            // Each segment carries 2 chars of PS (8 chars total)
            if (!version_b) {
                ps_buf_[seg * 2]     =
                    (char)((blocks[3] >> 8) & 0xFF);
                ps_buf_[seg * 2 + 1] =
                    (char)(blocks[3] & 0xFF);
            } else {
                ps_buf_[seg * 2]     =
                    (char)((blocks[2] >> 8) & 0xFF);
                ps_buf_[seg * 2 + 1] =
                    (char)(blocks[2] & 0xFF);
            }

            ps_complete_segments_ |= (1 << seg);
            if (ps_complete_segments_ == 0x0F) {
                // All 4 segments received
                data_.station_name =
                    std::string(ps_buf_.begin(),
                                ps_buf_.end());
                data_.has_ps = true;
            }
        }

        // Group 2A/2B — RadioText (RT)
        if (group_type == 2) {
            const uint8_t seg    = blocks[1] & 0x0F;
            const bool    ab_flag =
                (blocks[1] >> 4) & 0x01;

            // A/B flag change means new RT message starting
            if (ab_flag != rt_ab_flag_) {
                rt_buf_.fill(' ');
                rt_complete_segments_ = 0;
                rt_ab_flag_           = ab_flag;
            }

            if (!version_b) {
                // Group 2A: 4 chars per segment (64 char RT)
                rt_buf_[seg * 4]     =
                    (char)((blocks[2] >> 8) & 0xFF);
                rt_buf_[seg * 4 + 1] =
                    (char)(blocks[2] & 0xFF);
                rt_buf_[seg * 4 + 2] =
                    (char)((blocks[3] >> 8) & 0xFF);
                rt_buf_[seg * 4 + 3] =
                    (char)(blocks[3] & 0xFF);
            } else {
                // Group 2B: 2 chars per segment (32 char RT)
                rt_buf_[seg * 2]     =
                    (char)((blocks[3] >> 8) & 0xFF);
                rt_buf_[seg * 2 + 1] =
                    (char)(blocks[3] & 0xFF);
            }

            rt_complete_segments_ |= (1 << seg);

            // Check for end-of-RT marker (0x0D)
            bool end_found = false;
            for (int i = 0; i < 64; i++) {
                if (rt_buf_[i] == 0x0D) {
                    // Null terminate here
                    data_.radio_text =
                        std::string(rt_buf_.begin(),
                                    rt_buf_.begin() + i);
                    data_.has_rt = true;
                    end_found    = true;
                    break;
                }
            }
            if (!end_found && rt_complete_segments_ == 0xFFFF) {
                // All 16 segments received (no terminator)
                data_.radio_text =
                    std::string(rt_buf_.begin(),
                                rt_buf_.end());
                data_.has_rt = true;
            }
        }
    }

    const RDSData& data() const { return data_; }

   private:
    RDSData                  data_{};
    std::array<char, 8>      ps_buf_{};
    std::array<char, 64>     rt_buf_{};
    bool                     rt_ab_flag_{false};
    uint8_t                  ps_complete_segments_{0};
    uint32_t                 rt_complete_segments_{0};
};

}  // namespace ui

#endif /*__UI_RDS_DECODE_H__*/
