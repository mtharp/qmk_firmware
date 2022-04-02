#include <string.h>
#include <avr/io.h>
#include "gpio.h"
#include "matrix.h"
#include "sn74x138.h"
#include "quantum.h"

static pin_t col_pins[MATRIX_COLS]   = MATRIX_COL_PINS;

void
matrix_init_custom(void) {
    ATOMIC_BLOCK_FORCEON {
        for (uint8_t x = 0; x < MATRIX_COLS; x++) {
            setPinInputHigh(col_pins[x]);
        }
        sn74x138_init();
    }
}

static void
select_row(uint8_t row) {
    // numbering of rows on the PCB is backwards from how info.json has it
    sn74x138_set_addr(7-row);
}

static void
matrix_read_cols_on_row(matrix_row_t current_matrix[], uint8_t current_row) {
    // Start with a clear matrix row
    matrix_row_t current_row_value = 0;

    select_row(current_row);
    matrix_output_select_delay();

    // For each col...
    matrix_row_t row_shifter = MATRIX_ROW_SHIFTER;
    for (uint8_t col_index = 0; col_index < MATRIX_COLS; col_index++, row_shifter <<= 1) {
        uint8_t pin_state = readPin(col_pins[col_index]);

        // Populate the matrix row with the state of the col pin
        current_row_value |= pin_state ? 0 : row_shifter;
    }

    // Update the matrix
    current_matrix[current_row] = current_row_value;
}

bool
matrix_scan_custom(matrix_row_t current_matrix[]) {
    matrix_row_t new_matrix[MATRIX_ROWS] = {0};
    // Set row, read cols
    for (uint8_t current_row = 0; current_row < MATRIX_ROWS; current_row++) {
        matrix_read_cols_on_row(new_matrix, current_row);
    }

    bool changed = memcmp(current_matrix, new_matrix, sizeof(new_matrix)) != 0;
    if (changed) memcpy(current_matrix, new_matrix, sizeof(new_matrix));

    return changed;
}
