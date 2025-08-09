#include "file_type_check.hpp"
#include <librsvg/rsvg.h>
#include <glib.h>
#include <cstring>
#include <iostream>

// case-insensitive ends_with
static bool ends_with_ci(const std::string& s, const char* suf){
    size_t n = std::strlen(suf);
    if (s.size() < n) return false;
    for (size_t i=0;i<n;++i){
        char a = s[s.size()-n+i], b = suf[i];
        if (a>='A' && a<='Z') a += 'a'-'A';
        if (b>='A' && b<='Z') b += 'a'-'A';
        if (a != b) return false;
    }
    return true;
}

static cairo_surface_t* load_png(const std::string& path){
    cairo_surface_t* s = cairo_image_surface_create_from_png(path.c_str());
    if (cairo_surface_status(s) != CAIRO_STATUS_SUCCESS){
        std::cerr << "Failed to load PNG: " << path << "\n";
        cairo_surface_destroy(s);
        return nullptr;
    }
    return s;
}

static cairo_surface_t* render_svg(const std::string& path, int w, int h){
    GError* err=nullptr;
    RsvgHandle* hnd = rsvg_handle_new_from_file(path.c_str(), &err);
    if (!hnd){
        if (err){ std::cerr << err->message << "\n"; g_error_free(err); }
        return nullptr;
    }

    // If no size provided, use intrinsic SVG size
    if (w<=0 || h<=0){
        RsvgDimensionData dim{};
        rsvg_handle_get_dimensions(hnd, &dim);
        if (w <= 0) w = dim.width > 0 ? dim.width : 800;
        if (h <= 0) h = dim.height > 0 ? dim.height : 600;
    }

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    cairo_t* cr = cairo_create(surf);

    RsvgRectangle vp{0.0, 0.0, (double)w, (double)h};
    if (!rsvg_handle_render_document(hnd, cr, &vp, &err)){
        if (err){ std::cerr << err->message << "\n"; g_error_free(err); }
        cairo_destroy(cr);
        cairo_surface_destroy(surf);
        g_object_unref(hnd);
        return nullptr;
    }

    cairo_destroy(cr);
    g_object_unref(hnd);
    return surf;
}

cairo_surface_t* load_image_or_svg(const std::string& path, int width, int height){
    if (ends_with_ci(path, ".png")){
        return load_png(path);
    } else if (ends_with_ci(path, ".svg")){
        return render_svg(path, width, height);
    } else {
        std::cerr << "Unsupported file type: " << path << "\n";
        return nullptr;
    }
}



// ---------- small helpers ----------
static bool ends_with(const std::string& s, const char* suf){
    size_t n = std::strlen(suf);
    return s.size() >= n && strncasecmp(s.c_str()+s.size()-n, suf, n) == 0;
}

static bool atomic_write_png(cairo_surface_t* surf, const std::string& outpath){
    std::string tmp = outpath + ".tmp";
    if (cairo_surface_write_to_png(surf, tmp.c_str()) != CAIRO_STATUS_SUCCESS) return false;
    std::remove(outpath.c_str()); // OK if it doesn't exist
    return std::rename(tmp.c_str(), outpath.c_str()) == 0;
}

