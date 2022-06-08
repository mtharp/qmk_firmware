#pragma once

// Target frequency at which the whole matrix is scanned
#define BEAM_SCAN_RATE 1000

// Ticks to spend before moving to the next column
#define BEAM_COL_PERIOD (CAPTURE_FREQUENCY / BEAM_SCAN_RATE / MATRIX_COLS)

// Ticks before unselecting the column and checking the result
#define BEAM_COL_WINDOW (BEAM_COL_PERIOD / 2)

#define BEAM_COL_CONFIG PAL_STM32_OTYPE_PUSHPULL

// Ticks below this threshold are low-capacitance (pressed), above is
// high-capacitance (unpressed). This is expressed as a fraction of the timer
// frequency since it's not dependent on how much time we linger on each column.
#define BEAM_THRESHOLD (CAPTURE_FREQUENCY / 10000 * 3 / 32)

// Capacitance increases when key is pressed
#define BEAM_ACTIVE_HIGH

// Key matrix positions corresponding to unused or calibration pads. QMK seems
// to get confused if a key is pressed on startup, even if it is not mapped to a
// function.
#define BEAM_DEAD_KEYS \
    { 0x0, 0xb03, 0x200, 0x83, 0x100, 0x0, 0x0, 0x881 }

// Calibration pads, which read as unpressed
#define BEAM_CAL_KEYS \
    { 0, 0, 0, 0, 0, 0, 0, 0 }
