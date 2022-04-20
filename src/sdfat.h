
#include "sdfat_d.h"

#undef THIS 
#define THIS System.SdFat

void sdFatInit() {
	THIS.spi = SPIClass(VSPI);
	THIS.spi.begin(SD_PIN_NUM_CLK, SD_PIN_NUM_MISO, SD_PIN_NUM_MOSI, SD_PIN_NUM_CS);
/*	if (!SD.begin(SD_PIN_NUM_CS, THIS.spi, 40000000, "/sd")) {
		Serial.println("Card Mount failed");
		THIS.mount = false;
	} else {
		Serial.println("Card Mount done");
		THIS.mount= true;
		// listDir(SD, "/Oric/tapes", 3);
	} */
}

bool sdFatMount() {
	if (!SD.begin(SD_PIN_NUM_CS, THIS.spi, 40000000, "/sd")) {
		Serial.println("Card Mount failed");
		THIS.mount = false;
	} else {
		Serial.println("Card Mount done");
		THIS.mount= true;
		// listDir(SD, "/Oric/tapes", 3);
	}
	return THIS.mount;
}

void sdFatUnMount() {
	if (THIS.mount) {
		SD.end();
		THIS.mount = false;
	}
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
	Serial.printf("Listing directory: %s\n", dirname);

	File root = fs.open(dirname);
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
			if (levels) {
				listDir(fs, file.name(), levels - 1);
			}
		} else {
			Serial.print("  FILE: ");
			Serial.print(file.name());
			Serial.print("  SIZE: ");
			Serial.println(file.size());
		}
		file = root.openNextFile();
	}
}
