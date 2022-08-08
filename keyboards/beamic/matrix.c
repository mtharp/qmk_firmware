// Copyright 2022 Michael Tharp (@mtharp)
// SPDX-License-Identifier: GPL-2.0-or-later

#include <string.h>
#include "gpio.h"
#include "matrix.h"
#include "quantum.h"
#include "beam_config.h"
#include "histogram.h"

static pin_t row_pins[MATRIX_ROWS] = MATRIX_ROW_PINS;

static matrix_row_t dead_keys[MATRIX_ROWS] = BEAM_DEAD_KEYS;
static matrix_row_t cal_keys[MATRIX_ROWS]  = BEAM_CAL_KEYS;

static uint16_t beam_values[MATRIX_COLS][MATRIX_ROWS];

static event_source_t   beam_source;
static event_listener_t beam_listener;

static void timcap_init(TIM_TypeDef *tim) {
    // Reset the count when triggered by TIM2 cycling
    tim->SMCR |= TIM_SMCR_SMS_2;
    // Capture rising edges
    tim->CCMR1 = TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;
    tim->CCMR2 = TIM_CCMR2_CC3S_0 | TIM_CCMR2_CC4S_0;
    tim->CCER  = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    // Enable. TIM2 will reset to sync everything up.
    tim->CR1 = TIM_CR1_URS | TIM_CR1_CEN;
}

static void select_column(uint32_t col_bits) {
    // drive active column low and others high
    col_bits = ~col_bits;
    chSysLockFromISR();
    if (SREG_DRIVER.state == SPI_READY) {
        spiStartSendI(&SREG_DRIVER, 3, &col_bits);
    }
    chSysUnlockFromISR();
}

static void deselect_columns(void) {
    // drive everything high
    static uint32_t all_ones = 0xffffffff;
    chSysLockFromISR();
    if (SREG_DRIVER.state == SPI_READY) {
        spiStartSendI(&SREG_DRIVER, 3, &all_ones);
    }
    chSysUnlockFromISR();
}

static void latch_column(SPIDriver *d) {
    writePinHigh(LINE_STCP);
    writePinLow(LINE_STCP);
}

void matrix_init_custom(void) {
    // configure SPI to drive shift registers
    static const SPIConfig spicfg = {
        .data_cb = latch_column,
        .cr1     = SPI_CR1_LSBFIRST,
        .cr2     = 0,
    };
    spiStart(&SREG_DRIVER, &spicfg);
    // configure DAC
    static const DACConfig daccfg = {
        .init     = BEAM_DAC_INITIAL,
        .datamode = DAC_DHRM_12BIT_RIGHT,
        .cr       = DAC_CR_BOFF1,
    };
    dacStart(&VREF_DRIVER, &daccfg);
    // Init beam_values with unpressed state
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            beam_values[col][row] = 65535;
        }
        // Ensure cal keys are masked out
        dead_keys[row] |= cal_keys[row];
    }
    chEvtObjectInit(&beam_source);
    chEvtRegisterMask(&beam_source, &beam_listener, EVENT_MASK(0));

    // TIM3 and TIM4 capture the time at which each row discharges back above
    // the comparator threshold. They are reset to 0 each time TIM2 rolls over.
    rccEnableTIM3(true);
    rccResetTIM3();
    TIM3->SMCR = 1 << TIM_SMCR_TS_Pos; // triggered by TIM2
    timcap_init(TIM3);
#if MATRIX_ROWS == 8
    rccEnableTIM4(true);
    rccResetTIM4();
    TIM4->SMCR = 1 << TIM_SMCR_TS_Pos; // triggered by TIM2
    timcap_init(TIM4);
#endif

    // TIM2 runs continuously and dictates the pace of scanning the columns. It
    // fires two interrupts: the first (set by CCR4) unselects the current
    // column and processes the captured result, and the second (set by ARR)
    // selects the next column.
    rccEnableTIM2(true);
    rccResetTIM2();
    nvicEnableVector(STM32_TIM2_NUMBER, STM32_IRQ_TIM2_PRIORITY);

    TIM2->ARR  = BEAM_COL_PERIOD - 1;
    TIM2->CR2  = TIM_CR2_MMS_1;                 // TRGO on Update Event
    TIM2->CCR4 = BEAM_COL_WINDOW;               // set when OC triggers
    TIM2->CCER = TIM_CCER_CC4E;                 // enable output compare
    TIM2->DIER = TIM_DIER_CC4IE | TIM_DIER_UIE; // OC and update irqs
    TIM2->EGR  = TIM_EGR_UG;                    // generate a reset
    TIM2->CR1  = TIM_CR1_URS | TIM_CR1_CEN;     // start timer
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

OSAL_IRQ_HANDLER(STM32_TIM2_HANDLER) {
    OSAL_IRQ_PROLOGUE();
    static uint32_t logical_column = MATRIX_COLS - 1; // initial update rolls this over to 0
    static uint32_t col_mask       = 1 << 31;
    // read and clear status registers
    uint32_t sr = TIM2->SR;
    TIM2->SR    = ~sr;

    if (sr & TIM_SR_CC4IF) {
        // deselect current column
        deselect_columns();
        // read raw captured values
        read_input_captures(&beam_values[logical_column][0], &row_pins[0], TIM3);
#if MATRIX_ROWS == 8
        read_input_captures(&beam_values[logical_column][4], &row_pins[4], TIM4);
#endif
        if (logical_column == MATRIX_COLS - 1) {
            // wake up scan func
            chSysLockFromISR();
            chEvtBroadcastI(&beam_source);
            chSysUnlockFromISR();
        }
    }

    if (sr & TIM_SR_UIF) {
        // select next column
        if (++logical_column >= MATRIX_COLS) {
            logical_column = 0;
            // col0 corresponds to the MSB of the shift registers
            col_mask = 1 << 23;
        } else {
            col_mask >>= 1;
        }
        // skip over unused columns
        for (;; col_mask >>= 1) {
            if ((BEAM_COLUMNS & col_mask) != 0) {
                // use this column
                break;
            }
            if (col_mask <= 1) {
                // misconfigured - the number of 1s in BEAM_COLUMNS is < MATRIX_COLS
                col_mask = 1 << 31;
                break;
            }
        }
        select_column(col_mask);
        // drain stale values from input captures
        (void)TIM3->CCR1;
        (void)TIM3->CCR2;
        (void)TIM3->CCR3;
        (void)TIM3->CCR4;
#if MATRIX_ROWS == 8
        (void)TIM4->CCR1;
        (void)TIM4->CCR2;
        (void)TIM4->CCR3;
        (void)TIM4->CCR4;
#endif
    }
    OSAL_IRQ_EPILOGUE();
}

#if BEAM_THRESHOLD > BEAM_COL_WINDOW / 2
#    error Threshold for key-presses is too large to measure in the alotted window. Reduce BEAM_SCAN_RATE.
#endif
#if BEAM_COL_PERIOD >= 65535
#    error Timer period is too long. Increase BEAM_SCAN_RATE.
#endif
bool matrix_scan_custom(matrix_row_t current_matrix[]) {
    // wait for interrupts to scan all the raw values
    if (!chEvtWaitAnyTimeout(ALL_EVENTS, TIME_MS2I(100))) {
        return false;
    }
    // histograms for calibration
    static uint16_t max_any = 0, max_low = 0, min_high = BEAM_COL_WINDOW, min_any = BEAM_COL_WINDOW;
    // convert raw readings to pressed/unpressed state
    bool changed = false;
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        matrix_row_t new_row       = 0;
        matrix_row_t col_mask      = 1;
        matrix_row_t row_dead_keys = dead_keys[row];
        for (uint8_t col = 0; col < MATRIX_COLS; col++, col_mask <<= 1) {
            uint16_t val = beam_values[col][row];
#ifndef BEAM_ACTIVE_NEVER
#    ifdef BEAM_ACTIVE_HIGH
            if (val > BEAM_THRESHOLD) {
#    elif defined(BEAM_ACTIVE_LOW)
            if (val < BEAM_THRESHOLD) {
#    else
#        error define either BEAM_ACTIVE_HIGH or BEAM_ACTIVE_LOW
#    endif
                new_row |= col_mask;
            }
#endif
            // update calibration
            if (val > BEAM_COL_WINDOW) {
                val = BEAM_COL_WINDOW;
            }
            if ((row_dead_keys & col_mask) != 0) {
                if ((cal_keys[row] & col_mask) != 0) {
                    // calibration pad always reads as high capacitance
                    if (val < min_high) {
                        min_high = val;
                    }
                } else {
                    // dead pad always reads as low capacitance
                    if (val > max_low) {
                        max_low = val;
                    }
                }
            }
            if (val > max_any) {
                max_any = val;
            }
            if (val < min_any) {
                min_any = val;
            }
        }
        new_row &= ~dead_keys[row];
        if (new_row != current_matrix[row]) {
            changed             = true;
            current_matrix[row] = new_row;
        }
    }
    if (value_histogram(beam_values)) {
        if (max_low < min_high) {
            xprintf(" %hu < P < %hu < %hu < U < %hu << %hu  m=%hu\n", min_any, max_low, min_high, max_any, BEAM_COL_WINDOW, min_high - max_low);
        } else {
            xprintf(" %hu < P < %hu ! %hu < U < %hu << %hu  m=FAIL\n", min_any, max_low, min_high, max_any, BEAM_COL_WINDOW);
        }
        max_low  = 0;
        max_any  = 0;
        min_high = BEAM_COL_WINDOW;
        min_any  = BEAM_COL_WINDOW;
    }
    return changed;
}
