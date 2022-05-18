#pragma once

// Target frequency at which the whole matrix is scanned
#define BEAM_SCAN_RATE 500

// Ticks to spend before moving to the next column
#define BEAM_COL_PERIOD (CAPTURE_FREQUENCY / BEAM_SCAN_RATE / MATRIX_COLS)

// Ticks before unselecting the column and checking the result
#define BEAM_COL_WINDOW (BEAM_COL_PERIOD / 2)

// Ticks below this threshold are low-capacitance (pressed), above is
// high-capacitance (unpressed). This is expressed as a fraction of the timer
// frequency since it's not dependent on how much time we linger on each column.
#define BEAM_THRESHOLD (CAPTURE_FREQUENCY / 10000 * 3 / 32)
