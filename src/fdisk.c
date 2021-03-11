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

//#define DEBUG 1
/*
* Includes
*/
#include "fdisk.h"
#include "power.h"
#include "fatfs/ff.h"

#if USE_FDISK

/*
* Static values
*/


/*
* Types
*/


/*
* Variables
*/


/*
* Local function prototypes
*/


/*
* Interrupt handlers
*/


/*
* Functions
*/
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

	LOGGER("Formatting SD card");

	power_on_SD();

	// Is this right for the drive name? should it be "/" or something?
	res = f_mkfs("", &opt, buf, FF_MAX_SS);
	switch (res) {
	case FR_INVALID_DRIVE:
		err = EINVALID;
		break;
	case FR_NOT_READY:
		err = ENOTREADY;
		break;
	case FR_WRITE_PROTECTED:
		err = EPERM;
		break;
	case FR_DISK_ERR:
		err = EDISK;
		break;
	case FR_NOT_ENOUGH_CORE:
		err = EMEM;
		break;
	case FR_MKFS_ABORTED:
		err = EABORT;
		break;
	case FR_INVALID_PARAMETER:
		err = EUSAGE;
		break;

	default:
		err = EUNKNOWN;
		break;
	}
	if (res != FR_OK) {
		LOGGER("SD formatting error %d (fatfs error %d)", (int )err, (int )res);
	}

	power_off_SD();

	return err;
}


#endif // USE_FDISK

#ifdef __cplusplus
 }
#endif
