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
// config.h
// Program configuration file
// NOTES:
//    The order the configuration headers are included is a bit haphazard, but in
//    brief:
//       The main program includes uHAL's interface.h which pulls in config_uHAL.h
//       which pulls in this before doing anything else.
//
//       uHAL *also* pulls this in by way of interface.h.
//
//       ulib pulls this in via ulibconfig.h.
//    Because of the parallel include chains, it's important that neither library
//    configuration file change the main configuration for any reason in order to
//    keep them in sync. They can still modify themselves based on the main
//    configuration though, which is a big part of why we do things this way.
//
#ifndef _CONFIG_GENERAL_CONFIG_H
#define _CONFIG_GENERAL_CONFIG_H

#ifndef GHMON_INSTANCE_CONFIG_DIR
# define GHMON_INSTANCE_CONFIG_DIR config/instance
#endif

//
// Some helpers so we can configure header locations
#define GHMON_STRINGIZE(_x_) #_x_
#define _GHMON_INCLUDE_CONFIG_HEADER(_path_, _file_) GHMON_STRINGIZE(_path_/_file_)
#define GHMON_INCLUDE_CONFIG_HEADER(_file_) _GHMON_INCLUDE_CONFIG_HEADER(GHMON_INSTANCE_CONFIG_DIR, _file_)

//
// Instance configuration
#include GHMON_INCLUDE_CONFIG_HEADER(config.h)
#include GHMON_INCLUDE_CONFIG_HEADER(advanced.h)

// We use these elsewhere
//#undef GHMON_STRINGIZE
//#undef _GHMON_INCLUDE_CONFIG_HEADER
//#undef GHMON_INCLUDE_CONFIG_HEADER

#endif // _CONFIG_GENERAL_CONFIG_H
