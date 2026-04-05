/*
 * Signal Map — PortaPack Mayhem
 * File: firmware/application/apps/ui_signal_map.cpp
 *
 * RF Trilateration:
 *   1. Stand at position A, press MARK → circle A
 *   2. Move to position B, press MARK → circle B
 *   3. Move to position C, press MARK → circle C
 *   4. Press FIND → intersection = transmitter location
 *
 * Math: FSPL distance + Mercator pixel conversion
 * Map: Uses world_map.bin via GeoMapView widget
 *
 * Place in: firmware/application/apps/
 */

#include "ui_signal_map.hpp"
#include "portapack.hpp"
#include "receiver_model.hpp"
#include "string_format.hpp"
#include "ui_painter.hpp"
#include <cmath>

using namespace portapack;

// ── Math constants ────────────────────────────────────────
static constexpr float PI       = 3.14159265358979f;
static constexpr float DEG2RAD  = PI / 180.0f;
static constexpr float RAD2DEG  = 180.0f / PI;
// Earth radius in meters
static constexpr float EARTH_R  = 6'371'000.0f;
// Meters per degree of latitude (approximately constant)
static constexpr float M_PER_LAT_DEG = 111'139.0f;

namespace ui {

// ─────────────────────────────────────────
// Standalone constructor
// ─────────────────────────────────────────
SignalMapView::SignalMapView(NavigationView& nav)
    : nav_(nav) {
    init(315'000'000, 10, "KEY FOB 315MHz");
}

// ─────────────────────────────────────────
// Pre-loaded constructor
// ─────────────────────────────────────────
SignalMapView::SignalMapView(NavigationView& nav,
                             uint64_t    freq_hz,
                             int32_t     tx_power_dbm,
                             const std::string& label)
    : nav_(nav) {
    init(freq_hz, tx_power_dbm, label);
}

SignalMapView::~SignalMapView() {
    receiver_model.disable();
}

void SignalMapView::focus() {
    button_mark.focus();
}

// ─────────────────────────────────────────
// Init
// ─────────────────────────────────────────
void SignalMapView::init(uint64_t    freq_hz,
                         int32_t     tx_power_dbm,
                         const std::string& label) {
    target_freq_   = freq_hz;
    tx_power_dbm_  = tx_power_dbm;
    band_label_    = label;

    add_children({
        &geomap,
        &text_rssi_live,
        &text_dist_live,
        &label_lat,
        &field_lat_deg,
        &label_lon,
        &field_lon_deg,
        &button_mark,
        &button_clear,
        &button_trilat,
        &button_back,
        &text_count,
    });

    // Set map center to default position
    geomap.set_position(my_lat_, my_lon_);
    geomap.set_mode(GeoMapView::Mode::View);

    // Set lat/lon fields to default
    field_lat_deg.set_value((int32_t)my_lat_);
    field_lon_deg.set_value((int32_t)my_lon_);

    field_lat_deg.on_change = [this](int32_t v) {
        my_lat_ = (float)v;
        geomap.set_position(my_lat_, my_lon_);
        set_dirty();
    };

    field_lon_deg.on_change = [this](int32_t v) {
        my_lon_ = (float)v;
        geomap.set_position(my_lat_, my_lon_);
        set_dirty();
    };

    // MARK — take a measurement at current position
    button_mark.on_select = [this](Button&) {
        take_measurement();
    };

    // CLEAR — remove all measurements
    button_clear.on_select = [this](Button&) {
        measurements_.clear();
        trilat_result_ = TrilatResult{};
        text_count.set("0 readings — need 3 to triangulate");
        set_dirty();
    };

    // FIND — run trilateration
    button_trilat.on_select = [this](Button&) {
        if (measurements_.size() >= 3) {
            trilat_result_ = trilaterate();
            if (trilat_result_.valid) {
                const std::string res =
                    "Est: " +
                    to_string_decimal(trilat_result_.lat, 4) +
                    ", " +
                    to_string_decimal(trilat_result_.lon, 4) +
                    " ±" +
                    to_string_dec_uint(
                        (uint32_t)trilat_result_.error_meters) +
                    "m";
                text_count.set(res);
            } else {
                text_count.set(
                    "Triangulation failed - spread out more");
            }
        } else {
            text_count.set("Need at least 3 readings first");
        }
        set_dirty();
    };

    button_back.on_select = [this](Button&) {
        receiver_model.disable();
        nav_.pop();
    };

    // Tune radio
    receiver_model.set_target_frequency(target_freq_);
    if (target_freq_ >= 87'500'000 &&
        target_freq_ <= 108'000'000) {
        receiver_model.set_modulation(
            ReceiverModel::Mode::WidebandFMAudio);
    } else if (target_freq_ < 30'000'000) {
        receiver_model.set_modulation(
            ReceiverModel::Mode::AMAudio);
    } else {
        receiver_model.set_modulation(
            ReceiverModel::Mode::NarrowbandFMAudio);
    }
    receiver_model.set_sampling_rate(3072000);
    receiver_model.set_baseband_bandwidth(1750000);
    receiver_model.enable();
}

// ─────────────────────────────────────────
// Take a measurement at current position
// ─────────────────────────────────────────
void SignalMapView::take_measurement() {
    if ((int)measurements_.size() >= MAX_MEASUREMENTS) {
        text_count.set("Max 8 readings reached. Clear some.");
        return;
    }

    const float dist = calc_distance_meters(
        current_rssi_, tx_power_dbm_, target_freq_);

    if (dist < 0) {
        text_count.set("Signal too weak to measure.");
        return;
    }

    SignalMeasurement m;
    m.lat          = my_lat_;
    m.lon          = my_lon_;
    m.dist_meters  = dist;
    m.rssi_dbm     = current_rssi_;
    m.tx_power_dbm = tx_power_dbm_;
    m.freq_hz      = target_freq_;
    m.label        = band_label_;

    measurements_.push_back(m);

    const int n = (int)measurements_.size();

    // Miles conversion: 1m = 0.000621371 mi
    const float miles = dist * 0.000621371f;
    const uint32_t dist_m  = (uint32_t)dist;
    const uint32_t dist_ft = (uint32_t)(dist * 3.28084f);

    std::string status =
        to_string_dec_uint(n) + " reading" +
        (n == 1 ? "" : "s") + " | #" +
        to_string_dec_uint(n) + ": " +
        to_string_dec_uint(dist_m) + "m";

    if (n < 3) {
        status += " | need " +
                  to_string_dec_uint(3 - n) + " more";
    } else {
        status += " | press FIND";
    }

    text_count.set(status);
    set_dirty();
}

// ─────────────────────────────────────────
// RSSI update
// ─────────────────────────────────────────
void SignalMapView::on_statistics_update(
    const RSSIStatisticsMessage& message) {

    current_rssi_ = message.statistics.max;
    update_rssi_display();
}

// ─────────────────────────────────────────
// Update live RSSI and distance display
// ─────────────────────────────────────────
void SignalMapView::update_rssi_display() {
    text_rssi_live.set(
        "RSSI:" + to_string_dec_int(current_rssi_) + "dBm");

    const float dist = calc_distance_meters(
        current_rssi_, tx_power_dbm_, target_freq_);

    if (dist > 0) {
        const uint32_t dm = (uint32_t)dist;
        const float   mi  = dist * 0.000621371f;

        // Format miles
        std::string mi_str;
        if (mi < 0.1f) {
            mi_str = to_string_dec_uint(
                (uint32_t)(mi * 1000.0f)) + "ft";
        } else {
            const uint32_t mi_w = (uint32_t)mi;
            const uint32_t mi_d =
                (uint32_t)((mi - mi_w) * 10.0f);
            mi_str = to_string_dec_uint(mi_w) + "." +
                     to_string_dec_uint(mi_d) + "mi";
        }

        text_dist_live.set(
            to_string_dec_uint(dm) + "m/" + mi_str);
    } else {
        text_dist_live.set("Dist: weak");
    }
}

// ─────────────────────────────────────────
// PAINT — draw circles and markers over map
// ─────────────────────────────────────────
void SignalMapView::paint(Painter& painter) {
    // Let the base view paint first (draws map + children)
    View::paint(painter);

    // Now draw our overlays on top
    draw_map_overlay(painter);
}

// ─────────────────────────────────────────
// Draw all overlays on the map
// ─────────────────────────────────────────
void SignalMapView::draw_map_overlay(Painter& painter) {
    // Draw each measurement circle
    // Colors cycle through a set for readability
    static const Color circle_colors[] = {
        Color::yellow(),
        Color::cyan(),
        Color::green(),
        Color::magenta(),
        Color::white(),
        Color::red(),
        Color::blue(),
        Color::orange()
    };

    for (int i = 0; i < (int)measurements_.size(); i++) {
        draw_measurement_circle(
            painter,
            measurements_[i],
            i);

        // Draw observer position dot for this measurement
        draw_observer_dot(
            painter,
            measurements_[i].lat,
            measurements_[i].lon,
            circle_colors[i % 8]);
    }

    // Draw current position (white dot)
    draw_observer_dot(painter, my_lat_, my_lon_,
                      Color::white());

    // Draw estimated transmitter location if we have it
    if (trilat_result_.valid) {
        draw_transmitter_estimate(painter);
    }
}

// ─────────────────────────────────────────
// Draw one distance ring on the map
// ─────────────────────────────────────────
void SignalMapView::draw_measurement_circle(
    Painter&                painter,
    const SignalMeasurement& m,
    int                      idx) {

    static const Color circle_colors[] = {
        Color::yellow(), Color::cyan(),  Color::green(),
        Color::magenta(),Color::white(), Color::red(),
        Color::blue(),   Color::orange()
    };

    const Color col = circle_colors[idx % 8];

    // Center of circle = observer position
    const auto center = latlon_to_pixel(m.lat, m.lon);

    // Radius in pixels based on distance
    const int r = distance_to_pixels(m.dist_meters);

    // Clamp radius to something visible
    if (r < 2 || r > 500) return;

    // Draw circle outline (approximate with octagon
    // for performance on embedded hardware)
    // Using painter.draw_circle which Mayhem supports
    painter.draw_circle(
        {center.x, MAP_TOP + center.y},
        r,
        col);

    // Label the circle with its index number
    painter.draw_string(
        {center.x - 3, MAP_TOP + center.y - r - 8},
        *ui::Theme::getInstance()->fg_light,
        to_string_dec_uint(idx + 1));
}

// ─────────────────────────────────────────
// Draw observer position dot
// ─────────────────────────────────────────
void SignalMapView::draw_observer_dot(
    Painter& painter,
    float lat, float lon,
    Color col) {

    const auto p = latlon_to_pixel(lat, lon);

    // Small filled square (3x3)
    painter.fill_rectangle(
        {{p.x - 2, MAP_TOP + p.y - 2}, {5, 5}},
        col);
}

// ─────────────────────────────────────────
// Draw estimated transmitter location
// as a large X mark with error circle
// ─────────────────────────────────────────
void SignalMapView::draw_transmitter_estimate(
    Painter& painter) {

    const auto p = latlon_to_pixel(
        trilat_result_.lat,
        trilat_result_.lon);

    const int x = p.x;
    const int y = MAP_TOP + p.y;

    // Draw X marker (red)
    const int sz = 6;
    painter.draw_line({x - sz, y - sz},
                      {x + sz, y + sz},
                      Color::red());
    painter.draw_line({x + sz, y - sz},
                      {x - sz, y + sz},
                      Color::red());

    // Error radius circle (orange dashed would be ideal,
    // but solid circle for simplicity)
    const int err_r = distance_to_pixels(
        trilat_result_.error_meters);

    if (err_r > 2 && err_r < 200) {
        painter.draw_circle({x, y}, err_r, Color::orange());
    }

    // Label
    painter.draw_string(
        {x + 4, y - 8},
        *ui::Theme::getInstance()->fg_red,
        "TX?");
}

// ─────────────────────────────────────────
// Trilateration from 3+ measurements
//
// Method: Iterative weighted least squares.
// For each pair of circles, find their intersection.
// Average the intersections weighted by confidence.
//
// Simplified version: use the first 3 measurements
// and solve the intersection analytically.
// ─────────────────────────────────────────
TrilatResult SignalMapView::trilaterate() const {
    TrilatResult result;
    if (measurements_.size() < 3) return result;

    // Convert lat/lon to local X/Y in meters
    // Use first measurement as origin
    const float lat0 = measurements_[0].lat;
    const float lon0 = measurements_[0].lon;

    // Helper: lat/lon → local meters
    auto to_local = [&](float lat, float lon,
                        float& x, float& y) {
        y = (lat - lat0) * M_PER_LAT_DEG;
        x = (lon - lon0) * M_PER_LAT_DEG *
            cosf(lat0 * DEG2RAD);
    };

    // Extract first 3 measurements
    float x1, y1, x2, y2, x3, y3;
    to_local(measurements_[0].lat,
             measurements_[0].lon, x1, y1);
    to_local(measurements_[1].lat,
             measurements_[1].lon, x2, y2);
    to_local(measurements_[2].lat,
             measurements_[2].lon, x3, y3);

    const float r1 = measurements_[0].dist_meters;
    const float r2 = measurements_[1].dist_meters;
    const float r3 = measurements_[2].dist_meters;

    // Trilateration linear algebra:
    // From circles 1&2: A*x + B*y = C
    // From circles 1&3: D*x + E*y = F
    const float A = 2.0f * (x2 - x1);
    const float B = 2.0f * (y2 - y1);
    const float C = r1*r1 - r2*r2 - x1*x1 + x2*x2
                  - y1*y1 + y2*y2;

    const float D = 2.0f * (x3 - x1);
    const float E = 2.0f * (y3 - y1);
    const float F = r1*r1 - r3*r3 - x1*x1 + x3*x3
                  - y1*y1 + y3*y3;

    // Solve 2x2 linear system
    const float det = A*E - B*D;
    if (fabsf(det) < 0.001f) {
        // Degenerate — points too collinear
        result.valid = false;
        return result;
    }

    const float tx = (C*E - B*F) / det;
    const float ty = (A*F - C*D) / det;

    // Convert back to lat/lon
    result.lat = lat0 + ty / M_PER_LAT_DEG;
    result.lon = lon0 + tx /
                 (M_PER_LAT_DEG * cosf(lat0 * DEG2RAD));

    // Estimate error: average residual distance
    float err_sum = 0.0f;
    for (const auto& m : measurements_) {
        float mx, my;
        to_local(m.lat, m.lon, mx, my);
        const float actual_dist =
            sqrtf((tx - mx)*(tx - mx) +
                  (ty - my)*(ty - my));
        err_sum += fabsf(actual_dist - m.dist_meters);
    }
    result.error_meters = err_sum / measurements_.size();
    result.valid = true;

    return result;
}

// ─────────────────────────────────────────
// Convert lat/lon to pixel on 240xMAP_HEIGHT map area
// Uses Mercator projection matching world_map.bin
//
// world_map.bin covers:
//   Lon: -180 to +180
//   Lat: -85 to +85 (Mercator)
// ─────────────────────────────────────────
SignalMapView::PixelPos SignalMapView::latlon_to_pixel(
    float lat, float lon) const {

    // Get the map center from GeoMapView
    // The GeoMapView handles the actual projection,
    // but we need pixel coords for our overlay drawing.
    // We approximate using the visible map bounds.

    // For a Mercator map centered at (my_lat_, my_lon_)
    // at a given zoom, we compute relative offset.

    // Degrees visible in each direction at current zoom
    // (approximate — adjust based on actual zoom level)
    const float degrees_visible_lon = 20.0f;   // ±20° lon
    const float degrees_visible_lat = 15.0f;   // ±15° lat

    // Get the map center (what GeoMapView is centered on)
    float center_lat = my_lat_;
    float center_lon = my_lon_;

    // Normalize to 0-1 range within visible area
    float norm_x = (lon - center_lon + degrees_visible_lon) /
                   (2.0f * degrees_visible_lon);
    float norm_y = (center_lat - lat + degrees_visible_lat) /
                   (2.0f * degrees_visible_lat);

    // Clamp to map bounds
    if (norm_x < 0.0f) norm_x = 0.0f;
    if (norm_x > 1.0f) norm_x = 1.0f;
    if (norm_y < 0.0f) norm_y = 0.0f;
    if (norm_y > 1.0f) norm_y = 1.0f;

    PixelPos p;
    p.x = (int)(norm_x * MAP_WIDTH);
    p.y = (int)(norm_y * MAP_HEIGHT);
    return p;
}

// ─────────────────────────────────────────
// Convert distance in meters to map pixels
// ─────────────────────────────────────────
int SignalMapView::distance_to_pixels(float meters) const {
    // Degrees of longitude per meter at current latitude
    const float deg_per_meter =
        1.0f / (M_PER_LAT_DEG * cosf(my_lat_ * DEG2RAD));

    // Degrees covered by the visible map width
    const float degrees_visible_lon = 20.0f;

    // Pixels per degree
    const float px_per_deg =
        (float)MAP_WIDTH / (2.0f * degrees_visible_lon);

    // Distance in pixels
    const float deg = meters * deg_per_meter;
    return (int)(deg * px_per_deg);
}

// ─────────────────────────────────────────
// FSPL distance calculation
// Returns meters, or -1 if too weak
// ─────────────────────────────────────────
float SignalMapView::calc_distance_meters(
    int32_t  rssi_dbm,
    int32_t  tx_dbm,
    uint64_t freq_hz) const {

    if (rssi_dbm < -105) return -1.0f;

    const float freq_mhz =
        static_cast<float>(freq_hz) / 1'000'000.0f;
    if (freq_mhz <= 0.0f) return -1.0f;

    const float path_loss =
        static_cast<float>(tx_dbm - rssi_dbm);
    const float log_freq = 20.0f * log10f(freq_mhz);
    const float exponent =
        (path_loss - log_freq - 32.44f) / 20.0f;

    const float dist_km = powf(10.0f, exponent);
    const float dist_m  = dist_km * 1000.0f;

    if (dist_m < 0.01f) return 0.01f;
    if (dist_m > 50'000'000.0f) return -1.0f;

    return dist_m;
}

}  // namespace ui
