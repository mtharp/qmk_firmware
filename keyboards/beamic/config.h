// Copyright 2022 Michael Tharp (@mtharp)
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

// bootloader
#define EARLY_INIT_PERFORM_BOOTLOADER_JUMP TRUE

// shift register
#define SREG_DRIVER SPID2

// voltage reference
#define VREF_DRIVER DACD1

// solenoid
#define HAPTIC_ENABLE_PIN LINE_NSEN
#define HAPTIC_ENABLE_PIN_ACTIVE_LOW TRUE
#define HAPTIC_OFF_IN_LOW_POWER 1
#define NO_HAPTIC_MOD 1
#define SOLENOID_PIN LINE_TRG

// eeprom
#define I2C_DRIVER I2CD2
#define I2C1_SCL_PIN A9
#define I2C1_SDA_PIN A10
