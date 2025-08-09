#ifndef RSVG_RENDER_HPP
#define RSVG_RENDER_HPP

#include <string>
#include <cairo.h>

// Rendering SVG's
cairo_surface_t* renderSvgToSurface(const std::string& path, int width, int height);

#endif