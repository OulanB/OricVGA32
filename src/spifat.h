/* Wear levelling and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   This sample shows how to store files inside a FAT filesystem.
   FAT filesystem is stored in a partition inside SPI flash, using the
   flash wear levelling library.
*/

#include "spifat_d.h"

#undef THIS 
#define THIS System.SpiFat

static const char *TAG = "example";

// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

// Mount path for the partition
const char *base_path = "/flash";

void spiPrintDirectory(File dir, int numTabs) {
      while (true) {    
        File entry =  dir.openNextFile();
        if (! entry) { // no more files
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++) {
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory()) {
            Serial.println("/");
            spiPrintDirectory(entry, numTabs + 1);
        } else { // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}

void spiFatInit(void) {
    /*
    Serial.print(F("Inizializing spiFat..."));

    if (FFat.begin(false, "/spi")) {
        THIS.mount = true;
        Serial.println(F("done."));
    } else {
        THIS.mount = false;
        Serial.println(F("fail."));
    }
 
    // To format all space in FFat
    // FFat.format();
    
    // Get all information of your FFat
 
    unsigned int totalBytes = FFat.totalBytes();
    unsigned int usedBytes = FFat.usedBytes();
    unsigned int freeBytes  = FFat.freeBytes();

    Serial.println("File system info.");
 
    Serial.print("Total space:      ");
    Serial.print(totalBytes);
    Serial.println(" bytes");
 
    Serial.print("Total space used: ");
    Serial.print(usedBytes);
    Serial.println(" bytes");
 
    Serial.print("Total space free: ");
    Serial.print(freeBytes);
    Serial.println( " bytes");
 
    Serial.println();
    // Open dir folder
    // File dir = FFat.open("/");
    // Cycle all the content
    // spiPrintDirectory(dir, 0); */
}

bool spiFatMount() {
    if (THIS.mount) {
        spiFatUnMount();
    }
	if (FFat.begin(false, "/spi")) {
		THIS.mount = true;
	}
    return THIS.mount;
}

void spiFatUnMount() {
	if (THIS.mount) {
        FFat.end();
		THIS.mount = false;
	}
}