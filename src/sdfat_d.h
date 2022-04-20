
#ifndef _C_sdfat_h
#define _C_sdfat_h

#include "types.h"

#include "SD.h"

// #define MOUNT_POINT "/sd"

// Pin assignments can be set in menuconfig, see "SD SPI Example Configuration" menu.
// You can also change the pin assignments here by changing the following 4 lines.
#define SD_PIN_NUM_MISO  2
#define SD_PIN_NUM_MOSI  12
#define SD_PIN_NUM_CLK   14
#define SD_PIN_NUM_CS    13

typedef struct SDFAT {
    SPIClass spi;
    BOOL mount;
} SDFAT;

void sdFatInit();
bool sdFatMount();
void sdFatUnMount();

void listDir(fs::FS &fs, const char *dirname, uint8_t levels);

#endif
