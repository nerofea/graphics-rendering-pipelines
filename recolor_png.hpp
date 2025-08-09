#pragma once
#include <cairo/cairo.h>

bool parse_hex_rgba(const std::string& hex, double& r,double& g,double& b,double& a);

void tint_png_multiply(const char* in_png, const char* out_png,
                              double r, double g, double b, double a = 1.0);

void recolor_png_with_alpha_mask(const char* in_png, const char* out_png,
                                        double r, double g, double b, double a = 1.0);

void rgb_to_hsv(double R, double G, double B, double& h, double& s, double& v);

void hsv_to_rgb(double h, double s, double v, double& R, double& G, double& B);

void hue_shift_png(const char* in_png, const char* out_png, double hue_delta_deg);

main(); 