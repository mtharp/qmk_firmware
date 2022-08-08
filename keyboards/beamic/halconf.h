// Copyright 2022 Michael Tharp (@mtharp)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "mcuconf.h"

#define _CHIBIOS_HAL_CONF_
#define _CHIBIOS_HAL_CONF_VER_8_0_

#define HAL_USE_PAL                         TRUE
#define HAL_USE_ADC                         FALSE
#define HAL_USE_CAN                         FALSE
#define HAL_USE_CRY                         FALSE
#define HAL_USE_DAC                         TRUE
#define HAL_USE_EFL                         FALSE
#define HAL_USE_GPT                         FALSE
#define HAL_USE_I2C                         TRUE
#define HAL_USE_I2S                         FALSE
#define HAL_USE_ICU                         FALSE
#define HAL_USE_MAC                         FALSE
#define HAL_USE_MMC_SPI                     FALSE
#define HAL_USE_PWM                         FALSE
#define HAL_USE_RTC                         FALSE
#define HAL_USE_SDC                         FALSE
#define HAL_USE_SERIAL                      FALSE
#define HAL_USE_SERIAL_USB                  TRUE
#define HAL_USE_SIO                         FALSE
#define HAL_USE_SPI                         TRUE
#define HAL_USE_TRNG                        FALSE
#define HAL_USE_UART                        FALSE
#define HAL_USE_USB                         TRUE
#define HAL_USE_WDG                         FALSE
#define HAL_USE_WSPI                        FALSE

#define PAL_USE_CALLBACKS                   FALSE
#define PAL_USE_WAIT                        FALSE
#define DAC_USE_WAIT                        FALSE
#define DAC_USE_MUTUAL_EXCLUSION            FALSE
#define I2C_USE_MUTUAL_EXCLUSION            TRUE
#define SERIAL_USB_BUFFERS_SIZE             256
#define SERIAL_USB_BUFFERS_NUMBER           2
#define SPI_USE_WAIT                        TRUE
#define SPI_USE_ASSERT_ON_ERROR             TRUE
#define SPI_USE_MUTUAL_EXCLUSION            FALSE
#define SPI_SELECT_MODE                     SPI_SELECT_MODE_NONE
#define USB_USE_WAIT                        TRUE
