/*
*
* Look-up table definitions
* See sensors.h for structure documentation
* Try tools/VLUTgen.py for generating lookup tables for thermistors
*
*/
_FLASH const sensor_LUT_t LOOKUP_TABLES[] = {
// A selection of pre-computed tables is available in config/tables; see those
// files for comments about their suitability
#include "tables/VC_3950B.tab"
#include "tables/VF_3950B.tab"
};
// Defining the indexes of the tables makes it easier to reference them from
// sensors.c
#define TAB_VC_3950B 0
#define TAB_VF_3950B 1
