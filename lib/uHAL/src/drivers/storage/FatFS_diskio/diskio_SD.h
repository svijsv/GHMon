/*-----------------------------------------------------------------------
/  Low level disk interface modlue include file   (C)ChaN, 2014
/-----------------------------------------------------------------------*/
/*
  Modified 2024 svijsv
*/
/// @file
/// @brief SD card SPI driver derived from an example found at http://elm-chan.org/fsw/ff/00index_e.html
///
/// @attention
/// This driver requires the FatFS headers be available in the C header search path.
///
/// @attention
/// Because this driver is intended for use by FatFS, the return codes differ from
/// those used throughout the rest of the HAL.
///
/// @attention
/// The macro @c SPI_CS_SD_PIN must be set to the gpio_pin_t of the SD card's chip-select pin
/// and must be set to GPIO_MODE_PP and set HIGH on initialization.
///
// This file was originally a modification of FatFS's diskio.h. It just adds Doxygen
// comments and re-arranges things a bit. I'm abandoning most of it because it's no
// longer being exported as part of the API (and so Doxygen comments aren't needed)
// and because it needs to match between diskio_SD.c and FatFS and that's easier to
// guarantee if they both include the same header.
#ifndef _uHAL_DRIVERS_STORAGE_DISKIO_DISKIO_SD_H
#define _uHAL_DRIVERS_STORAGE_DISKIO_DISKIO_SD_H

#include FATFS_FF_H_PATH
#include FATFS_DISKIO_H_PATH


/*
///
/// @name Status of Disk Functions
/// @{
//
///
/// Disk Status type.
typedef BYTE	DSTATUS;
//  Disk Status Bits (DSTATUS) 
#define STA_NOINIT		0x01	/// < Drive not initialized 
#define STA_NODISK		0x02	/// < No medium in the drive 
#define STA_PROTECT		0x04	/// < Write protected 
/// @}
*/

/*
///
/// Results of Disk Functions
///
typedef enum {
	RES_OK = 0,		/// < 0: Successful 
	RES_ERROR,		/// < 1: R/W Error 
	RES_WRPRT,		/// < 2: Write Protected 
	RES_NOTRDY,		/// < 3: Not Ready 
	RES_PARERR		/// < 4: Invalid Parameter 
} DRESULT;
*/

/*
///
/// @name Command code for disk_ioctrl function
/// @{
//
//  Generic command (Used by FatFs) 
#define CTRL_SYNC			0	/// < Complete pending write process (needed at FF_FS_READONLY == 0) 
#define GET_SECTOR_COUNT	1	/// < Get media size (needed at FF_USE_MKFS == 1) 
#define GET_SECTOR_SIZE		2	/// < Get sector size (needed at FF_MAX_SS != FF_MIN_SS) 
#define GET_BLOCK_SIZE		3	/// < Get erase block size (needed at FF_USE_MKFS == 1) 
#define CTRL_TRIM			4	/// < Inform device that the data on the block of sectors is no longer used (needed at FF_USE_TRIM == 1) 

//  Generic command (Not used by FatFs) 
#define CTRL_FORMAT			5	/// < Create physical format on the media 
#define CTRL_POWER_IDLE		6	/// < Put the device idle state 
#define CTRL_POWER_OFF		7	/// < Put the device off state 
#define CTRL_LOCK			8	/// < Lock media removal 
#define CTRL_UNLOCK			9	/// < Unlock media removal 
#define CTRL_EJECT			10	/// < Eject media 
#define CTRL_GET_SMART		11	/// < Read SMART information 

//  MMC/SDC specific command (Not used by FatFs) 
#define MMC_GET_TYPE		50	/// < Get card type 
#define MMC_GET_CSD			51	/// < Read CSD 
#define MMC_GET_CID			52	/// < Read CID 
#define MMC_GET_OCR			53	/// < Read OCR 
#define MMC_GET_SDSTAT		54	/// < Read SD status 
#define ISDIO_READ			55	/// < Read data form SD iSDIO register 
#define ISDIO_WRITE			56	/// < Write data to SD iSDIO register 
#define ISDIO_MRITE			57	/// < Masked write data to SD iSDIO register 

//  ATA/CF specific command (Not used by FatFs) 
#define ATA_GET_REV			60	/// < Get F/W revision 
#define ATA_GET_MODEL		61	/// < Get model name 
#define ATA_GET_SN			62	/// < Get serial number 
*/


//  MMC card type flags (MMC_GET_TYPE) 
#define CT_MMC3		0x01		/// < MMC ver 3 
#define CT_MMC4		0x02		/// < MMC ver 4+ 
#define CT_MMC		0x03		/// < MMC 
#define CT_SDC1		0x02		/// < SDC ver 1 
#define CT_SDC2		0x04		/// < SDC ver 2+ 
#define CT_SDC		0x0C		/// < SDC 
#define CT_BLOCK	0x10		/// < Block addressing 
/// @}


// ---------------------------------------
//  Prototypes for disk control functions 
// ---------------------------------------

/*
///
/// Initialize an SD card.
///
/// @param pdrv The drive number. Must be 0 for now.
///
/// @returns 0 if successful, otherwise an DSTATUS code indicating the nature
///  of the problem encountered.
DSTATUS disk_initialize (BYTE pdrv);

///
/// Check the status of an SD card.
///
/// @param pdrv The drive number. Must be 0 for now.
///
/// @returns A DSTATUS code indicatint the current status of the drive.
DSTATUS disk_status (BYTE pdrv);

///
/// Read an SD card.
///
/// @param pdrv The drive number. Must be 0 for now.
/// @param buff The data buffer in which to store the data.
/// @param sector The start sector number.
/// @param count The number of sectors to read. Must be 1-128.
///
/// @returns RES_OK if successful, otherwise a DRESULT error code indicating
///  the nature of the problem encountered.
DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count);

///
/// Write to an SD card.
///
/// @param pdrv The drive number. Must be 0 for now.
/// @param buff The data buffer from which to write the data.
/// @param sector The start sector number.
/// @param count The number of sectors to write. Must be 1-128.
///
/// @returns RES_OK if successful, otherwise a DRESULT error code indicating
///  the nature of the problem encountered.
DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count);

///
/// Miscellaneous drive controls other than data read/write.
///
/// @param pdrv The drive number. Must be 0 for now.
/// @param cmd The command to issue.
/// @param buff The control data buffer.
///
/// @attention Only those commands needed by FatFS are implemented.
///
/// @returns RES_OK if successful, otherwise a DRESULT error code indicating
///  the nature of the problem encountered.
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);
*/


#endif // _uHAL_DRIVERS_STORAGE_DISKIO_DISKIO_SD_H
