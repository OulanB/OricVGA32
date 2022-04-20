
#ifndef _C_file_h
#define _C_file_h

#include "types.h"

#include "System_d.h"

void fileCopySdFatToSpiFat(char *sdInDir, char *spiOutDir) {
    char nameIn[256];
    char nameOut[256];
    Serial.printf("Listing directory: %s\n", sdInDir);

    File root = SD.open(sdInDir, "rb");
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
	if (!root.isDirectory()) {
		Serial.println("Not a directory");
		return;
	}

	File file = root.openNextFile();
	while (file) {
		if (file.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.println(file.name());
//			if (levels) {
//				listDir(fs, file.name(), levels - 1);
//			}
		} else {
            char buff[256];
            size_t l;
            size_t s = 0;
			Serial.print("  FILE: ");
			Serial.print(file.name());
			Serial.print("  SIZE: ");
			Serial.println(file.size());
            *nameIn = 0x00;
            strcat(nameIn, file.name());
            *nameOut = 0x00;
            strcat(nameOut, spiOutDir);
            
            strcat(nameOut, strrchr(file.name(), '/')+1);
            Serial.printf("    Copy %s to %s ... ", nameIn, nameOut);
            File fileIn = SD.open(nameIn, "rb");
            File fileOut = FFat.open(nameOut, "wb");
            do {
                l = fileIn.readBytes(buff, 256);
                s += l;
                if (l > 0)
                    fileOut.write((uint8_t *)buff, l);
            } while (l > 0);
            fileOut.close();
            fileIn.close(); 
            Serial.printf("%d bytes written\n", s);
		}
		file = root.openNextFile();
	}
}

#endif