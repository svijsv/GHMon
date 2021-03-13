// SPDX-License-Identifier: GPL-3.0-only
/***********************************************************************
*                                                                      *
*                                                                      *
* Copyright 2021 svijsv                                                *
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
// interrupts.h
// Manage system IRQs
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _PLATFORM_CMSIS_INTERRUPTS_H
#define _PLATFORM_CMSIS_INTERRUPTS_H

/*
* Includes
*/
#include "common.h"


/*
* Static values
*/


/*
* Types
*/


/*
* Variable declarations (defined in interrupts.c)
*/


/*
* Function prototypes (defined in interrupts.c)
*/
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
// void SysTick_Handler(void);


/*
* Macros
*/


#endif // _PLATFORM_CMSIS_INTERRUPTS_H
#ifdef __cplusplus
 }
#endif
