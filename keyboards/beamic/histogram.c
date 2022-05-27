// Copyright 2022 Michael Tharp (@mtharp)
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>
#include "quantum.h"
#include "beam_config.h"

#define HIST_BINS 24
// This is log2(max_value/HIST_BINS) . A good max_value is 2*BEAM_THRESHOLD
// e.g. max_value = 2 * 168MHz fclk / 500Hz scan / 20 cols / 2 window * 3 / 16 threshold = 3150
#define HIST_SHIFT 7

bool value_histogram(uint16_t values[][MATRIX_ROWS]) {
#ifdef HIST_BINS
    static systime_t last_report = 0;
    static uint8_t   val_bins[HIST_BINS];
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
        for (uint8_t row = 0; row < 1; row++) {
            uint8_t hist_val = values[col][row] >> HIST_SHIFT;
            if (hist_val >= HIST_BINS) {
                hist_val = HIST_BINS - 1;
            }
            val_bins[hist_val]++;
        }
    }

    if (chVTTimeElapsedSinceX(last_report) < TIME_MS2I(250)) {
        return false;
    }
    last_report = chVTGetSystemTimeX();

    uint8_t thresh_bin = BEAM_THRESHOLD >> HIST_SHIFT;
    xprintf("[");
    for (uint8_t i = 0; i < HIST_BINS; i++) {
        uint8_t count = val_bins[i];
        if (i == thresh_bin) {
            if (count == 0) {
                xprintf("|");
            } else {
                xprintf("X");
            }
        } else if (count == 0) {
            xprintf(" ");
        } else if (count < 20) {
            xprintf(".");
        } else if (count < 200) {
            xprintf("o");
        } else {
            xprintf("O");
        }
    }
    xprintf("]");
    for (uint8_t i = 0; i < HIST_BINS; i++) {
        val_bins[i] = 0;
    }
    return true;
#else
    return false;
#endif
}
