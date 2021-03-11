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
// config.c
// Program configuration file
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif

//#define DEBUG 1
/*
* Includes
*/
#include "config.h"
#include "common.h"

#include "sensors.h"
#include "controllers.h"

//
// User configuration
//
// Include tables before sensors so that convenience macros can be defined for
// the table indexes
#if USE_LOOKUP_TABLES
# include "../config/tables.c"
#endif

// Include sensors before controllers so that convenience macros can be defined
// for the sensor indexes
#include "../config/sensors.c"

#if USE_CONTROLLERS
# include "../config/controllers.c"
#endif

#ifdef __cplusplus
 }
#endif
