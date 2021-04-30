/*------------------------------------------------------------------------*/
/* STM32F100: MMCv3/SDv1/SDv2 (SPI mode) control module                   */
/*------------------------------------------------------------------------*/
/*
/  Copyright (C) 2018, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/
/*
  Modified 2021 svijsv
*/

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

#include "common.h"
#include "fatfs/diskio.h"
#include "ulib/time.h"

#if USE_SD

/* MMC/SD command */
#define CMD0	(0)			/* GO_IDLE_STATE */
#define CMD1	(1)			/* SEND_OP_COND (MMC) */
#define	ACMD41	(0x80+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(8)			/* SEND_IF_COND */
#define CMD9	(9)			/* SEND_CSD */
#define CMD10	(10)		/* SEND_CID */
#define CMD12	(12)		/* STOP_TRANSMISSION */
#define ACMD13	(0x80+13)	/* SD_STATUS (SDC) */
#define CMD16	(16)		/* SET_BLOCKLEN */
#define CMD17	(17)		/* READ_SINGLE_BLOCK */
#define CMD18	(18)		/* READ_MULTIPLE_BLOCK */
#define CMD23	(23)		/* SET_BLOCK_COUNT (MMC) */
#define	ACMD23	(0x80+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(24)		/* WRITE_BLOCK */
#define CMD25	(25)		/* WRITE_MULTIPLE_BLOCK */
#define CMD32	(32)		/* ERASE_ER_BLK_START */
#define CMD33	(33)		/* ERASE_ER_BLK_END */
#define CMD38	(38)		/* ERASE */
#define CMD55	(55)		/* APP_CMD */
#define CMD58	(58)		/* READ_OCR */


/* Physical drive status */
static volatile DSTATUS drive_status = STA_NOINIT;
/* Card type flags */
static BYTE drive_type;


/*-----------------------------------------------------------------------*/
/* SPI controls (Platform dependent)                                     */
/*-----------------------------------------------------------------------*/


static void CS_HIGH(void) {
	gpio_set_state(SPI_CS_SD_PIN, GPIO_HIGH);
	delay(1);

	return;
}
static void CS_LOW(void) {
	gpio_set_state(SPI_CS_SD_PIN, GPIO_LOW);
	delay(1);

	return;
}


/* Exchange a byte */
/*
* dat: Data to send
*/
static BYTE xchg_spi (BYTE tx) {
	BYTE rx;

	spi_exchange_byte(tx, &rx, 100);

	return rx;
}

/* Receive multiple byte */
/*
* buf: Pointer to data buffer
* btr: Number of bytes to receive
*/
static void rx_spi_multi (BYTE *buf, UINT btr) {
	assert(buf != NULL);
	assert(btr > 0);

	/*
	// Detect and handle blocks that aren't a multiple of 2 bytes
	if ((btr % 2) != 0) {
		spi_exchange_byte(0xFF, buf, 100);
		++buf;
		--btr;
	}
	*/

	// TODO: Check the return value and propagate errors
	// SD cards expect MOSI to be held high while they send data
	spi_receive_block(buf, btr, 0xFF, 1000);

	return;
}

#if FF_FS_READONLY == 0
/* Send multiple byte */
/*
* buf: Pointer to the data
* btx: Number of bytes to send
*/
static void tx_spi_multi (const BYTE *buf, UINT btx) {
	assert(buf != NULL);
	assert(btx > 0);

	/*
	// Detect and handle blocks that aren't a multiple of 2 bytes
	if ((btx % 2) != 0) {
		uint8_t rx;

		spi_exchange_byte(buf[0], &rx, 100);
		++buf;
		--btx;
	}
	*/

	// TODO: Check the return value and propagate errors
	spi_transmit_block(buf, btx, 1000);

	return;
}
#endif



/*-----------------------------------------------------------------------*/
/* Platform-independent code after this point                            */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/
/*
* 1:Ready, 0:Timeout
* wt: Timeout [ms]
*/
static int wait_ready (UINT wt) {
	BYTE d;
	utime_t timeout;

	timeout = SET_TIMEOUT(wt);
	do {
		d = xchg_spi(0xFF);
		/* This loop takes a time. Insert rot_rdq() here for multitask envilonment. */
	} while (d != 0xFF && !TIMES_UP(timeout)); /* Wait for card goes ready or timeout */

	return (d == 0xFF) ? 1 : 0;
}

/*-----------------------------------------------------------------------*/
/* Deselect card and release SPI                                         */
/*-----------------------------------------------------------------------*/
static void deselect_drive (void) {
	CS_HIGH(); /* Set CS# high */
	xchg_spi(0xFF); /* Dummy clock (force DO hi-z for multiple slave SPI) */

	return;
}

/*-----------------------------------------------------------------------*/
/* Select card and wait for ready                                        */
/*-----------------------------------------------------------------------*/
/*
* 1:OK, 0:Timeout
*/
static int select_drive (void) {
	CS_LOW(); /* Set CS# low */
	xchg_spi(0xFF); /* Dummy clock (force DO enabled) */
	if (wait_ready(500)) { /* Wait for card ready */
		return 1;
	}

	deselect_drive();
	return 0; /* Timeout */
}

/*-----------------------------------------------------------------------*/
/* Receive a data packet from the MMC                                    */
/*-----------------------------------------------------------------------*/
/*
* 1:OK, 0:Error
* buf: Data buffer
* btr: Data block length (byte)
*/
static int rx_datablock (BYTE *buf, UINT btr) {
	BYTE token;
	utime_t timeout;

	assert(buf != NULL);
	assert(btr > 0);

	timeout = SET_TIMEOUT(200);
	do { /* Wait for DataStart token in timeout of 200ms */
		token = xchg_spi(0xFF);
		/* This loop will take a time. Insert rot_rdq() here for multitask envilonment. */
	} while ((token == 0xFF) && !TIMES_UP(timeout));
	if(token != 0xFE) { /* Function fails if invalid DataStart token or timeout */
		return 0;
	}

	rx_spi_multi(buf, btr); /* Store trailing data to the buffer */
	xchg_spi(0xFF); xchg_spi(0xFF); /* Discard CRC */

	return 1; /* Function succeeded */
}

/*-----------------------------------------------------------------------*/
/* Send a data packet to the MMC                                         */
/*-----------------------------------------------------------------------*/
/*
* 1:OK, 0:Failed
* buf  : Ponter to 512 byte data to be sent
* token: Token
*/
#if FF_FS_READONLY == 0
static int tx_datablock (const BYTE *buf, BYTE token) {
	BYTE resp;

	assert(buf != NULL);

	/* Wait for card ready */
	if (!wait_ready(500)) {
		return 0;
	}

	xchg_spi(token); /* Send token */
	if (token != 0xFD) { /* Send data if token is other than StopTran */
		tx_spi_multi(buf, 512); /* Data */
		xchg_spi(0xFF); xchg_spi(0xFF); /* Dummy CRC */

		resp = xchg_spi(0xFF); /* Receive data resp */
		if ((resp & 0x1F) != 0x05) { /* Function fails if the data packet was not accepted */
			return 0;
		}
	}
	return 1;
}
#endif

/*-----------------------------------------------------------------------*/
/* Send a command packet to the MMC                                      */
/*-----------------------------------------------------------------------*/
/*
* Return value: R1 resp (bit7==1:Failed to send)
* cmd: Command index
* arg: Argument
*/
static BYTE send_cmd (BYTE cmd, DWORD arg) {
	BYTE n, res;

	/* Send a CMD55 prior to ACMD<n> */
	if (cmd & 0x80) {
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) {
			return res;
		}
	}

	/* Select the card and wait for ready except to stop multiple block read */
	if (cmd != CMD12) {
		deselect_drive();
		if (!select_drive()) {
			return 0xFF;
		}
	}

	/* Send command packet */
	xchg_spi(0x40 | cmd);        /* Start + command index */
	xchg_spi((BYTE)(arg >> 24)); /* Argument[31..24] */
	xchg_spi((BYTE)(arg >> 16)); /* Argument[23..16] */
	xchg_spi((BYTE)(arg >> 8));  /* Argument[15..8] */
	xchg_spi((BYTE)arg);         /* Argument[7..0] */
	switch (cmd) {
	case CMD0:
		n = 0x95; /* Valid CRC for CMD0(0) */
		break;
	case CMD8:
		n = 0x87; /* Valid CRC for CMD8(0x1AA) */
		break;
	default:
		n = 0x01; /* Dummy CRC + Stop */
		break;
	}
	xchg_spi(n);

	/* Receive command resp */
	if (cmd == CMD12) { /* Discard following one byte when CMD12 */
		xchg_spi(0xFF);
	}
	/* Wait for response (10 bytes max) */
	n = 10;
	do {
		res = xchg_spi(0xFF);
	} while ((res & 0x80) && --n);

	return res; /* Return received response */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize disk drive                                                 */
/*-----------------------------------------------------------------------*/
/*
* lun: Drive number (always 0)
* Assumes SPI is already set up.
*/
DSTATUS disk_initialize (BYTE lun) {
	BYTE n, cmd, type, ocr[4];
	utime_t timeout;

	if (lun != 0) {
		return STA_NOINIT;
	}

	/* Is card existing in the soket? */
	if (drive_status & STA_NODISK) {
		return drive_status;
	}

	/* Send 80 dummy clocks */
	for (n = 10; n; n--) {
		xchg_spi(0xFF);
	}

	type = 0;
	if (send_cmd(CMD0, 0) == 1) { /* Put the card SPI/Idle state */
		timeout = SET_TIMEOUT(1000); /* Initialization timeout = 1 sec */
		if (send_cmd(CMD8, 0x1AA) == 1) { /* SDv2? */
			for (n = 0; n < 4; n++) { /* Get 32 bit return value of R7 resp */
				ocr[n] = xchg_spi(0xFF);
			}
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) { /* Is the card supports vcc of 2.7-3.6V? */
				while (!TIMES_UP(timeout) && send_cmd(ACMD41, 1UL << 30)) { /* Wait for end of initialization with ACMD41(HCS) */
				}
				if (!TIMES_UP(timeout) && send_cmd(CMD58, 0) == 0) { /* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = xchg_spi(0xFF);
					type = (ocr[0] & 0x40) ? CT_SDC2 | CT_BLOCK : CT_SDC2; /* Card id SDv2 */
				}
			}
		} else { /* Not SDv2 card */
			if (send_cmd(ACMD41, 0) <= 1) { /* SDv1 or MMC? */
				type = CT_SDC1; cmd = ACMD41; /* SDv1 (ACMD41(0)) */
			} else {
				type = CT_MMC3; cmd = CMD1; /* MMCv3 (CMD1(0)) */
			}
			while (!TIMES_UP(timeout) && send_cmd(cmd, 0)) { /* Wait for end of initialization */
			}
			if (TIMES_UP(timeout) || send_cmd(CMD16, 512) != 0) { /* Set block length: 512 */
				type = 0;
			}
		}
	}
	drive_type = type; /* Card type */
	deselect_drive();

	if (type != 0) { /* OK */
		drive_status &= ~STA_NOINIT; /* Clear STA_NOINIT flag */
	} else { /* Failed */
		drive_status = STA_NOINIT;
	}

	return drive_status;
}

/*-----------------------------------------------------------------------*/
/* Get disk status                                                       */
/*-----------------------------------------------------------------------*/
/*
* lun: Drive number (always 0)
*/
DSTATUS disk_status (BYTE lun) {
	if (lun != 0) {
		return STA_NOINIT;
	}

	return drive_status;
}

/*-----------------------------------------------------------------------*/
/* Read sector(s)                                                        */
/*-----------------------------------------------------------------------*/
/*
* lun   : Drive number (always 0)
* buf   : Pointer to the data buffer to store read data
* sector: Start sector number (LBA)
* count : Number of sectors to read (1..128)
*/
DRESULT disk_read (BYTE lun, BYTE *buf, LBA_t sector, UINT count) {
	DWORD sect = (DWORD)sector;

	assert(buf != NULL);

	if (lun != 0) {
		return RES_PARERR;
	}
	if (count == 0) {
		return RES_PARERR;
	}
	if (drive_status & STA_NOINIT) {
		return RES_NOTRDY;
	}

	/* LBA ot BA conversion (byte addressing cards) */
	if (!(drive_type & CT_BLOCK)) {
		sect *= 512;
	}

	if (count == 1) { /* Single sector read */
		if ((send_cmd(CMD17, sect) == 0) && rx_datablock(buf, 512)) { /* READ_SINGLE_BLOCK */
			count = 0;
		}
	} else { /* Multiple sector read */
		if (send_cmd(CMD18, sect) == 0) { /* READ_MULTIPLE_BLOCK */
			do {
				if (!rx_datablock(buf, 512)) {
					break;
				}
				buf += 512;
			} while (--count);
			send_cmd(CMD12, 0); /* STOP_TRANSMISSION */
		}
	}
	deselect_drive();

	return count ? RES_ERROR : RES_OK; /* Return result */
}

/*-----------------------------------------------------------------------*/
/* Write sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/*
* lun   : Drive number (always 0)
* buf   : Pointer to the data to write
* sector: Start sector number (LBA)
* count : Number of sectors to write (1..128)
*/
#if FF_FS_READONLY == 0
DRESULT disk_write (BYTE lun, const BYTE *buf, LBA_t sector, UINT count) {
	DWORD sect = (DWORD)sector;

	assert(buf != NULL);

	LOGGER("Syncing %u sectors to SD", (uint )count);

	if (lun != 0) {
		return RES_PARERR;
	}
	if (count == 0) {
		return RES_PARERR;
	}
	if (drive_status & STA_NOINIT) {
		return RES_NOTRDY;
	}
	if (drive_status & STA_PROTECT) {
		return RES_WRPRT;
	}

	/* LBA ==> BA conversion (byte addressing cards) */
	if (!(drive_type & CT_BLOCK)) {
		sect *= 512;
	}

	if (count == 1) { /* Single sector write */
		if ((send_cmd(CMD24, sect) == 0) && tx_datablock(buf, 0xFE)) { /* WRITE_BLOCK */
			count = 0;
		}
	} else { /* Multiple sector write */
		if (drive_type & CT_SDC) send_cmd(ACMD23, count); /* Predefine number of sectors */
		if (send_cmd(CMD25, sect) == 0) { /* WRITE_MULTIPLE_BLOCK */
			do {
				if (!tx_datablock(buf, 0xFC)){
					break;
				}
				buf += 512;
			} while (--count);
			if (!tx_datablock(0, 0xFD)) { /* STOP_TRAN token */
				count = 1;
			}
		}
	}
	deselect_drive();

	return count ? RES_ERROR : RES_OK; /* Return result */
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous drive controls other than data read/write               */
/*-----------------------------------------------------------------------*/

/*
* lun: Drive number (always 0)
* cmd: Control command code
* buf: Pointer to control data
*/
DRESULT disk_ioctl (BYTE lun, BYTE cmd, void *buf) {
	DRESULT res;
	BYTE n, csd[16];
	DWORD st, ed, csize;
	LBA_t *dp;

	// check buf on a case-by-case basis because not all ioctls use it.
	switch (cmd) {
	case CTRL_SYNC:
	case CTRL_TRIM:
		break;
	default:
		assert(buf != NULL);
		break;
	}

	if (lun != 0) {
		return RES_PARERR;
	}
	if (drive_status & STA_NOINIT) {
		return RES_NOTRDY;
	}
	res = RES_ERROR;

	switch (cmd) {
	/* Wait for end of internal write process of the drive */
	case CTRL_SYNC :
		if (select_drive()) {
			res = RES_OK;
		}
		break;

	/* Get drive capacity in unit of sector (DWORD) */
	case GET_SECTOR_COUNT :
		if ((send_cmd(CMD9, 0) == 0) && (rx_datablock(csd, 16) != 0)) {
			if ((csd[0] >> 6) == 1) { /* SDC CSD ver 2 */
				csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				*(LBA_t*)buf = csize << 10;
			} else { /* SDC CSD ver 1 or MMC */
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				*(LBA_t*)buf = csize << (n - 9);
			}
			res = RES_OK;
		}
		break;

	/* Get erase block size in unit of sector (DWORD) */
	case GET_BLOCK_SIZE :
		if (drive_type & CT_SDC2) { /* SDC ver 2+ */
			if (send_cmd(ACMD13, 0) == 0) { /* Read SD status */
				xchg_spi(0xFF);
				if (rx_datablock(csd, 16)) { /* Read partial block */
					for (n = 64 - 16; n; n--) xchg_spi(0xFF); /* Purge trailing data */
					*(DWORD*)buf = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else { /* SDC ver 1 or MMC */
			if ((send_cmd(CMD9, 0) == 0) && rx_datablock(csd, 16)) { /* Read CSD */
				if (drive_type & CT_SDC1) { /* SDC ver 1.XX */
					*(DWORD*)buf = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else { /* MMC */
					*(DWORD*)buf = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		break;

	/* Erase a block of sectors (used when _USE_ERASE == 1) */
	case CTRL_TRIM :
		if (!(drive_type & CT_SDC)) { /* Check if the card is SDC */
			break;
		}
		if (disk_ioctl(lun, MMC_GET_CSD, csd)) { /* Get CSD */
			break;
		}
		if (!(csd[10] & 0x40)) { /* Check if ERASE_BLK_EN = 1 */
			break;
		}
		dp = buf; st = (DWORD)dp[0]; ed = (DWORD)dp[1]; /* Load sector block */
		if (!(drive_type & CT_BLOCK)) {
			st *= 512; ed *= 512;
		}
		if (send_cmd(CMD32, st) == 0 && send_cmd(CMD33, ed) == 0 && send_cmd(CMD38, 0) == 0 && wait_ready(30000)) { /* Erase sector block */
			res = RES_OK; /* FatFs does not check result of this command */
		}
		break;

	/* Following commands are never used by FatFs module */

	/* Get MMC/SDC type (BYTE) */
	case MMC_GET_TYPE:
		*(BYTE*)buf = drive_type;
		res = RES_OK;
		break;

	/* Read CSD (16 bytes) */
	case MMC_GET_CSD:
		if (send_cmd(CMD9, 0) == 0 && rx_datablock((BYTE*)buf, 16)) { /* READ_CSD */
			res = RES_OK;
		}
		break;

	/* Read CID (16 bytes) */
	case MMC_GET_CID:
		if (send_cmd(CMD10, 0) == 0 && rx_datablock((BYTE*)buf, 16)) { /* READ_CID */
			res = RES_OK;
		}
		break;

	/* Read OCR (4 bytes) */
	case MMC_GET_OCR:
		if (send_cmd(CMD58, 0) == 0) { /* READ_OCR */
			for (n = 0; n < 4; n++) *(((BYTE*)buf) + n) = xchg_spi(0xFF);
			res = RES_OK;
		}
		break;

	/* Read SD status (64 bytes) */
	case MMC_GET_SDSTAT:
		if (send_cmd(ACMD13, 0) == 0) { /* SD_STATUS */
			xchg_spi(0xFF);
			if (rx_datablock((BYTE*)buf, 64)) res = RES_OK;
		}
		break;

	default:
		res = RES_PARERR;
	}

	deselect_drive();

	return res;
}
// 
// http:// www.elm-chan.org/fsw/ff/doc/fattime.html
DWORD get_fattime (void) {
	uint32_t now, fatnow;
	uint8_t year, month, day, hour, minute, second;

	now = NOW();
	seconds_to_date(now, &year, &month, &day);
	seconds_to_time(now, &hour, &minute, &second);

	// bit31:25 Year origin from the 1980 (0..127, e.g. 37 for 2017)
	fatnow = (uint32_t )((uint32_t )year + (YEAR_0 - 1980)) << 25;
	// bit24:21 Month (1..12)
	fatnow |= (uint32_t )month << 21;
	// bit20:16 Day of the month (1..31)
	fatnow |= (uint32_t )day << 16;
	// bit15:11 Hour (0..23)
	fatnow |= (uint32_t )hour << 11;
	// bit10:5 Minute (0..59)
	fatnow |= (uint32_t )minute << 5;
	// bit4:0 Second / 2 (0..29, e.g. 25 for 50) 
	fatnow |= (uint32_t )(second/2);

	return fatnow;
}

#endif // USE_SD
