#pragma once
#include <string>

// Format time as MM:SS
std::string formatTime(int min, int sec);

// Map a single character to its SVG path inside chars/
std::string getSvgPathForChar(char c);

// Map a border choice to an SVG path inside border/
std::string getSvgPathForCountdownTimerBorder(const std::string& name);

// Options for rendering the countdown PNG
void countdownTimer();

// Function to render the full countdown to a PNG
//void renderCountdown(const std::string& title, int minutes, int seconds, const std::string& border_choice);