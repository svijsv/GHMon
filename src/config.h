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
// config.h
// Program configuration file
// NOTES:
//

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef _CONFIG_H
#define _CONFIG_H

//#define DEBUG 1
/*
* Includes
*/
#include "platform.h"

// User configuration
#include "../config/config.h"
#include "../config/advanced.h"

// This is used internally to determine which subsystems should be enabled or
// disabled based on user-configured values
#include "config_unify.h"

#endif // _CONFIG_H
#ifdef __cplusplus
 }
#endif
