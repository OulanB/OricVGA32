/*
 * for osd 
 */

#include "system_d.h"

#undef THIS 
#define THIS System.Osd

#undef TAPE 
#define TAPE System.Tape

#define OSD_INK     0
#define OSD_PAPER  16
#define OSD_BLINK  12
#define OSD_BLACK   0
#define OSD_RED     1
#define OSD_GREEN   2
#define OSD_YELLOW  3
#define OSD_BLUE    4
#define OSD_MAGENTA 5
#define OSD_CYAN    6
#define OSD_WHITE   7

#define OSD_FYTOP 2
#define OSD_FYBOTTOM 26
#define OSD_FPAPER OSD_MAGENTA
#define OSD_FINK OSD_YELLOW

#define OSD_STATE_TAPE 100

#define OSD_STATE_BOX_YES_NO 900

const WORD __modeState[] = {100, 200, 300, 400};

const char *__osdStatusTop[] = {
    "Delete|spiFs|Mnt|maKdir|         |Quik|-",         // L R N D
    "Disk:xxxxxxxxxxxxxxxxxxxxxxxxxxxx       ",
    "Cpu:                                    ",
    "Mem:                                    "
};
const char *__osdStatusBottom[] = {
    "Save|Home|<-|->|End|Cut|neW|Ren|       -",         // S B P N E C
    "|Eject|Save|Load|                       ",
    "|Step|                                  ",
    "|Mem|                                   "
};

void osdPutScancode(BYTE code) {
    THIS.bufScan[THIS.bufScanHigh++] = code;
    THIS.bufScanHigh &= 0x1F;
}

bool _osdIsScancode() {
    return THIS.bufScanHigh != THIS.bufScanLow;
}

BYTE _osdGetScancode() {
    BYTE code = 0xFF;
    if (THIS.bufScanHigh != THIS.bufScanLow) {
        code = THIS.bufScan[THIS.bufScanLow++];
        THIS.bufScanLow &= 0x1F;
    }
    return code;
}

void osdInit(const char *tapePath) {
    strcpy(THIS.tapePath, tapePath);
    THIS.fileSys = OSD_SPIFAT;
    THIS.mount = FALSE;
    THIS.display = 0;
    THIS.mode = 0;
    THIS.state = 0;
    THIS.prevState = -1;
    //THIS.substate = OSD_STATE_TAPE;        // TAPE
    THIS.refresh = TRUE;
    THIS.top = System.Tape.topLine;
    THIS.bottom = System.Tape.bottomLine;

    memset(THIS.noTopBottom, 32, 41);
    THIS.noTopBottom[0] = 0x00;

    _osdCls(OSD_YELLOW, OSD_MAGENTA);
/*
    THIS.fdRoot = new fs::File();
    THIS.fdIn = new fs::File();
    THIS.fdOut = new fs::File(); */
    THIS.maxfiles = 200;
    THIS.nfiles = 0;
    THIS.files = (OSDFILE *) ps_malloc(sizeof(OSDFILE) * THIS.maxfiles);
}

void _osdUnMountFs() {
    if (THIS.mount) {
        if (THIS.fileSys == OSD_SDFAT) {            // SD fs
            sdFatUnMount();
            THIS.mount = FALSE;
        } else if (THIS.fileSys == OSD_SPIFAT) {
            spiFatUnMount();
            THIS.mount = FALSE;
        }
    }
}

BOOL _osdMountFs() {
    if (THIS.mount) {
        _osdUnMountFs();
    } 
    if (!THIS.mount) {
        if (THIS.fileSys == OSD_SDFAT) {            // SD fs
            sdFatMount();
            THIS.mount = TRUE;
        } else if (THIS.fileSys == OSD_SPIFAT) {
            spiFatMount();
            THIS.mount = TRUE;
        }
    }
    return THIS.mount;
}

BOOL _osdTapeExist(const char *path) {
    strcpy(THIS.tmpName, path);
    if (strlen(path) > 1) {
        strcat(THIS.tmpName, "/");
    }
    strcat(THIS.tmpName, TAPE.name);
    if (THIS.fileSys == OSD_SPIFAT) {
        return FFat.exists(THIS.tmpName);
    } else if (THIS.fileSys == OSD_SDFAT)  {
        return SD.exists(THIS.tmpName);
    }
}

BOOL _osdFileDelete(const char *path, const char *name) {
    strcpy(THIS.tmpName, path);
    if (strlen(path) > 1) {
        strcat(THIS.tmpName, "/");
    }
    strcat(THIS.tmpName, name);
    if (THIS.fileSys == OSD_SPIFAT) {
        return FFat.remove(THIS.tmpName);
    } else if (THIS.fileSys == OSD_SDFAT)  {
        return SD.remove(THIS.tmpName);
    }
}

BOOL _osdDirDelete(const char *path, const char *name) {
    strcpy(THIS.tmpName, path);
    if (strlen(path) > 1) {
        strcat(THIS.tmpName, "/");
    }
    strcat(THIS.tmpName, name);
    if (THIS.fileSys == OSD_SPIFAT) {
        return FFat.rmdir(THIS.tmpName);
    } else if (THIS.fileSys == OSD_SDFAT)  {
        return SD.rmdir(THIS.tmpName);
    }
}

BOOL _osdTapeSave(const char *path) {
    if (THIS.fileSys == OSD_SPIFAT) {
        return tapeSaveSPI(path);
    } else if (THIS.fileSys == OSD_SDFAT)  {
        return tapeSaveSD(path);
    }
    return FALSE;
}

BOOL _osdTapeLoad(const char *name) {
    if (THIS.fileSys == OSD_SPIFAT) {
        return tapeLoadSPI(name);
    } else if (THIS.fileSys == OSD_SDFAT)  {
        return tapeLoadSD(name);
    }
    return FALSE;
}

void _osdCls(BYTE paper, BYTE ink) {
    THIS.ink = ink & 7;
    THIS.paper = paper & 7;
    BYTE *line = THIS.screen; 
    for (int i = 0; i < 28; i++) {
        *line++ = (THIS.paper << 4) | THIS.ink;
        memset(line, 32, 40);
        line += 40;
    }
}

void _osdClsFrame() {
    THIS.paper = OSD_FPAPER;
    THIS.ink = OSD_FINK;
    BYTE *line = THIS.screen + 41*OSD_FYTOP; 
    for (int i = OSD_FYTOP; i <= OSD_FYBOTTOM; i++) {
        *line++ = (OSD_FPAPER << 4) | OSD_FINK;
        memset(line, 32, 40);
        line += 40;
    }
}

void _osdPutPaper(BYTE x, BYTE y, BYTE paper) {
    BYTE *line = THIS.screen + 1 + x + 41*y; 
    *line = OSD_PAPER | paper;
}
void _osdPutInk(BYTE x, BYTE y, BYTE ink) {
    BYTE *line = THIS.screen + 1 + x + 41*y; 
    *line = OSD_INK | ink;
}

void _osdLineColor(BYTE y, BYTE paper, BYTE ink) {
    BYTE *line = THIS.screen + 41*y; 
    *line = (paper << 4) | ink;
}

void _osdLineColorReverse(BYTE y) {
    BYTE *line = THIS.screen + 41*y; 
    *line = ((*line & 0x0F) << 4) | ((*line >> 4) & 0x0F);
}

void _osdScroll() {
    BYTE *dest = THIS.screen + (OSD_FYTOP+1)*41;
    BYTE *src = dest - 41;
    for (int y=OSD_FYTOP+1; y <= OSD_FYBOTTOM; y++) {
        memcpy(dest, src, 41);
        dest += 41;
        src += 41;
    }    
    *dest++ = (THIS.paper << 4) | THIS.ink;
    memset(dest, 32, 40);
}

void _osdPrint(const char *text) {
    BYTE *line = THIS.screen + THIS.locX+1 + THIS.locY*41;
    BYTE c;
    do {
        c = *text++;
        if (c != 0) {
            *line++ = c;
            THIS.locX++;
            if (THIS.locX == 40) {
                THIS.locX = 0;
                THIS.locY++;
                if (THIS.locY == (OSD_FYBOTTOM+1)) {
                    THIS.locY--;
                    _osdScroll();
                }
                line = THIS.screen + THIS.locX+1 + THIS.locY*41;
            }
        }
    } while (c);
}

void _osdPrintAt(BYTE x, BYTE y, const char *text) {
    THIS.locX = x;
    THIS.locY = y;
    _osdPrint(text);
}

void _osdPrintLineAt(BYTE x, BYTE y, const char *text) {
    BYTE *line = THIS.screen + x+1 + y*41;
    BYTE c;
    do {
        c = *text;
        if (c != 0) {
            text++;
        } else {
            c = 32;
        }
        *line++ = c;
        x++;
    } while (x < 40);
}

void _osdRefreshBase() {
    _osdCls(OSD_MAGENTA, OSD_WHITE);
    _osdPrintAt(0, OSD_FYTOP-2, __osdStatusTop[THIS.mode]);
    _osdPrintAt(34, OSD_FYTOP-2, System.quickTape ? "Quik" : "/Qik"); 
    _osdPrintAt(7, OSD_FYTOP-2, THIS.fileSys == OSD_SDFAT ? " sdFs" : "spiFs"); 
    _osdPrintAt(0, OSD_FYBOTTOM+1, __osdStatusBottom[THIS.mode]);
    _osdPrintAt(0, OSD_FYTOP-1, "Path:"); 
    _osdPrintLineAt(5, OSD_FYTOP-1, THIS.tapePath); 
}

static int _compareOsdFile (void const *a, void const *b) {
    OSDFILE *pa = (OSDFILE *)a;
    OSDFILE *pb = (OSDFILE *)b;
    if (pa->dir) {                              // a is dir
        if (pb->dir) {                              // b is dir
            return strcasecmp(pa->name, pb->name);          // 2 dirs to compare
        } else {                                    // b is file
            return -1;                                  // a is first
        }
    } else {                                    // a is file
        if (pb->dir) {                              // b is dir
            return 1;                                   // b is first
        } else {                                    // b is file
            return strcasecmp(pa->name, pb->name);          // 2 files to compare
        }
    } 
}

void osdFsm() {
    THIS.prevState = THIS.state;
    BYTE code = _osdIsScancode() ? _osdGetScancode() : 0;
    if (code == 140) {          // F12 ?
        THIS.display = 0;
    }
    BOOL ok;
    switch(THIS.state) {
        case 0:         // entry STATE
            if (THIS.display) {
                _osdRefreshBase();
                THIS.state = 1;
            }
            break;
        case 1:         // wait state
            THIS.state = __modeState[THIS.mode];
            break;
        case OSD_STATE_TAPE:            // TAPE
            if (THIS.refresh) {
                THIS.refresh = 0;
                THIS.state = OSD_STATE_TAPE + 90;
            } else {
                _osdPrintAt(39, OSD_FYTOP-2,"-"); 
                THIS.state++;
            }
            break;
        case OSD_STATE_TAPE+1:              // main loop 
            if (code == 152 || code == 154) {                  // UP
                int loop = (code == 152) ? 1 : 12;
                do {
                    if (THIS.selectLine > OSD_FYTOP) {
                        THIS.selectLine--;
                        THIS.state = OSD_STATE_TAPE+80;
                    } else if (THIS.topFile > 0) {
                        THIS.topFile--;
                        THIS.state = OSD_STATE_TAPE+80;
                    }
                } while (--loop);
            } else if (code == 153 || code == 155) {           // DOWN
                int loop = (code == 153) ? 1 : 12;
                do {
                    if (THIS.selectLine < OSD_FYBOTTOM) {
                        if ((THIS.selectLine - OSD_FYTOP) + THIS.topFile < THIS.nfiles) {
                            THIS.selectLine++;
                            THIS.state = OSD_STATE_TAPE+80;
                        }                    
                    } else if (THIS.topFile + (OSD_FYBOTTOM - OSD_FYTOP + 1) < THIS.nfiles) {
                        THIS.topFile++;
                        THIS.state = OSD_STATE_TAPE+80; 
                    }
                } while (--loop);
            } else if (code ==8) {                             // backspace Up dir
                if (THIS.mount) {
                    if (strlen(THIS.tapePath) > 1) {                        // can go up
                        strcpy(THIS.tmpName, THIS.tapePath);
                        THIS.tmpName[strrchr(THIS.tmpName, '/') - THIS.tmpName] = 0x00;
                        if (strlen(THIS.tmpName) == 0) {
                            strcat(THIS.tmpName, "/");
                        }
                        // Serial.printf("Want to chdir %s\n", THIS.tmpName);
                        strcpy(THIS.tapePath, THIS.tmpName);
                        THIS.state = OSD_STATE_TAPE;
                        THIS.refresh = TRUE;
                    }
                }
            } else if (code == 13) {                            // Enter -> Load
                if (THIS.mount && THIS.nfiles > 0) {
                    strcpy(THIS.tmpName, THIS.tapePath);
                    THIS.selectFile = (THIS.selectLine - OSD_FYTOP) + THIS.topFile;
                    if (THIS.files[THIS.selectFile].dir) {          // up dir
                        if (THIS.tmpName[strlen(THIS.tmpName)-1] != '/') {
                            strcat(THIS.tmpName, "/");
                        }
                        strcat(THIS.tmpName, THIS.files[THIS.selectFile].name);
                        // Serial.printf("Want to chdir %s\n", THIS.tmpName);
                        strcpy(THIS.tapePath, THIS.tmpName);
                        THIS.state = OSD_STATE_TAPE;
                        THIS.refresh = TRUE;
                    } else {                                        // load file
                        THIS.state = OSD_STATE_TAPE+10;
                    }
                }
            } else if (code == 'F' || code == 'f') {            // Filesys Spi or Sd
                if (!THIS.mount) {
                    if (THIS.fileSys == OSD_SDFAT) {
                        THIS.fileSys = OSD_SPIFAT;                           // SPI
                        _osdPrintAt(7, OSD_FYTOP-2, "spi"); 
                    } else {
                        THIS.fileSys = OSD_SDFAT;                            // SD
                        _osdPrintAt(7, OSD_FYTOP-2, " sd"); 
                    }
                }
            } else if (code == 'M' || code == 'm') {            // Mount Unmount
                if (THIS.mount) {
                    _osdUnMountFs();
                    strcpy(THIS.tapePath, "no FS mounted");
                } else {
                    if (_osdMountFs()) {
                        strcpy(THIS.tapePath, "/");
                    } else {
                    strcpy(THIS.tapePath, "no FS mounted");
                    }
                }
                THIS.state = OSD_STATE_TAPE;
                THIS.refresh = TRUE;
            } else if (code == 'Q' || code == 'q') {            // Quick / Norm
                systemTapeSpeed(1 - System.quickTape);
                _osdPrintAt(34, OSD_FYTOP-2, System.quickTape ? "Quik" : "/Qik"); 
            } else if (code == 150) {                           // Prev tape part
                tapePrev();
            } else if (code == 151) {                           // Next tape part
                tapeNext();
            } else if (code == 156) {                           // Home : Begin tape 
                tapeBegin();
            } else if (code == 157) {                           // End : End tape
                tapeEnd();
            } else if (code == 'C' || code == 'c') {                // Cut part from tape
                if (TAPE.nparts > 0 && TAPE.part < TAPE.nparts) {
                    THIS.state = OSD_STATE_TAPE+70;
                }
            } else if (code == 'W' || code == 'w') {                // New tape
                THIS.state = OSD_STATE_TAPE+60;
            } else if (code == 'R' || code == 'r') {                // Rename tape
                THIS.retNoState = OSD_STATE_TAPE+1;
                THIS.retYesState = OSD_STATE_TAPE+1;
                THIS.state = OSD_STATE_TAPE+30;
            } else if (code == 'S' || code == 's') {                // Save tape
                if (THIS.mount) {
                    THIS.state = OSD_STATE_TAPE+20;
                }
            } else if (code == 127 || code == 'd' ||code == 'D') {  // delete file ?
                if (THIS.mount) {
                    THIS.state = OSD_STATE_TAPE+40;
                }
            }
            break;
// Load tape
        case OSD_STATE_TAPE+10:             // Load tape
            if (TAPE.dataChanged) {
                strcpy(THIS.message1, "Do you really want to loose");
                strcpy(THIS.message2, "the current tape ?");
                THIS.retNoState = OSD_STATE_TAPE+1;
                THIS.retYesState = OSD_STATE_TAPE+11;
                THIS.state = OSD_STATE_BOX_YES_NO;
            } else {
                THIS.state++;
            }
            break;
        case OSD_STATE_TAPE+11:
            if (strlen(THIS.tmpName) != 1) {
                strcat(THIS.tmpName, "/");
            }
            strcat(THIS.tmpName, THIS.files[THIS.selectFile].name); 
            // Serial.printf("Want to load %s\n", THIS.tmpName);
            _osdTapeLoad(THIS.tmpName);
            THIS.state = OSD_STATE_TAPE+1;
            break;
// Save tape
        case OSD_STATE_TAPE+20:             // Save tape
            if (_osdTapeExist(THIS.tapePath)) {
                strcpy(THIS.message1, "Overwrite file ?");
                strncpy(THIS.message2, TAPE.name, 34);
                THIS.message2[34] = 0x00;
                THIS.retNoState = OSD_STATE_TAPE+1;
                THIS.retYesState = OSD_STATE_TAPE+21;
                THIS.state = OSD_STATE_BOX_YES_NO;
            } else {
                THIS.state++;
            }
            break;
        case OSD_STATE_TAPE+21:
            _osdTapeSave(THIS.tapePath);
            THIS.state = OSD_STATE_TAPE;
            THIS.refresh = TRUE;
            break;
// Rename tape
        case OSD_STATE_TAPE+30:             // rename tape
            THIS.tmpName[0] = 0x00;
            THIS.tmpNameLen = 0;
            THIS.tmpCounter = 0;
            THIS.state++;
            break;
        case OSD_STATE_TAPE+31: 
            THIS.tmpCounter++;
            TAPE.topLine[3+THIS.tmpNameLen] = 32 + ((THIS.tmpCounter & 32) ? 0 : 128);
            if (code == 13) {           // enter -> ok
                size_t len = strlen(THIS.tmpName);
                THIS.tmpName[THIS.tmpNameLen] = 0x00;
                if (len >= 4) {
                    if (strcasecmp(".TAP", THIS.tmpName + len - 4) != 0) {
                        strcat(THIS.tmpName, ".tap");
                    }
                }
                strcpy(TAPE.name, THIS.tmpName);
                _tapeRajInfoTape();
                THIS.state = THIS.retYesState;
            } else if (code == 8) {     // backspace
                if (THIS.tmpNameLen > 0) {
                    TAPE.topLine[3+THIS.tmpNameLen] = 0x20;
                    THIS.tmpNameLen--;
                    THIS.tmpName[THIS.tmpNameLen] = 0x00;
                    THIS.tmpCounter = 0;
                }
            } else if (code == 27) {    // escape -> cancel
                _tapeRajInfoTape();
                THIS.state = THIS.retNoState;
            } else if (code > 32 && code < 127 && code != '/') {  // good char
                if (THIS.tmpNameLen < 30) {
                    THIS.tmpName[THIS.tmpNameLen] = code;
                    TAPE.topLine[3+THIS.tmpNameLen] = code;
                    THIS.tmpNameLen++;
                    THIS.tmpCounter = 0;
                }
            } break;
// delete file or dir ?
        case OSD_STATE_TAPE+40:             // delete file or dir ?
            THIS.selectFile = (THIS.selectLine - OSD_FYTOP) + THIS.topFile;
            if (THIS.files[THIS.selectFile].dir) {      // delete dir ?
                strcpy(THIS.message1, "Really delete the dir (empty?)");
            } else {
                strcpy(THIS.message1, "Really delete the file");                
            }
            strncpy(THIS.message2, THIS.files[THIS.selectFile].name, 34);
            THIS.message2[34] = 0x00;
            THIS.retNoState = OSD_STATE_TAPE+1;
            THIS.retYesState = OSD_STATE_TAPE+41;
            THIS.state = OSD_STATE_BOX_YES_NO;
            break;
        case OSD_STATE_TAPE+41:
            if (THIS.files[THIS.selectFile].dir) {    
                if (_osdDirDelete(THIS.tapePath, THIS.files[THIS.selectFile].name)) {
                    THIS.state++;
                } else {
                    THIS.state = OSD_STATE_TAPE+1;
                }
            } else {
                if (_osdFileDelete(THIS.tapePath, THIS.files[THIS.selectFile].name)) {
                    THIS.state++;
                } else {
                    THIS.state = OSD_STATE_TAPE+1;
                }
            }
            break;
        case OSD_STATE_TAPE+42: 
            if (THIS.selectFile == THIS.nfiles-1) {
                if (THIS.nfiles > 1) {
                    THIS.selectLine--;
                }
            } else {
                int i = THIS.selectFile;
                do {
                    THIS.files[i].dir = THIS.files[i+1].dir;
                    THIS.files[i].size = THIS.files[i+1].size;
                    strcpy(THIS.files[i].name, THIS.files[i+1].name);
                    i++;
                } while (i < THIS.nfiles-1); 
            }
            THIS.nfiles--;
            THIS.state = OSD_STATE_TAPE+80;
            break;
// make a new tape
        case OSD_STATE_TAPE+60:             // New tape
            if (TAPE.dataChanged) {
                strcpy(THIS.message1, "Do you really want to loose");
                strcpy(THIS.message2, "the current tape ?");
                THIS.retNoState = OSD_STATE_TAPE+1;
                THIS.retYesState = OSD_STATE_TAPE+61;
                THIS.state = OSD_STATE_BOX_YES_NO;
            } else {
                THIS.state++;
            }
            break;
        case OSD_STATE_TAPE+61:
            tapeEmptyTape();
            THIS.state = OSD_STATE_TAPE+1;
            break;
// cut part from tape
        case OSD_STATE_TAPE+70:             // Cut part from tape
            strcpy(THIS.message1, "Do you really want to Cut");
            sprintf(THIS.message2, "Part:%d <<%s>> ?", TAPE.part, TAPE.parts[TAPE.part].name);
            THIS.retNoState = OSD_STATE_TAPE+1;
            THIS.retYesState = OSD_STATE_TAPE+71;
            THIS.state = OSD_STATE_BOX_YES_NO;
            break;
        case OSD_STATE_TAPE+71:
            tapeDeleteFromTape();
            THIS.state = OSD_STATE_TAPE+1;
            break;
// refresh list of files
        case OSD_STATE_TAPE+80:             // display fill at 
            THIS.i = THIS.topFile;
            THIS.y = OSD_FYTOP;
            THIS.state++;
            break;
        case OSD_STATE_TAPE+81:
            do {
                if (THIS.i < THIS.nfiles) {
                    if (THIS.y == THIS.selectLine) {
                        _osdLineColor(THIS.y, OSD_FINK, OSD_FPAPER);
                    } else {
                        _osdLineColor(THIS.y, OSD_FPAPER, OSD_FINK);
                    }
                    _osdPrintLineAt(1, THIS.y, THIS.files[THIS.i].name);
                    _osdPrintAt(0, THIS.y, THIS.files[THIS.i].dir ? ">" : " ");  
                    THIS.i++;
                } else {
                    _osdLineColor(THIS.y, OSD_FPAPER, OSD_FINK);
                    _osdPrintLineAt(0, THIS.y, "                                         ");
                }
                THIS.y++;
                if (THIS.y > OSD_FYBOTTOM) {
                    THIS.state = OSD_STATE_TAPE;
                    break;
                }
            }  while (1); // (THIS.y % 9) != 0);
            break;
// reload files list from dir
        case OSD_STATE_TAPE+90:             // refresh start
            _osdClsFrame();
            _osdPrintLineAt(5, OSD_FYTOP-1, THIS.tapePath); 
            _osdPrintAt(39, OSD_FYTOP-2,"*"); 
            if (THIS.mount) {
                THIS.state++;
            } else {
                THIS.nfiles = 0;
                THIS.selectLine = OSD_FYTOP;                // first line selected
                THIS.topFile = 0;                           // offset 0
                THIS.state = OSD_STATE_TAPE+80;             // display all
            }
            break;
        case OSD_STATE_TAPE+91:                 // open the root ?    
            if (THIS.fileSys == OSD_SDFAT) {
                THIS.fdRoot = SD.open(THIS.tapePath, "rb");
            } else if (THIS.fileSys == OSD_SPIFAT) {
                THIS.fdRoot = FFat.open(THIS.tapePath, "rb");
            } else {
                THIS.state = OSD_STATE_TAPE;
            }
            if (THIS.fdRoot) {
                if (THIS.fdRoot.isDirectory()) {
                    THIS.tooMuchFiles = FALSE;
                    THIS.nfiles = 0;
                    THIS.state++;
                } else {
                    THIS.state = OSD_STATE_TAPE;
                }
            } else {
                THIS.state = OSD_STATE_TAPE;
            }
            break;
        case OSD_STATE_TAPE+92:                 // next entry
            ok = TRUE;
            while (ok) {
                BOOL good = FALSE;
                THIS.fdIn = THIS.fdRoot.openNextFile();
                if (THIS.fdIn) {
                    // Serial.printf(" .. got %s\n", THIS.fdIn.name());
                    strcpy(THIS.files[THIS.nfiles].name, strrchr(THIS.fdIn.name(), '/') + 1);
                    if (THIS.fdIn.isDirectory()) {
                        // Serial.printf("dir : %s\n", THIS.fdIn.name());
                        THIS.files[THIS.nfiles].dir = TRUE;
                        THIS.files[THIS.nfiles].size = 0;
                        good = TRUE;
                    } else {
                        size_t l = strlen(THIS.files[THIS.nfiles].name);
                        if ( l > 4) {
                            if (strcasecmp(".TAP", THIS.files[THIS.nfiles].name + l - 4) == 0) {
                                THIS.files[THIS.nfiles].dir = FALSE;
                                THIS.files[THIS.nfiles].size = THIS.fdIn.size();
                                good = TRUE;
                            }
                        }
                    }
                    if (good) {
                        // Serial.printf("%02d:%c %s: %d\n", THIS.nfiles, THIS.files[THIS.nfiles].dir ? '>' : ' ', THIS.files[THIS.nfiles].name, THIS.files[THIS.nfiles].size);
                        THIS.nfiles++;
                    }
                    if ((THIS.nfiles % 30) == 0) {  // 30 files done ... 
                        ok = FALSE;                     // leave for now
                    }
                    if (THIS.nfiles >= THIS.maxfiles) {     // too much files
                        THIS.tooMuchFiles = TRUE;
                        ok = FALSE;
                        THIS.state++;
                    }
                    THIS.fdIn.close();
                } else {
                    THIS.files[THIS.nfiles].size = -1;  // no more files
                    ok = FALSE;
                    THIS.state++;
                }
            }
            break;
        case OSD_STATE_TAPE+93:                 // all entries read
            THIS.fdRoot.close();
            qsort(THIS.files, THIS.nfiles, sizeof(OSDFILE), &_compareOsdFile);
#ifdef OSD_DUMP_LIST_SERIAL
            THIS.i = 0;
            THIS.state++;
#else
            THIS.selectLine = OSD_FYTOP;                // first line selected
            THIS.topFile = 0;                           // offset 0
            THIS.state = OSD_STATE_TAPE+80;             // display all
#endif
            break;
        case OSD_STATE_TAPE+94:                 // just dump on serial ...
            ok = TRUE;
            while(ok) {
                if (THIS.i < THIS.nfiles) {
                    Serial.printf("%02d:%c %s: %d\n", THIS.i, THIS.files[THIS.i].dir ? '>' : ' ', THIS.files[THIS.i].name, THIS.files[THIS.i].size);
                    THIS.i++;
                    if ((THIS.i % 10) == 0) {
                        ok = FALSE;
                    }
                } else {
                    THIS.selectLine = OSD_FYTOP;                // first line selected
                    THIS.topFile = 0;                           // offset 0
                    THIS.state = OSD_STATE_TAPE+80;             // display all
                    ok = FALSE;
                }
            }
            break;
// utils
        case OSD_STATE_BOX_YES_NO:
            memcpy(THIS.saveMsg1, THIS.screen+12*41, 41);
            memcpy(THIS.saveMsg2, THIS.screen+13*41, 41);
            memcpy(THIS.saveMsg3, THIS.screen+14*41, 41);
//            memcpy(THIS.saveMsg4, THIS.screen+15*41, 41);
            THIS.state++;
            break;
        case OSD_STATE_BOX_YES_NO+1: {
            size_t len1 = strlen(THIS.message1);
            size_t len2 = strlen(THIS.message2);
            size_t len = (len1 > len2) ? len1 : len2;
            if (len < 12) len = 12;
            BYTE left = 20 - len/2;
            BYTE right = left + len;
            BYTE i;
            while (len1 <= len) {
                THIS.message1[len1++] = 32;
            }
            while (len2 <= len) {
                THIS.message2[len2++] = 32;
            }
            // line 1
            _osdPutPaper(left-2, 12, OSD_WHITE);
            _osdPutInk(left-1, 12, OSD_BLACK);
            _osdPrintAt(left, 12, THIS.message1);
            _osdPutInk(right+1, 12, (THIS.selectLine != 12) ? OSD_FPAPER : OSD_FINK);
            _osdPutPaper(right+2, 12, (THIS.selectLine != 12) ? OSD_FINK : OSD_FPAPER);
            // line 2
            _osdPutPaper(left-2, 13, OSD_WHITE);
            _osdPutInk(left-1, 13, OSD_BLACK);
            _osdPrintAt(left, 13, THIS.message2);
            _osdPutInk(right+1, 13, (THIS.selectLine != 13) ? OSD_FPAPER : OSD_FINK);
            _osdPutPaper(right+2, 13, (THIS.selectLine != 13) ? OSD_FINK : OSD_FPAPER);
            // line 3
            _osdPutPaper(left-2, 14, OSD_RED);
            _osdPutInk(left-1, 14, OSD_WHITE);
            _osdPrintAt(left, 14, "NO  ");
            _osdPutPaper(left+4, 14, OSD_WHITE);
            i = left + 5;
            while (i < right-4) {
                _osdPrintAt(i++, 14, " ");
            }
            _osdPutPaper(right-5, 14, OSD_GREEN);
            _osdPrintAt(right-4, 14, " YES ");
            _osdPutInk(right+1, 14, (THIS.selectLine != 14) ? OSD_FPAPER : OSD_FINK);
            _osdPutPaper(right+2, 14, (THIS.selectLine != 14) ? OSD_FINK : OSD_FPAPER);
            THIS.state++;
            } break;
        case OSD_STATE_BOX_YES_NO+2: 
            ok = FALSE;
            if (code == 13 || code == 'y' || code == 'Y') {           // return OK
                ok = TRUE;
                THIS.state = THIS.retYesState;
            } else if (code == 27 ||code == 'n' || code == 'N') {     // esc Cancel
                ok = TRUE;
                THIS.state = THIS.retNoState;
            }
            if (ok) {
                memcpy(THIS.screen+12*41, THIS.saveMsg1, 41);
                memcpy(THIS.screen+13*41, THIS.saveMsg2, 41);
                memcpy(THIS.screen+14*41, THIS.saveMsg3, 41);
            }
            break;
    }
    if (THIS.state != THIS.prevState) {
        Serial.printf("State : %d -> %d (%d %d/%d)\n", THIS.prevState, THIS.state, THIS.selectLine, THIS.topFile, THIS.nfiles);
    }
}