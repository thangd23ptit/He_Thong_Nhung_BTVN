#ifndef __FAT32_H
#define __FAT32_H

#include <stdint.h>

#define FAT32_OK       0
#define FAT32_ERROR    1

#define FAT32_EOC      0x0FFFFFF8UL
#define FAT32_BAD      0x0FFFFFF7UL

uint8_t FAT32_Init(void);

uint8_t FAT32_CreateFile(
    const char *filename
);

uint8_t FAT32_AppendFile(
    const char *filename,
    const char *data,
    uint32_t length
);

uint8_t FAT32_ReadFile(
    const char *filename
);

void FAT32_PrintStatus(void);

#endif