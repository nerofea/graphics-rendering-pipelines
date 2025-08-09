#include <cairo/cairo.h>
#include <string>
#include <functional>
#include <cmath>
#include <iostream>

// HSV → RGB helper
static void hsv_to_rgb(double h, double s, double v, double& r, double& g, double& b) {
    double c = v * s;
    double x = c * (1 - std::fabs(std::fmod(h / 60.0, 2) - 1));
    double m = v - c;
    double r1=0,g1=0,b1=0;
    if      (h < 60)  { r1=c; g1=x; b1=0; }
    else if (h < 120) { r1=x; g1=c; b1=0; }
    else if (h < 180) { r1=0; g1=c; b1=x; }
    else if (h < 240) { r1=0; g1=x; b1=c; }
    else if (h < 300) { r1=x; g1=0; b1=c; }
    else              { r1=c; g1=0; b1=x; }
    r = r1 + m; g = g1 + m; b = b1 + m;
}

// Turn any string (e.g., a name, hex, or arbitrary key) into RGBA
static void string_to_rgba(const std::string& key, double& r, double& g, double& b, double& a) {
    // If user passes a #RRGGBB or #RRGGBBAA, honor it directly
    if (!key.empty() && key[0] == '#') {
        unsigned int rv=255, gv=255, bv=255, av=255;
        if (key.size() == 7)       { sscanf(key.c_str()+1, "%02x%02x%02x", &rv, &gv, &bv); }
        else if (key.size() == 9)  { sscanf(key.c_str()+1, "%02x%02x%02x%02x", &rv, &gv, &bv, &av); }
        r = rv/255.0; g = gv/255.0; b = bv/255.0; a = (key.size()==9 ? av/255.0 : 1.0);
        return;
    }

    // Hash → HSV (wrap hue around the wheel)
    size_t hsh = std::hash<std::string>{}(key);
    double hue = (hsh % 360);                 // 0..359 degrees
    double sat = 0.62 + ((hsh >> 9) & 0x3F) / 255.0 * 0.25; // ~0.62..0.87
    double val = 0.85;                        // keep it bright
    hsv_to_rgb(hue, sat, val, r, g, b);
    a = 1.0;
}

// Example draw: colored rounded rect background + text stub
void draw_colored_frame(const std::string& color_key, int W, int H, const char* out_png) {
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, W, H);
    cairo_t* cr = cairo_create(surface);

    // Background
    double r,g,b,a;
    string_to_rgba(color_key, r, g, b, a);
    cairo_set_source_rgba(cr, r, g, b, a);
    double radius = 24.0;
    // rounded rect path
    cairo_new_path(cr);
    cairo_arc(cr, W - radius, radius, radius, -M_PI/2, 0);
    cairo_arc(cr, W - radius, H - radius, radius, 0, M_PI/2);
    cairo_arc(cr, radius, H - radius, radius, M_PI/2, M_PI);
    cairo_arc(cr, radius, radius, radius, M_PI, 3*M_PI/2);
    cairo_close_path(cr);
    cairo_fill(cr);

    // Foreground white bar
    cairo_set_source_rgba(cr, 1, 1, 1, 0.92);
    cairo_rectangle(cr, 20, H/2 - 20, W - 40, 40);
    cairo_fill(cr);

    cairo_surface_write_to_png(surface, out_png);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

int main(int argc, char** argv) {
    // Usage: app <color_key> <out.png>
    std::string colorKey = (argc > 1) ? argv[1] : "Nerofea";
    const char* outPng   = (argc > 2) ? argv[2] : "frame.png";

    draw_colored_frame(colorKey, 800, 300, outPng);
    std::cout << "Wrote " << outPng << " using color key: " << colorKey << "\n";
    return 0;
}
