#pragma once
#include <string>
#include <cairo/cairo.h>

// Detect file extension and load into a Cairo surface.
// Returns nullptr if unsupported or load fails.
// If SVG and width/height > 0, resizes output.
cairo_surface_t* load_image_or_svg(const std::string& path, int width = -1, int height = -1);
