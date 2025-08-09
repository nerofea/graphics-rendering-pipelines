#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <limits>
#include <cairo.h>
#include "countdown_timer.hpp"
#include "rsvg_render.hpp"

// Format time as MM:SS
static std::string formatTime(int min, int sec) {
    std::ostringstream oss;
    if (min < 10) oss << '0';
    oss << min << ':';
    if (sec < 10) oss << '0';
    oss << sec;
    return oss.str();
}

// Map a single character to its SVG path inside chars/
static std::string getSvgPathForChar(char c) {
    if (c == ':') return "chars/colon.svg";
    return std::string("chars/") + c + ".svg";
}

// Map a border choice to an SVG path inside border/
static std::string getSvgPathForCountdownTimerBorder(const std::string& name) {
    return std::string("border/") + name + ".svg";
}

void countdownTimer() {
    // ---- Input ----
    int minutes = 0, seconds = 0;
    std::string title;
    std::string border_choice;

    std::cout << "Enter title: ";
    std::getline(std::cin, title);

    std::cout << "Enter minutes: ";
    std::cin >> minutes;

    std::cout << "Enter seconds: ";
    std::cin >> seconds;

    // clear leftover newline before next getline
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Enter border color (e.g. blue): ";
    std::getline(std::cin, border_choice);

    // ---- Build strings ----
    const std::string time_string = formatTime(minutes, seconds);

    // ---- Sizes/Layout (tweak as you like) ----
    const int border_margin = 20;
    const int spacing       = 10;

    const int digit_width   = 100;
    const int digit_height  = 150;

    const int char_width    = 40;   // title character size
    const int char_height   = 60;
    const int title_y       = border_margin;
    const int digits_y      = border_margin + char_height + 20;

    // ---- Render title characters to surfaces ----
    std::vector<cairo_surface_t*> title_surfaces;
    title_surfaces.reserve(title.size());
    for (char c : title) {
        auto s = renderSvgToSurface(getSvgPathForChar(c), char_width, char_height);
        title_surfaces.push_back(s);
    }

    // ---- Render digits to surfaces ----
    std::vector<cairo_surface_t*> digit_surfaces;
    digit_surfaces.reserve(time_string.size());
    for (char c : time_string) {
        auto s = renderSvgToSurface(getSvgPathForChar(c), digit_width, digit_height);
        digit_surfaces.push_back(s);
    }

    // ---- Border surface (scaled big enough to cover everything) ----
    // We'll compute canvas size first, then render border the same size.
    const int title_row_width =
        (int)title_surfaces.size() * char_width + (int)std::max<size_t>(0, title_surfaces.size() ? title_surfaces.size()-1 : 0) * spacing;
    const int digits_row_width =
        (int)digit_surfaces.size() * digit_width + (int)std::max<size_t>(0, digit_surfaces.size() ? digit_surfaces.size()-1 : 0) * spacing;

    const int content_width  = std::max(title_row_width, digits_row_width);
    const int canvas_width   = content_width + 2 * border_margin;
    const int canvas_height  = digits_y + digit_height + border_margin;

    cairo_surface_t* border_surface =
        renderSvgToSurface(getSvgPathForCountdownTimerBorder(border_choice), canvas_width, canvas_height);

    // ---- Create final canvas ----
    cairo_surface_t* canvas = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, canvas_width, canvas_height);
    cairo_t* cr = cairo_create(canvas);

    // Draw border background first
    if (border_surface) {
        cairo_set_source_surface(cr, border_surface, 0, 0);
        cairo_paint(cr);
    }

    // Draw title characters centered horizontally
    int title_total_width = title_row_width;
    int title_x = (canvas_width - title_total_width) / 2;
    for (auto* s : title_surfaces) {
        cairo_set_source_surface(cr, s, title_x, title_y);
        cairo_paint(cr);
        title_x += char_width + spacing;
    }

    // Draw digits centered horizontally
    int digits_total_width = digits_row_width;
    int digits_x = (canvas_width - digits_total_width) / 2;
    for (auto* s : digit_surfaces) {
        cairo_set_source_surface(cr, s, digits_x, digits_y);
        cairo_paint(cr);
        digits_x += digit_width + spacing;
    }

    // ---- Save and cleanup ----
    cairo_surface_write_to_png(canvas, "countdown_output.png");

    cairo_destroy(cr);
    cairo_surface_destroy(canvas);
    if (border_surface) cairo_surface_destroy(border_surface);
    for (auto* s : title_surfaces) cairo_surface_destroy(s);
    for (auto* s : digit_surfaces) cairo_surface_destroy(s);

    std::cout << "Wrote countdown_output.png (" << canvas_width << "x" << canvas_height << ")\n";
    return 0;
}
