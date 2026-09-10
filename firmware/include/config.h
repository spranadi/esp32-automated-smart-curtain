#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- PIN DEFINITIONS ---
constexpr int PIN_STEP   = 18;  // A4988 STEP pin
constexpr int PIN_DIR    = 19;  // A4988 DIR pin
constexpr int PIN_ENABLE = 21;  // A4988 EN pin (Active LOW)
constexpr int PIN_BUTTON = 32;  // Manual toggle trigger to GND

// --- MOTION BOUNDARIES (28.5 inches horizontal travel) ---
constexpr long POSITION_CLOSED = 0; // Fully closed position (0 steps)
constexpr long POSITION_OPEN   = 3620; // Fully open position (3620 steps, ~28.5 inches)

// --- HIGH-TORQUE TIMING / SPEED PROFILE ---
constexpr int STEP_MIN_DELAY_US = 5000;   // Cruising speed delay (~40s travel)
constexpr int STEP_MAX_DELAY_US = 10000;  // High-torque start delay
constexpr int STEP_RAMP_STEPS   = 400;    // Steps spent accelerating/decelerating

// --- DEBOUNCE TRACKING ---
constexpr unsigned long BUTTON_DEBOUNCE_MS = 80;

// --- TIME SYNCHRONIZATION ---
constexpr const char* NTP_SERVER = "pool.ntp.org"; // sync time from NTP server
constexpr long GMT_OFFSET_SEC    = -28800;  // PST Offset (-8 hours)
constexpr int DAYLIGHT_OFFSET_SEC = 3600;   // Daylight Savings (+1 hour)

#endif // CONFIG_H