#ifndef __SD_H
#define __SD_H

#include <stdint.h>

#define SD_OK       0
#define SD_ERROR    1

#define SD_SDSC     0
#define SD_SDHC     1

uint8_t SD_Init(void);

uint8_t SD_ReadBlock(
    uint32_t sector,
    uint8_t *buffer
);

uint8_t SD_WriteBlock(
    uint32_t sector,
    const uint8_t *buffer
);

uint8_t SD_GetType(void);

#endif