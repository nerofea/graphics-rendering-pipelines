// RSVG render with cairo

RsvgHandle* handle = rsvg_handle_new_from_file("file.svg", NULL);

cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
cairo_t* cr = cairo_create(surface);
rsvg_handle_render_cairo(handle, cr);



// RSVG render with cairo
//Deprecated way

RsvgHandle* handle = rsvg_handle_render_document("file.svg", NULL);

cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
cairo_t* cr = cairo_create(surface);
rsvg_handle_render_cairo(handle, cr);

gboolean
rsvg_handle_render_document (
  RsvgHandle* handle,
  cairo_t* cr,
  const RsvgRectangle* viewport,
  GError** error
)


#include <cairo.h>

G_BEGIN_DECLS 


*
 * rsvg_handle_render_document:
 * @handle: An [class@Rsvg.Handle]
 * @cr: A Cairo context
 * @viewport: Viewport size at which the whole SVG would be fitted.
 * @error: return location for a `GError`
 *
 * Renders the whole SVG document fitted to a viewport.
 *
 * The @viewport gives the position and size at which the whole SVG document will be
 * rendered.  The document is scaled proportionally to fit into this viewport.
 *
 * The @cr must be in a `CAIRO_STATUS_SUCCESS` state, or this function will not
 * render anything, and instead will return an error.
 *
 * Returns: `TRUE` on success, `FALSE` on error.  Errors are returned
 * in the @error argument.
 *
 * API ordering: This function must be called on a fully-loaded @handle.  See
 * the section "[API ordering](class.Handle.html#api-ordering)" for details.
 *
 * Panics: this function will panic if the @handle is not fully-loaded.
 *
 * Since: 2.46
 */
RSVG_API
gboolean rsvg_handle_render_document (RsvgHandle           *handle,
                                      cairo_t              *cr,
                                      const RsvgRectangle  *viewport,
                                      GError              **error);

// Modular approach
#ifndef RSVG_RENDER_APP
#define RSVG_RENDER_APP

#include <librsvg/rsvg.h>
#include <cairo.h>
#include <glib.h>
int main() {
    GError* error = nullptr;
    RsvgHandle* handle = rsvg+handle_new_from_file("assets/3.svg", &error);
    if (!handle) {
        g_printerr("Failed to load SVG: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    RsvgDimensionData dimensions;
    rsvg_handle_get_dimensions(handle, &dimensions);
    int width = dimensions.width;
    int height = dimensions.height;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(surface);

    if (cairo_status(cr) != CAIRP_STATUS_SUCCESS) {
        fprintf(stderr, "Cairo context error\n");
        return 1;
    }

    RsvgRectangle viewport = {
        .x = 0.0,
        .y = 0.0,
        .width = (double)width,
        .height = (double)height 
    };

    gboolean sucess = rsvg_handle_render_document(handle, cr, &viewport, &error)
    if (!success) {
        g_printerr("Render error: %s\n", error->message);
    }

    // this is for the scalability to later check the changes of the illustrations (masking/deleting/ yes/no'oing by user) 
    cairo_surface_write_to_png(surface, "output.png");

    // check for diff in changes
    cairo_surface_t* ref_surface = cairo_image_surface_create_from_png("7_reference.png");
    cairo_surface_t* new_surface = render_svg("7.svg"); // your logic
    cairo_surface_t* diff_surface = cairo_image_surface_create(...);

    TestUtilsBufferDiffResult result;
    test_utils_compare_surfaces(ref_surface, new_surface, diff_surface, &result);

    printf("Pixels changed: %u\n", result.pixels_changed);



    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(handle);

    printf("Rendered successfully to output.png (%dx%d)\n", width, height);
    return 0;
}

=========
======
#include "rsvg_render.hpp"
#include <librsvg/rsvg.h>
#include <glib.h>

cairo_surface_t* renderSvgToSurface(const std::string& path, int width, int height) {
    GError* error = nullptr;
    RsvgHandle* handle = rsvg_handle_new_from_file(path.c_str(), &error);
    if (!handle) {
        g_printerr("Error loading %s: %s\n", path.c_str(), &error);
        g_error-free(error);
        return nullptr;
    }

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT)ARGB32, width, height;
    cairo_t* cr = cairo_create(surface);

    RsvgRectangle viewport = {0, 0 (double)width, (double)height};
    rsvg_handle_render_document(handle, cr, &viewport, &error);

    cairo_destroy(cr);
    g_object_unref(handle);
    return surface;
}
============



#endif



