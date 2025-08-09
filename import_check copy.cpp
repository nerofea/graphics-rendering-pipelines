// app.cpp
#include <cairo/cairo.h>
#include <librsvg/rsvg.h>
#include <glib.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

// ---------- load surface from PNG or SVG ----------
static cairo_surface_t* load_png(const std::string& path){
    cairo_surface_t* s = cairo_image_surface_create_from_png(path.c_str());
    if (cairo_surface_status(s) != CAIRO_STATUS_SUCCESS){ cairo_surface_destroy(s); return nullptr; }
    return s;
}

static cairo_surface_t* render_svg(const std::string& path, int w, int h){
    GError* err=nullptr;
    RsvgHandle* hnd = rsvg_handle_new_from_file(path.c_str(), &err);
    if (!hnd){ if (err){ g_error_free(err);} return nullptr; }

    // If no size provided, use intrinsic SVG size
    int outW=w, outH=h;
    if (outW<=0 || outH<=0){
        RsvgDimensionData dim{};
        rsvg_handle_get_dimensions(hnd, &dim); // works broadly; fine for our purpose
        outW = (outW>0? outW : dim.width);
        outH = (outH>0? outH : dim.height);
        if (outW<=0) outW = 800;
        if (outH<=0) outH = 600;
    }

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, outW, outH);
    cairo_t* cr = cairo_create(surf);

    RsvgRectangle vp{0.0,0.0,(double)outW,(double)outH};
    if (!rsvg_handle_render_document(hnd, cr, &vp, &err)){
        cairo_destroy(cr);
        cairo_surface_destroy(surf);
        g_object_unref(hnd);
        if (err){ g_error_free(err); }
        return nullptr;
    }

    cairo_destroy(cr);
    g_object_unref(hnd);
    return surf;
}

// ---------- recolor (blend) ----------
static cairo_operator_t op_from_name(const std::string& name){
    static const std::unordered_map<std::string,cairo_operator_t> lut = {
        {"multiply",    CAIRO_OPERATOR_MULTIPLY},
        {"overlay",     CAIRO_OPERATOR_OVERLAY},
        {"soft-light",  CAIRO_OPERATOR_SOFT_LIGHT},
        {"hsl-color",   CAIRO_OPERATOR_HSL_COLOR},
        {"screen",      CAIRO_OPERATOR_SCREEN},
        {"darken",      CAIRO_OPERATOR_DARKEN},
        {"lighten",     CAIRO_OPERATOR_LIGHTEN}
    };
    auto it = lut.find(name);
    return it==lut.end() ? CAIRO_OPERATOR_MULTIPLY : it->second;
}

static cairo_surface_t* tint_surface(const cairo_surface_t* src, double r,double g,double b,double a,
                                     cairo_operator_t op){
    int W = cairo_image_surface_get_width(const_cast<cairo_surface_t*>(src));
    int H = cairo_image_surface_get_height(const_cast<cairo_surface_t*>(src));
    cairo_surface_t* dst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, W, H);
    cairo_t* cr = cairo_create(dst);

    // draw original
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface(cr, const_cast<cairo_surface_t*>(src), 0, 0);
    cairo_paint(cr);

    // tint via blend, masked by source alpha (no spill)
    cairo_set_operator(cr, op);
    cairo_set_source_rgba(cr, r,g,b,a);
    cairo_mask_surface(cr, const_cast<cairo_surface_t*>(src), 0, 0);

    cairo_destroy(cr);
    return dst;
}

// ---------- CLI ----------
static void usage(const char* prog){
    std::cerr <<
      "Usage:\n"
      "  " << prog << " <input.(png|svg)> <output.png> <#RRGGBB[AA]> [mode] [width height]\n"
      "Defaults: mode=multiply; width/height only apply to SVG.\n"
      "Examples:\n"
      "  " << prog << " logo.svg out.png #6A5ACD overlay 1024 512\n"
      "  " << prog << " icon.png out.png #00FF88 soft-light\n";
}

int main(int argc, char** argv){
    if (argc < 4){ usage(argv[0]); return 1; }

    std::string in  = argv[1];
    std::string out = argv[2];
    std::string hex = argv[3];
    std::string mode = (argc>=5 ? argv[4] : "multiply");

    int w = -1, h = -1;
    if (argc>=7){ w = std::max(1, std::atoi(argv[5])); h = std::max(1, std::atoi(argv[6])); }

    double r,g,b,a;
    if (!parse_hex_rgba(hex, r,g,b,a)){
        std::cerr << "Invalid color. Use #RRGGBB or #RRGGBBAA\n";
        return 1;
    }

    // Load to surface
    cairo_surface_t* src = nullptr;
    if (ends_with_ci(in, ".png")){
        src = load_png(in);
    } else if (ends_with_ci(in, ".svg")){
        src = render_svg(in, w, h);
    } else {
        std::cerr << "Unsupported input (use .png or .svg)\n";
        return 1;
    }

    if (!src){
        std::cerr << "Failed to load/render input.\n";
        return 1;
    }

    // Recolor
    cairo_operator_t op = op_from_name(mode);
    cairo_surface_t* tinted = tint_surface(src, r,g,b,a, op);

    // Save atomically (great for OBS hot-swap)
    bool ok = atomic_write_png(tinted, out);

    cairo_surface_destroy(tinted);
    cairo_surface_destroy(src);

    if (!ok){
        std::cerr << "Failed to write output.\n";
        return 1;
    }
    std::cout << "Wrote " << out << "\n";
    return 0;
}