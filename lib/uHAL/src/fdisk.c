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
// fdisk.h
// Format SD cards
// NOTES:
//

#include "common.h"

#if uHAL_USE_FDISK

#include FATFS_FF_H_PATH

err_t format_SD(void) {
	err_t err;
	FRESULT res;
	uint8_t buf[FF_MAX_SS];
	MKFS_PARM opt = {
		.fmt = FM_FAT32|FM_FAT, // Choose between FAT and FAT32
		.n_fat = 0,   // Use default number of FATs
		.align = 0,   // Auto-detect block alignment
		.n_root = 0,  // Use default number of root directory entries
		.au_size = 0, // Auto-detect cluster size
	};

#if ! uHAL_SKIP_INIT_CHECKS
#endif
#if ! uHAL_SKIP_INVALID_ARG_CHECKS
#endif

	res = f_mkfs("", &opt, buf, FF_MAX_SS);
	switch (res) {
	case FR_OK:
		err = ERR_OK;
		break;
	case FR_INVALID_DRIVE:
		err = ERR_NODEV;
		break;
	case FR_NOT_READY:
		err = ERR_RETRY;
		break;
	case FR_WRITE_PROTECTED:
		err = ERR_PERM;
		break;
	case FR_DISK_ERR:
		err = ERR_IO
		break;
	case FR_NOT_ENOUGH_CORE:
		err = ERR_NOMEM;
		break;
	case FR_MKFS_ABORTED:
		err = ERR_UNKNOWN;
		break;
	case FR_INVALID_PARAMETER:
		err = ERR_BADARG;
		break;

	default:
		err = ERR_UNKNOWN;
		break;
	}

	return err;
}


#endif // uHAL_USE_FDISK
