
#ifndef _C_spifat_h
#define _C_spifat_h

#include "FS.h"
#include "FFat.h"

typedef struct SPIFAT {
    SPIClass spi;
    bool mount;
} SPIFAT;

void spiFatInit();
bool spiFatMount();
void spiFatUnMount();

void spiPrintDirectory(File dir, int numTabs);

// void spiListDir(fs::FS &fs, const char *dirname, uint8_t levels);

#endif