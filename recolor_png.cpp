#include <librsvg/rsvg.h>
#include "recolor_png.hpp"
#include <cairo/cairo.h>


// parse "#RRGGBB" or "#RRGGBBAA" (alpha optional)
static bool parse_hex_rgba(const std::string& hex, double& r,double& g,double& b,double& a){
    if (hex.empty() || hex[0] != '#') return false;
    unsigned rv=0,gv=0,bv=0,av=255;
    if (hex.size()==7)       { if (sscanf(hex.c_str()+1,"%02x%02x%02x",&rv,&gv,&bv)!=3) return false; }
    else if (hex.size()==9 ) { if (sscanf(hex.c_str()+1,"%02x%02x%02x%02x",&rv,&gv,&bv,&av)!=4) return false; }
    else return false;
    r=rv/255.0; g=gv/255.0; b=bv/255.0; a=av/255.0; return true;
}


static void tint_png_multiply(const char* in_png, const char* out_png,
                              double r, double g, double b, double a = 1.0) {
    cairo_surface_t* src = cairo_image_surface_create_from_png(in_png);
    int W = cairo_image_surface_get_width(src);
    int H = cairo_image_surface_get_height(src);

    cairo_surface_t* dst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, W, H);
    cairo_t* cr = cairo_create(dst);

    // 1) Draw original
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface(cr, src, 0, 0);
    cairo_paint(cr);

    // 2) Multiply tint only where the PNG has alpha
    cairo_set_operator(cr, CAIRO_OPERATOR_MULTIPLY); // try OVERLAY or SOFT_LIGHT too
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_mask_surface(cr, src, 0, 0); // use PNG’s alpha as mask

    cairo_surface_write_to_png(dst, out_png);
    cairo_destroy(cr);
    cairo_surface_destroy(dst);
    cairo_surface_destroy(src);
}

static void recolor_png_with_alpha_mask(const char* in_png, const char* out_png,
                                        double r, double g, double b, double a = 1.0) {
    cairo_surface_t* mask = cairo_image_surface_create_from_png(in_png);
    int W = cairo_image_surface_get_width(mask);
    int H = cairo_image_surface_get_height(mask);

    cairo_surface_t* dst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, W, H);
    cairo_t* cr = cairo_create(dst);

    // Clear
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);

    // Paint solid color through the PNG's alpha
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(cr, r, g, b, a);
    cairo_mask_surface(cr, mask, 0, 0);

    cairo_surface_write_to_png(dst, out_png);
    cairo_destroy(cr);
    cairo_surface_destroy(dst);
    cairo_surface_destroy(mask);
}


#include <cstdint>
#include <cmath>

static inline void rgb_to_hsv(double R, double G, double B, double& h, double& s, double& v){
    double mx = std::fmax(R, std::fmax(G, B));
    double mn = std::fmin(R, std::fmin(G, B));
    v = mx; double d = mx - mn; s = (mx == 0 ? 0 : d / mx);
    if (d == 0) { h = 0; return; }
    if (mx == R) h = 60.0 * std::fmod(((G - B) / d), 6.0);
    else if (mx == G) h = 60.0 * (((B - R) / d) + 2.0);
    else h = 60.0 * (((R - G) / d) + 4.0);
    if (h < 0) h += 360.0;
}
static inline void hsv_to_rgb(double h, double s, double v, double& R, double& G, double& B){
    double C = v * s;
    double X = C * (1 - std::fabs(std::fmod(h/60.0, 2) - 1));
    double m = v - C;
    double r=0,g=0,b=0;
    if (h < 60)      { r=C; g=X; b=0; }
    else if (h<120 ) { r=X; g=C; b=0; }
    else if (h<180 ) { r=0; g=C; b=X; }
    else if (h<240 ) { r=0; g=X; b=C; }
    else if (h<300 ) { r=X; g=0; b=C; }
    else             { r=C; g=0; b=X; }
    R=r+m; G=g+m; B=b+m;
}

static void hue_shift_png(const char* in_png, const char* out_png, double hue_delta_deg){
    cairo_surface_t* s = cairo_image_surface_create_from_png(in_png);
    cairo_surface_flush(s);

    int W = cairo_image_surface_get_width(s);
    int H = cairo_image_surface_get_height(s);
    uint8_t* data = cairo_image_surface_get_data(s);
    int stride = cairo_image_surface_get_stride(s);

    for (int y=0; y<H; ++y){
        uint32_t* row = reinterpret_cast<uint32_t*>(data + y*stride);
        for (int x=0; x<W; ++x){
            uint32_t p = row[x];
            uint8_t a = (p >> 24) & 0xFF, r = (p >> 16) & 0xFF, g = (p >> 8) & 0xFF, b = p & 0xFF;
            if (a == 0) continue; // fully transparent

            // Un-premultiply
            double R = r/255.0, G = g/255.0, B = b/255.0, A = a/255.0;
            if (A > 0) { R /= A; G /= A; B /= A; }

            // Shift hue
            double h,s,v; rgb_to_hsv(R,G,B,h,s,v);
            h = std::fmod(h + hue_delta_deg + 360.0, 360.0);
            hsv_to_rgb(h,s,v,R,G,B);

            // Re-premultiply
            R = std::min(std::max(R,0.0),1.0);
            G = std::min(std::max(G,0.0),1.0);
            B = std::min(std::max(B,0.0),1.0);
            uint8_t R8 = (uint8_t)std::round(R * A * 255.0);
            uint8_t G8 = (uint8_t)std::round(G * A * 255.0);
            uint8_t B8 = (uint8_t)std::round(B * A * 255.0);

            row[x] = (a<<24) | (R8<<16) | (G8<<8) | (B8);
        }
    }
    cairo_surface_mark_dirty(s);
    cairo_surface_write_to_png(s, out_png);
    cairo_surface_destroy(s);
}


#include <algorithm> // add this
#include <iostream>

int main() {
    tint_png_multiply("in.png", "mul.png", 0.9, 0.25, 0.2, 1.0);
    recolor_png_with_alpha_mask("in.png", "flat.png", 0.2, 0.55, 0.9, 1.0);
    hue_shift_png("in.png", "hue.png", 40.0);
    std::cout << "done\n";
    return 0;
}
