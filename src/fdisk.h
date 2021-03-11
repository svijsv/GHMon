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
// fdisk.h
// Format SD cards
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _FDISK_H
#define _FDISK_H

/*
* Includes
*/
#include "config.h"
#include "common.h"

// Need to check this *after* the headers are included
#if USE_FDISK


/*
* Static values
*/


/*
* Types
*/


/*
* Variable declarations (defined in log.c)
*/


/*
* Function prototypes (defined in log.c)
*/
// Format the SD card
err_t format_SD(void);


/*
* Macros
*/


#endif // USE_FDISK


#endif // _FDISK_H
#ifdef __cplusplus
 }
#endif
