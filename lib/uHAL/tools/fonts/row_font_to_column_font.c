// row_font_to_column_font.c
// Author: svijsv
// License: Public Domain
//
// Convert a font from a C array of bytes where each byte represents one row
// of an 8x8 glyph to a C array of bytes where each byte represents one column
//
// To build, define INFILE as the name of a file containing the font array
// (don't forget the '"'s) and ARRAY_NAME as the name of the array.
//
// To run, execute the binary with the desired outfile name as the sole argument.
//

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef INFILE
# define INFILE "font8x8_basic.h"
#endif
#include INFILE

#ifndef ARRAY_NAME
# define ARRAY_NAME font8x8_basic
#endif

#define XTRINGIZE(x) STRINGIZE(x)
#define STRINGIZE(x) #x

typedef unsigned int uint_t;

int main(int argc, char **argv) {
	char *progname, *outpath, *inpath, *arr_name;
	size_t sbuf;
	FILE* outfile;

	progname = (argv[0] != NULL) ? argv[0] : "<command>";
	inpath = INFILE;
	arr_name = XTRINGIZE(ARRAY_NAME);

	if ((argc != 2) || (argv[1] == NULL)) {
		fprintf(stderr, "Usage: %s <outfile>\nReading '%s' from '%s'\n", progname, arr_name, inpath);
		return 1;
	}

	outpath = argv[1];

	//outfile = fopen(outpath, "ab");
	outfile = fopen(outpath, "wb");
	if (outfile == NULL) {
		fprintf(stderr, "Error opening %s: %s\n", outpath, strerror(errno));
		return 1;
	}

	//fprintf(outfile, "const FMEM_STORAGE uint8_t %s[][8] = {\n", arr_name);
	fprintf(outfile, "const uint8_t %s[] = {\n", arr_name);

	sbuf = sizeof(ARRAY_NAME) / 8;
	for (uint8_t i = 0; i < sbuf; ++i) {
		//uint8_t (*ibuf)[8], obuf[8];
		uint8_t ibuf[8], obuf[8];

		//ibuf = ARRAY_NAME[i];
		memcpy(ibuf, ARRAY_NAME[i], sizeof(ibuf));
		memset(obuf, 0, sizeof(obuf));
		for (uint8_t j = 0; j < 8; ++j) {
			for (uint8_t k = 0; k < 8; ++k) {
				//obuf[k] <<= 1U;
				//obuf[k] |= ibuf[j] & 0x01U;
				//ibuf[j] >>= 1U;

				//obuf[k] |= ibuf[j] & (0x01U << k);

				obuf[k] |= (ibuf[j] & 0x01U) << j;
				ibuf[j] >>= 1U;
			}
		}

		//fprintf(outfile, "\t{ 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X },", (uint_t )obuf[0], (uint_t )obuf[1], (uint_t )obuf[2], (uint_t )obuf[3], (uint_t )obuf[4], (uint_t )obuf[5], (uint_t )obuf[6], (uint_t )obuf[7]);
		fprintf(outfile, "\t0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X,", (uint_t )obuf[0], (uint_t )obuf[1], (uint_t )obuf[2], (uint_t )obuf[3], (uint_t )obuf[4], (uint_t )obuf[5], (uint_t )obuf[6], (uint_t )obuf[7]);

		if ((sbuf == 128) || (sbuf == 256)) {
			char c = ((i >= ' ') && (i <= '~')) ? (char )i : ' ';
			//fprintf(outfile, "   // %3u ('%c')\n", (uint_t )i, c);
			fprintf(outfile, "   // 0x%02X ('%c')\n", (uint_t )i, c);
		}
	}
	fprintf(outfile, "}; // %s\n", arr_name);


	/*
	infile = fopen(inpath, "rb");
	if (infile == NULL) {
		fprintf(stderr, "Error opening %s: %s\n", inpath, strerror(errno));
		return 1;
	}
	fseek(infile, 0, SEEK_END);
	sbuf = ftell(infile) + 1;
	if ((sbuf % 8) != 0) {
		fprintf(stderr, "Unexpected size of %s: expected multiple of 8 bytes\n", inpath);
		return 1;
	}
	buf = malloc(sbuf);
	rewind(infile);
	if (fread(buf, sbuf, 1, infile) == 0) {
		fprintf(stderr, "Error reading %s: %s\n", inpath, strerror(errno));
		return 1;
	}

	fprintf(outfile, "const uint8_t %s[][8] = { \n", arr_name);

	for (size_t i = 0; i < sbuf; i += 8) {
		uint8_t ibuf[8], obuf[8];

		for (size_t j = 0; j < 8; ++j) {
			ibuf[j] = buf[i+j];
			obuf[j] = 0;
		}
		for (size_t j = 0; j < 8; ++j) {
			for (size_t k = 0; k < 8; ++k) {
				obuf[k] <<= 1U;
				obuf[k] |= ibuf[j] & 0x01;
				ibuf[j] >>= 1U;
			}
		}

		fprintf(outfile, "\t{ 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X }, \n", (uint_t )obuf[0], (uint_t )obuf[1], (uint_t )obuf[2], (uint_t )obuf[3], (uint_t )obuf[4], (uint_t )obuf[5], (uint_t )obuf[6], (uint_t )obuf[7]);
	}
	fprintf(outfile, "}; // %s\n", arr_name);
	*/

	return 0;
}
