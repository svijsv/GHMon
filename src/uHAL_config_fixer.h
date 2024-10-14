// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021, 2024 svijsv                                          *
* This program is free software: you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation, version 3.                             *
*                                                                      *
* This program is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU    *
* General Public License for more details.                             *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program.  If not, see <http:// www.gnu.org/licenses/>.*
*                                                                      *
*                                                                      *
***********************************************************************/
// config_fixer.h
// Determine which subsystems to enable or disable based on user configuration
// NOTES:
//
#ifndef _CONFIG_FIXER_H
#define _CONFIG_FIXER_H


#ifndef DEBUG
# define DEBUG 0
#endif
#ifndef NDEBUG
# if ! DEBUG
#  define NDEBUG 1
# endif
#endif

#if uHAL_USE_FATFS_SD && !uHAL_USE_FATFS
# error "uHAL_USE_FATFS_SD requires uHAL_USE_FATFS"
#endif
#if uHAL_USE_FDISK && !uHAL_USE_FATFS
# error "uHAL_USE_FDISK requires uHAL_USE_FATFS"
#endif
#if uHAL_USE_FDISK && !uHAL_USE_TERMINAL
# error "uHAL_USE_FDISK requires uHAL_USE_TERMINAL"
#endif
#if uHAL_USE_TERMINAL && !uHAL_USE_UART_COMM
# error "uHAL_USE_TERMINAL requires uHAL_USE_UART_COMM"
#endif
#if uHAL_USE_UART_COMM && !uHAL_USE_UART
# error "uHAL_USE_UART_COMM requires uHAL_USE_UART"
#endif
#if uHAL_USE_FATFS_SD && !uHAL_USE_SPI
# error "uHAL_USE_FATFS_SD requires uHAL_USE_SPI"
#endif
// We use a 16-bit duty cycle
#if PWM_DUTY_CYCLE_SCALE > 0xFFFFU
# error "PWM_DUTY_CYCLE_SCALE can not be > 0xFFFF"
#endif

// Platforms that can use RTC emulation define this in their configuration files,
// the ones that don't still need it defined as something though.
#ifndef uHAL_USE_RTC_EMULATION
# define uHAL_USE_RTC_EMULATION 0
#endif


#endif // _CONFIG_FIXER_H
