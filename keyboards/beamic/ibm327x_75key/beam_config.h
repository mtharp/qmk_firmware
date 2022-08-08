#pragma once

// Target frequency at which the whole matrix is scanned
#define BEAM_SCAN_RATE 500

// Ticks to spend before moving to the next column
#define BEAM_COL_PERIOD (STM32_TIMCLK1 / BEAM_SCAN_RATE / MATRIX_COLS)

// Ticks before unselecting the column and checking the result
#define BEAM_COL_WINDOW (BEAM_COL_PERIOD / 2)

// Ticks below this threshold are low-capacitance (pressed), above is
// high-capacitance (unpressed). This is expressed as a fraction of the timer
// frequency since it's not dependent on how much time we linger on each column.
#define BEAM_THRESHOLD (STM32_TIMCLK1 / 10000 * 8 / 100)

// Capacitance drops when a key is pressed
#define BEAM_ACTIVE_LOW

// Initial DAC value for the reference voltage
#define BEAM_DAC_INITIAL 4040

// Map of which column positions are actually used. Note that the bits here
// visually match the order on the connector -- the leftmost bit (MSB)
// corresponds to column 0 (on the left)
#define BEAM_COLUMNS 0b000111111111110111111111

// Key matrix positions corresponding to unused or calibration pads. QMK seems
// to get confused if a key is pressed on startup, even if it is not mapped to a
// function.
#define BEAM_DEAD_KEYS \
    { 0x20004, 0x20000, 0, 0x10008 }

// Calibration pads, which read as unpressed
#define BEAM_CAL_KEYS \
    { 0, 0, 0, 0x10008 }
