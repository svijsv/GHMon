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
// calendar.h
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _CALENDAR_H
#define _CALENDAR_H

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
* Variable declarations
*/


/*
* Function prototypes
*/
// Set system date and time
err_t set_time(uint8_t hour, uint8_t minute, uint8_t second);
err_t set_date(uint8_t year, uint8_t month, uint8_t day);


/*
* Macros
*/


#endif // _CALENDAR_H
#ifdef __cplusplus
 }
#endif
