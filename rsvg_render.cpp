#include "rsvg_render.hpp"
#include <cairo.h>           // explicit, even though header already has it
#include <librsvg/rsvg.h>
#include <glib.h>

cairo_surface_t* renderSvgToSurface(const std::string& path, int width, int height) {
    GError* error = nullptr;

    RsvgHandle* handle = rsvg_handle_new_from_file(path.c_str(), &error);
    if (!handle) {
        if (error) {
            g_printerr("Error loading %s: %s\n", path.c_str(), error->message);
            g_error_free(error);
        } else {
            g_printerr("Unknown error loading %s\n", path.c_str());
        }
        return nullptr;
    }

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(surface);

    RsvgRectangle vp { 0.0, 0.0, (double)width, (double)height };
    if (!rsvg_handle_render_document(handle, cr, &vp, &error)) {
        if (error) {
            g_printerr("Render error for %s: %s\n", path.c_str(), error->message);
            g_error_free(error);
        } else {
            g_printerr("Unknown render error for %s\n", path.c_str());
        }
    }

    cairo_destroy(cr);
    g_object_unref(handle);
    return surface;
}
