// Copyright 2022 Michael Tharp (@mtharp)
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>
#include "gpio.h"
#include "matrix.h"
#include "quantum.h"
#include "histogram.h"

static pin_t col_pins[MATRIX_COLS] = MATRIX_COL_PINS;
static pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;

static matrix_row_t dead_keys[MATRIX_ROWS] = {0x20004, 0x20002, 0x80043, 0x64002};

static uint16_t beam_values[MATRIX_COLS][MATRIX_ROWS];

static event_source_t   beam_source;
static event_listener_t beam_listener;

static void timcap_init(TIM_TypeDef *tim) {
    // Reset the count when triggered by TIM1 cycling
    tim->SMCR = TIM_SMCR_SMS_2;
    // Capture rising edges
    tim->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;
    tim->CCMR2 = TIM_CCMR2_CC3S_0 | TIM_CCMR2_CC4S_0;
    tim->CCER  = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    // Enable. TIM1 will reset to sync everything up.
    tim->CR1 = TIM_CR1_URS | TIM_CR1_CEN;
}

void matrix_init_custom(void) {
    // Drive columns in open drain mode
    for (uint8_t x = 0; x < MATRIX_COLS; x++) {
        palSetLineMode(col_pins[x], PAL_STM32_MODE_OUTPUT | PAL_STM32_OTYPE_OPENDRAIN);
    }
    // Assign row pins to timer input capture
    for (uint8_t x = 0; x < MATRIX_ROWS; x++) {
        palSetLineMode(row_pins[x], PAL_MODE_ALTERNATE(TIM345_ALT_FUNC) | PAL_STM32_PUPDR_PULLUP);
    }

    // Init beam_values with unpressed state
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            beam_values[col][row] = 65535;
        }
    }
    chEvtObjectInit(&beam_source);
    chEvtRegisterMask(&beam_source, &beam_listener, EVENT_MASK(0));

    // TIM3 and TIM5 capture the time at which each row discharges back above
    // the comparator threshold. They are reset to 0 each time TIM1 rolls over.
    rccEnableTIM3(true);
    rccResetTIM3();
    timcap_init(TIM3);
#if MATRIX_ROWS == 8
    rccEnableTIM5(true);
    rccResetTIM5();
    timcap_init(TIM5);
#endif

    // TIM1 runs continuously and dictates the pace of scanning the columns. It
    // fires two interrupts: the first (set by CCR1) unselects the current
    // column and processes the captured result, and the second (set by ARR)
    // selects the next column.
    rccEnableTIM1(true);
    rccResetTIM1();
    nvicEnableVector(STM32_TIM1_UP_NUMBER, STM32_IRQ_TIM1_UP_PRIORITY);
    nvicEnableVector(STM32_TIM1_CC_NUMBER, STM32_IRQ_TIM1_CC_PRIORITY);

    TIM1->ARR  = BEAM_COL_PERIOD - 1;
    TIM1->CR2  = TIM_CR2_MMS_1;                 // TRGO on Update Event
    TIM1->CCR1 = BEAM_COL_WINDOW;               // set when OC triggers
    TIM1->CCER = TIM_CCER_CC1E;                 // enable output compare
    TIM1->DIER = TIM_DIER_CC1IE | TIM_DIER_UIE; // OC and update irqs
    TIM1->EGR  = TIM_EGR_UG;                    // generate a reset
    TIM1->CR1  = TIM_CR1_URS | TIM_CR1_CEN;     // start timer

    matrix_init_user();
}

static void read_input_captures(uint16_t *col, pin_t *row_pins, TIM_TypeDef *timer) {
    uint32_t sr = timer->SR;
    if (sr & TIM_SR_CC1IF) {
        col[0] = timer->CCR1;
    } else {
        // If no capture occurred and the line is still high then it never fell,
        // treat this as 0. If the line is still low, treat this as a very long
        // duration.
        // TODO: if a rising edge happens during this interrupt, this will race
        col[0] = readPin(row_pins[0]) ? 0 : 65535;
    }
    if (sr & TIM_SR_CC2IF) {
        col[1] = timer->CCR2;
    } else {
        col[1] = readPin(row_pins[1]) ? 0 : 65535;
    }
    if (sr & TIM_SR_CC3IF) {
        col[2] = timer->CCR3;
    } else {
        col[2] = readPin(row_pins[2]) ? 0 : 65535;
    }
    if (sr & TIM_SR_CC4IF) {
        col[3] = timer->CCR4;
    } else {
        col[3] = readPin(row_pins[3]) ? 0 : 65535;
    }
}

static void handle_tim1(void) {
    static uint8_t current_column = MATRIX_COLS - 1; // initial update rolls this over to 0
    // read and clear status registers
    uint32_t sr = TIM1->SR;
    TIM1->SR    = ~sr;

    if (sr & TIM_SR_CC1IF) {
        // deselect previous column
        writePinHigh(col_pins[current_column]);
        // read raw captured values
        read_input_captures(&beam_values[current_column][0], &row_pins[0], TIM3);
#if MATRIX_ROWS == 8
        read_input_captures(&beam_values[current_column][4], &row_pins[4], TIM5);
#endif
        if (current_column == MATRIX_COLS - 1) {
            // wake up scan func
            chSysLockFromISR();
            chEvtBroadcastI(&beam_source);
            chSysUnlockFromISR();
        }
    }

    if (sr & TIM_SR_UIF) {
        // select next column
        if (++current_column >= MATRIX_COLS) {
            current_column = 0;
        }
        writePinLow(col_pins[current_column]);
        // drain stale values from input captures
        (void)TIM3->CCR1;
        (void)TIM3->CCR2;
        (void)TIM3->CCR3;
        (void)TIM3->CCR4;
#if MATRIX_ROWS == 8
        (void)TIM5->CCR1;
        (void)TIM5->CCR2;
        (void)TIM5->CCR3;
        (void)TIM5->CCR4;
#endif
    }
}

OSAL_IRQ_HANDLER(STM32_TIM1_UP_HANDLER) {
    OSAL_IRQ_PROLOGUE();
    handle_tim1();
    OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(STM32_TIM1_CC_HANDLER) {
    OSAL_IRQ_PROLOGUE();
    handle_tim1();
    OSAL_IRQ_EPILOGUE();
}

#if BEAM_THRESHOLD > BEAM_COL_WINDOW / 2
#    warning Threshold for key-presses is too large to measure in the alotted window. Reduce BEAM_SCAN_RATE.
#endif

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    // wait for interrupts to scan all the raw values
    if (!chEvtWaitAnyTimeout(ALL_EVENTS, TIME_MS2I(100))) {
        return false;
    }
    // convert raw readings to pressed/unpressed state
    bool changed = false;
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_row_t new_row  = 0;
        matrix_row_t col_mask = 1;
        for (uint8_t col = 0; col < MATRIX_COLS; col++, col_mask <<= 1) {
            uint16_t val = beam_values[col][row];
            if (val < BEAM_THRESHOLD) {
                new_row |= col_mask;
            }
        }
        new_row &= ~dead_keys[row];
        if (new_row != current_matrix[row]) {
            changed             = true;
            current_matrix[row] = new_row;
        }
    }
    value_histogram(&beam_values[0][0]);
    return changed;
}
