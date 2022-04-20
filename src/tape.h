/*
 * stuff for tape operations ...
 *
 * top display T:name          
 * 
 * 
 * 
 * 
 * (c)2013-2022 ods
 */

#include "tape_d.h"

#undef THIS
#define THIS System.Tape

#define DEBUGTAPE

void _tapeRajInfoPartPercent() {
    if (THIS.loaded) {
        if (THIS.part < THIS.nparts) {
            int delta = THIS.parts[THIS.part+1].ptr - THIS.parts[THIS.part].ptr;
            if (delta > 0) {
                THIS.lastPercent = (100 * (THIS.ptr - THIS.parts[THIS.part].ptr))/delta;
            } else {
                THIS.lastPercent = 0;
            }
        } else {
            THIS.lastPercent = 0;
        }
        if (THIS.lastPercent < 0) THIS.lastPercent = 0;
        if (THIS.lastPercent > 99) THIS.lastPercent = 99;
        THIS.bottomLine[4] = 48 + (THIS.lastPercent/10);
        THIS.bottomLine[5] = 48 + (THIS.lastPercent%10);
    } else {
        THIS.bottomLine[4] = '_';
        THIS.bottomLine[5] = '_';
    }
}

void _tapeRajInfoPart() {
    if (!THIS.loaded) {
        strcpy((char *)THIS.bottomLine+1, "__|__%|________________|____|____|___|_ ");
    } else {
        if (THIS.part < THIS.nparts) {
            int delta = THIS.parts[THIS.part+1].ptr - THIS.parts[THIS.part].ptr;
            if (delta > 0) {
                THIS.lastPercent = (100 * (THIS.ptr - THIS.parts[THIS.part].ptr))/delta;
            } else {
                THIS.lastPercent = 0;
            }
        } else {
            THIS.lastPercent = 0;
        }
        sprintf((char *)THIS.bottomLine+1, "%02d|__%%|%- 16.16s|%04X|%04X|%c%c%c|%c",
                THIS.part, 
                THIS.parts[THIS.part].name, THIS.parts[THIS.part].start, 
                THIS.parts[THIS.part].end, THIS.parts[THIS.part].autost ? 'A' : '_',
                THIS.parts[THIS.part].typeS[0], THIS.parts[THIS.part].bitImage ? 'b' : 'n',
                THIS.dataChanged ? '*' : '-');
    }
    _tapeRajInfoPartPercent();
}

void _tapeRajInfoTape() {
    if (!THIS.loaded) {
        strcpy((char *)THIS.topLine+1, "T:no tape inserted               |__|__%");
    } else {
        sprintf((char *)THIS.topLine+1, "T:%- 31.31s|%02d|%02d%%", THIS.name, THIS.nparts, (THIS.size*100) / THIS.length);
    }
    _tapeRajInfoPart();
}

BYTE _tapeLookDataByte(int ptr) {
    return (ptr < THIS.size) ? THIS.data[ptr] : 0;
}

WORD _tapeLookDataWordR(int ptr) {
    WORD data = (WORD)(_tapeLookDataByte(ptr++)) << 8;
    data |= (WORD)(_tapeLookDataByte(ptr));
    return data;
}

int _tapeFindHeader(int ptr) {
    int x16 = 0;
    BYTE data;
    while (ptr < THIS.size) {
        data = THIS.data[ptr++];
        if (data == 0x16) {
            x16++;
        } else if ((data == 0x24) && (x16 >= 3)) {
            return ptr;
        } else {
            x16 = 0;
        }
    }
    return 0;   // not found, at least is 4
}

void _tapeGetNameRam(char *name) {
    *name = 0x00;
    int i = 0;
    int j = 0;
    BYTE c = 0;
    if (System.atmos != 0) {		// Oric Atmos
        for (i = 0x27F, j = 0; i < 0x28F; i++, j++) {
            c = systemRead(i) & 0x7F;
            if (c == 0x00) break;
            *name++ = c;
        }
    } else {					// Oric 1
        for (i = 0x35, j = 0; i < 0x45; i++, j++) {
            c = systemRead(i) & 0x7F;
            if (c == 0x00) break;
            *name++ = c;
        }
    }
    if (j == 0) {
        *name++ = '_'; *name++ = '_'; *name++ ='_'; *name++ = '_';
        *name++ = '_'; *name++ = '_'; *name++ ='_'; *name++ = '_';
    }
    *name++ = 0x00;
}

char *_tapeGetNameData(int ptr, char *str) {
    BYTE c = 0;
    for (int j = 0; j < 17; j++) {
        c = _tapeLookDataByte(ptr++);
        *str++ = c;
        if (c == 0) {
            return str;
        }
    }
    *str++ = 0x00;
    return str;
}

void _tapeAnalyse() {
    int ptr = 0;
    int ptrE;
    THIS.nparts = 0;
    do {
        ptrE = ptr;
        if (ptr < THIS.size) {
            if (THIS.data[ptr] == 0x61) {           // good init for bit image
                if (THIS.nparts > 0) {
                    THIS.parts[THIS.nparts-1].last = ptr-1;
                }
                THIS.parts[THIS.nparts].bitImage = 1;
                ptr++;
                int size = (_tapeLookDataByte(ptr) << 16) + (_tapeLookDataByte(ptr+1) << 8) + _tapeLookDataByte(ptr+2);	// size
#ifdef DEBUGTAPE
                Serial.printf("found %d bytes\n", size);
#endif
                THIS.parts[THIS.nparts].ptr = ptrE;					// start of header
                THIS.parts[THIS.nparts].H7 = 0x00;
                THIS.parts[THIS.nparts].start = 0x0000;
                THIS.parts[THIS.nparts].end = ptrE + size + 4;		// size + 4 of header
                THIS.parts[THIS.nparts].last = ptrE + size + 3;		// size - 1 + 4 of header
                THIS.parts[THIS.nparts].autost = 0x00;
                THIS.parts[THIS.nparts].type = 0x00;					// good entry, 0x61, size
                THIS.parts[THIS.nparts].array1 = 0x00;
                THIS.parts[THIS.nparts].array2 = 0x00;
                strcpy(THIS.parts[THIS.nparts].name, "<<BitImage>>");
                THIS.nparts++;
                ptr += size + 3;
            } else if (THIS.data[ptr] == 0x16) {    // good init for byte image
                if (THIS.nparts > 0) {
                    THIS.parts[THIS.nparts-1].last = ptr-1;
                }
                ptr = _tapeFindHeader(ptr);
                if (ptr > 0) {	// got one
                    THIS.parts[THIS.nparts].bitImage = 0;
                    THIS.parts[THIS.nparts].ptr = ptr-4;		                // start of good sync x16 x16 x16 x24 {ptr}
                    THIS.parts[THIS.nparts].H7 = _tapeLookDataByte(ptr+8);
                    THIS.parts[THIS.nparts].start = _tapeLookDataWordR(ptr+6);
                    THIS.parts[THIS.nparts].end = _tapeLookDataWordR(ptr+4);
                    THIS.parts[THIS.nparts].autost = _tapeLookDataByte(ptr+3);
                    THIS.parts[THIS.nparts].type = _tapeLookDataByte(ptr+2);
                    THIS.parts[THIS.nparts].array1 = _tapeLookDataByte(ptr+1);
                    THIS.parts[THIS.nparts].array2 = _tapeLookDataByte(ptr+0);  // seem useless
                    if (THIS.parts[THIS.nparts].type == 0) {
                        THIS.parts[THIS.nparts].typeS[0] = 'B';		// BASIC
                    } else if ((THIS.parts[THIS.nparts].type & 0x80) != 0) {
                        THIS.parts[THIS.nparts].typeS[0] = 'C';		// CODE
                    } else if ((THIS.parts[THIS.nparts].type & 0x80) != 0) {
                        THIS.parts[THIS.nparts].typeS[0] = 'S';		// STRING
                    } else if ((THIS.parts[THIS.nparts].type & 0x80) != 0) {
                        THIS.parts[THIS.nparts].typeS[0] = 'I';		// INTEGER
                    } else {
                        THIS.parts[THIS.nparts].typeS[0] = 'R';		// REAL
                    }
                    ptr += 9;
                    char *end = _tapeGetNameData(ptr, THIS.parts[THIS.nparts].name);
                    ptr += end - THIS.parts[THIS.nparts].name;
#ifdef DEBUGTAPE
                    Serial.printf("%d < %s > S:%04X E: %04X A: %02X R: %02X %02X H7: %02X\n",
                                THIS.parts[THIS.nparts].ptr, THIS.parts[THIS.nparts].name,
                                THIS.parts[THIS.nparts].start,
                                THIS.parts[THIS.nparts].end,
                                THIS.parts[THIS.nparts].autost,
                                THIS.parts[THIS.nparts].array1,
                                THIS.parts[THIS.nparts].array2,
                                THIS.parts[THIS.nparts].H7);
#endif
                    ptr += THIS.parts[THIS.nparts].end - THIS.parts[THIS.nparts].start + 1;
                    THIS.nparts++;
                } else {
#ifdef DEBUGTAPE
                    Serial.println("seen a init header but not header ...");
#endif
                }
            } else {							// bad init, fill up to end
                if (THIS.nparts > 0) {
                    THIS.parts[THIS.nparts-1].last = ptr-1;
                }
                THIS.parts[THIS.nparts].bitImage = 1;
                THIS.parts[THIS.nparts].ptr = ptrE;		// start of no Header
                THIS.parts[THIS.nparts].H7 = 0x00;
                THIS.parts[THIS.nparts].start = 0x0000;
                THIS.parts[THIS.nparts].end = THIS.size; // up to end
                THIS.parts[THIS.nparts].last = THIS.size - 1;
                THIS.parts[THIS.nparts].autost = 0;
                THIS.parts[THIS.nparts].type = 0x01;     // bizarre one, no 0x61, no size
                THIS.parts[THIS.nparts].array1 = 0x00;
                THIS.parts[THIS.nparts].array2 = 0x00;
                strcpy(THIS.parts[THIS.nparts].name, "<<BI unknown 1>>");
#ifdef DEBUGTAPE
                Serial.printf("%d < %s > S:%04X E: %04X A: %02X R: %02X %02X H7: %02X\n",
                            THIS.parts[THIS.nparts].ptr, THIS.parts[THIS.nparts].name,
                            THIS.parts[THIS.nparts].start,
                            THIS.parts[THIS.nparts].end,
                            THIS.parts[THIS.nparts].autost,
                            THIS.parts[THIS.nparts].array1,
                            THIS.parts[THIS.nparts].array2,
                            THIS.parts[THIS.nparts].H7);
#endif
                THIS.nparts++;
                ptr = -1;							// end
            }
        } else {
            ptr = -1;
        }
    } while (ptr > 0);
    if (ptrE < THIS.size) {					// some data left ...
        if (THIS.nparts > 0) {
            THIS.parts[THIS.nparts-1].last = ptrE-1;
        }
        THIS.parts[THIS.nparts].bitImage = 1;
        THIS.parts[THIS.nparts].ptr = ptrE;		// start of no Header
        THIS.parts[THIS.nparts].H7 = 0x00;
        THIS.parts[THIS.nparts].start = 0x0000;
        THIS.parts[THIS.nparts].end = THIS.size; // up to end
        THIS.parts[THIS.nparts].autost = 0;
        THIS.parts[THIS.nparts].type = 0x01;     // bizarre one, no 0x61, no size
        THIS.parts[THIS.nparts].array1 = 0x00;
        THIS.parts[THIS.nparts].array2 = 0x00;
        strcpy(THIS.parts[THIS.nparts].name, "<<BI unknown 2>>");
#ifdef DEBUGTAPE
        Serial.printf("%d < %s > S:%04X E: %04X A: %02X R: %02X %02X H7: %02X\n",
                    THIS.parts[THIS.nparts].ptr, THIS.parts[THIS.nparts].name,
                    THIS.parts[THIS.nparts].start,
                    THIS.parts[THIS.nparts].end,
                    THIS.parts[THIS.nparts].autost,
                    THIS.parts[THIS.nparts].array1,
                    THIS.parts[THIS.nparts].array2,
                    THIS.parts[THIS.nparts].H7);
#endif
        THIS.nparts++;
        ptrE = THIS.size;
    }
    if (THIS.nparts > 0) {
        THIS.parts[THIS.nparts-1].last = ptrE-1;
    }
    THIS.parts[THIS.nparts].bitImage = 0;
    THIS.parts[THIS.nparts].ptr = THIS.size;
    THIS.parts[THIS.nparts].last = THIS.size;
    THIS.parts[THIS.nparts].H7 = 0x00;
    THIS.parts[THIS.nparts].start = 0x0000;
    THIS.parts[THIS.nparts].end = 0x0000; // up to end
    THIS.parts[THIS.nparts].autost = 0;
    THIS.parts[THIS.nparts].type = 0x00;
    THIS.parts[THIS.nparts].array1 = 0x00;
    THIS.parts[THIS.nparts].array2 = 0x00;
    strcpy(THIS.parts[THIS.nparts].name, "<< EOT >>");
#ifdef DEBUGTAPE
    Serial.printf("%d < %s > S:%04X E: %04X A: %02X R: %02X %02X H7: %02X\n",
                THIS.parts[THIS.nparts].ptr, THIS.parts[THIS.nparts].name,
                THIS.parts[THIS.nparts].start,
                THIS.parts[THIS.nparts].end,
                THIS.parts[THIS.nparts].autost,
                THIS.parts[THIS.nparts].array1,
                THIS.parts[THIS.nparts].array2,
                THIS.parts[THIS.nparts].H7);
#endif
    THIS.part = 0;

    if (THIS.part > THIS.nparts) {
        THIS.part = THIS.nparts;
    }
}

void _tapeUpdatePart(int ptr) {
    int j = 0;
    THIS.part = 0;
    for (j = 0; j < THIS.nparts; j++) {
        if (ptr == THIS.parts[j].ptr) {
            THIS.part = j;
            break;
        }
        if (ptr < THIS.parts[j].ptr) {
            THIS.part = j-1;
            // break;
        }
    }
    if (THIS.part < 0) THIS.part = 0;
    THIS.ptr = THIS.parts[THIS.part].ptr;
    Serial.printf("new part is %d at %d\n", THIS.part, THIS.ptr);
    _tapeRajInfoPart();
}

void tapeInitOp() {
    THIS.playing = 1;
    _tapeGetNameRam(THIS.partRamName);
#ifdef DEBUGTAPE
    Serial.printf("start of tape op for %s\n", THIS.partRamName);
#endif
    if (THIS.loaded == 0) {
        if (strlen(THIS.partRamName) == 0) {
            strcpy(THIS.name, "________.tap");
        } else {
            sprintf(THIS.name, "%s.tap", THIS.partRamName);
        }
        if (tapeLoadSPI(THIS.name) == 0) {		// try to load an existing one ...
            THIS.loaded = 1;		            // else create an empty one
            THIS.size = 0;						// just an EOT
            THIS.ptr = 0;
            THIS.part = 0;
            THIS.nparts = 0;
            THIS.parts[THIS.part].ptr = THIS.size;
            THIS.parts[THIS.part].H7 = 0x00;
            THIS.parts[THIS.part].start = 0x0000;
            THIS.parts[THIS.part].end = 0x0000;
            THIS.parts[THIS.part].autost = 0;
            THIS.parts[THIS.part].type = 0x00;
            THIS.parts[THIS.part].array1 = 0x00;
            THIS.parts[THIS.part].array2 = 0x00;
            strcpy(THIS.parts[THIS.part].name, "<< EOT >>");
            THIS.parts[THIS.part].bitImage = 0;
            _tapeRajInfoTape();
#ifdef DEBUGTAPE
            Serial.println("create empty tape");
#endif
        }
    }
    THIS.ptrBit = 0;
    THIS.ptrBitWr = 0;
    THIS.wrPtr = 0;				// for quick tape write
    THIS.quickRead = 0;
    THIS.quickWrite = 0;
    THIS.lastPercent = 0;
}

void _tapeLoadBitsBit(BYTE data) {
//    Serial.printf("new bit byte is %02X\n", data);
    THIS.bit[0] = data & 1;
    THIS.bit[1] = (data >> 1) & 1;
    THIS.bit[2] = (data >> 2) & 1;
    THIS.bit[3] = (data >> 3) & 1;
    THIS.bit[4] = (data >> 4) & 1;
    THIS.bit[5] = (data >> 5) & 1;
    THIS.bit[6] = (data >> 6) & 1;
    THIS.bit[7] = (data >> 7) & 1;
    THIS.ptrBit = 0;
}

void _tapeLoadBits(BYTE data) {
    THIS.bit[0] = 0;                          // start
    THIS.bit[1] = data & 1;
    THIS.bit[2] = (data >> 1) & 1;
    THIS.bit[3] = (data >> 2) & 1;
    THIS.bit[4] = (data >> 3) & 1;
    THIS.bit[5] = (data >> 4) & 1;
    THIS.bit[6] = (data >> 5) & 1;
    THIS.bit[7] = (data >> 6) & 1;
    THIS.bit[8] = (data >> 7) & 1;
    THIS.bit[9] = ((THIS.bit[1]+THIS.bit[2]+THIS.bit[3]+THIS.bit[4]+
                    THIS.bit[5]+THIS.bit[6]+THIS.bit[7]+THIS.bit[8]) & 1) ^ 1;
    THIS.bit[10] = 1;
    THIS.bit[11] = 1;
    THIS.bit[12] = 1;
    THIS.bit[13] = 1;
    THIS.ptrBit = 0;
}

BYTE tapeQuickSync() {
    THIS.bottomLine[40] = 0x12; // GREEN start reading
    THIS.quickRead = 1;
    if (THIS.loaded != 0) {
        if (THIS.part == THIS.nparts) { // at EOT, rewind
            THIS.part = 0;
        }
        if (THIS.ptr > THIS.parts[THIS.part].ptr) {     // in stream, skip to the next header

        } else {

        }
#ifdef DEBUGTAPE
        // Serial.println("quick sync");
#endif
        //Log.d(__D_TAG, String.format("quick sync %02X", __tapeData[__tapePtr]));
        return 0x16;
    } else {
        return 0x00;
    }
}

void tapeQuickWrite(BYTE data) {
    if (THIS.loaded != 0) {
        if (THIS.quickWrite == 0) {
            THIS.bottomLine[40] = 0x11; // RED start writing
            THIS.quickWrite = 1;
#ifdef DEBUGTAPE
            Serial.printf("quick write ");
#endif
        }
        if (THIS.wrPtr < 64*1024) {
            THIS.wrData[THIS.wrPtr++] = data;
#ifdef DEBUGTAPE
            Serial.printf("0x%02X, ", data);
            if ((THIS.wrPtr % 16) == 0) {
                Serial.println("");
            }
#endif
        }
    }
}

BYTE tapeQuickRead() {
    if (THIS.loaded != 0) {
        if (THIS.quickRead == 0) {
            THIS.bottomLine[40] = 0x12; // GREEN start reading
            THIS.quickRead = 1;
        }
#ifdef DEBUGTAPE
        // Serial.printf("quick read 0x%02X", THIS.data[THIS.ptr]);
#endif
//        if (THIS.ptr < THIS.size) {
            if ((THIS.ptr & 0x7) == 0) {
                _tapeRajInfoPartPercent();
            }

            if (THIS.ptr > THIS.parts[THIS.part].last) {	// reach end of part
#ifdef DEBUGTAPE
                Serial.printf("end of part %d -> ", THIS.part);
#endif
                if (THIS.part == THIS.nparts) {				// last one, sorry
#ifdef DEBUGTAPE
                    Serial.println("end of tape");  // rewind
#endif
                    THIS.part = 0;
                    THIS.ptr = 0;
                } else {
                    THIS.part++;
                }
#ifdef DEBUGTAPE
                Serial.printf("next part %d\n", THIS.part);
#endif
                THIS.ptr = THIS.parts[THIS.part].ptr;
                if (THIS.parts[THIS.part].bitImage != 0) {
                    Serial.println("bit image tape in QUICK mode ?!?");
                }
                _tapeRajInfoPart();
            }
            return THIS.data[THIS.ptr++];
 //       } else {
 //           return 0x16;
 //       }
    } else {
        return 0x00;
    }
}

BYTE tapeGetFromTape() {
    BYTE data = 0;
    if (THIS.loaded != 0) {     // tape loaded
        if (THIS.playing) {         // motor on ?
            if (THIS.ptrBit >= THIS.bitTape) {   // byte finished
                if (THIS.ptr > THIS.parts[THIS.part].last) {	// reach end of part
#ifdef DEBUGTAPE
                    Serial.printf("end of part %d -> ", THIS.part);
#endif
                    if (THIS.part == THIS.nparts) {				// last one, sorry
#ifdef DEBUGTAPE
                        Serial.println("end of tape");  // rewind
#endif
                        THIS.part = 0;
                        THIS.ptr = 0;
                    } else {
                        THIS.part++;
                    }
#ifdef DEBUGTAPE
                    Serial.printf("next part %d\n", THIS.part);
#endif
                    THIS.ptr = THIS.parts[THIS.part].ptr;
                    if (THIS.parts[THIS.part].bitImage != 0) {
                        if (THIS.parts[THIS.part].type == 0x00) {	// normal one,
                            THIS.ptr += 4;							    // skip header
                        }
                        THIS.bitTape = 8;
                    } else {
                        THIS.bitTape = 13;
                    }
                    _tapeRajInfoPart();
                }
                data = THIS.data[THIS.ptr++];
                if (THIS.bitTape == 8) {
                    _tapeLoadBitsBit(data);
                } else {
                    _tapeLoadBits(data);
                }
                _tapeRajInfoPartPercent();
            }
            return THIS.bit[THIS.ptrBit++];
        } else {                    // motor off ?
            return 0;
        }
    } else {                    // no tape loaded
        return 0;
    }
}

/*
BYTE tapeGetFromTapeOld() {
    BYTE data = 0;
    if (THIS.loaded != 0) {
        if (THIS.bitTape != 0) {
            if (THIS.playing != 0) {
                if (THIS.ptrBit > 7) {	// byte finished
                    if (THIS.ptr < THIS.size) {
                        if (THIS.ptr == THIS.parts[THIS.part].end) {	// reach end of part
                            Serial.printf("end of part %d -> ", THIS.part);
                            if (THIS.part == THIS.nparts) {				// last one, sorry
                                Serial.println("end of tape");
                            } else {
                                THIS.part++;
                                Serial.printf("next part %d\n", THIS.part);
                                if (THIS.parts[THIS.part].bitImage != 0) {
                                    if (THIS.parts[THIS.part].type == 0x00) {	// normal one,
                                        THIS.ptr += 4;							// skip header
                                    }
                                } else {
                                    THIS.bitTape = 0;
                                }
                                // THIS.rajButtonPart();
                                // THIS.rajButtonInfo();
                                data = THIS.data[THIS.ptr++];
                                _tapeRajInfoPart();
                                // THIS.rajButtonPartPerCent();
                            }
                        } else {
                            data = THIS.data[THIS.ptr++];
                            _tapeRajInfoPartPercent();
                            // THIS.rajButtonPartPerCent();
                        }
                    } else {        // end of tape ... rewind and restart ...

                    }
                    //Cpu.setDataSpe(data);		// for test actually to check consistency
                    if (THIS.bitTape != 0) {
                        _tapeLoadBitsBit(data);
                    } else {
                        _tapeLoadBits(data);
                    }
                }
            } else {
                return 0;
            }
        } else {
            if (THIS.ptrBit > 12) {	// byte finished
                if (THIS.ptr < THIS.size) {
                    data = THIS.data[THIS.ptr++];
                    _tapeRajInfoPartPercent();
                    // THIS.rajButtonPartPerCent();
                } else { // ... no part treated .. ??? ...
                    data = 0x16;	// pad with headers
                }
                //Cpu.setDataSpe(data);		// for test actually to check consistency
                //Serial.printf("new byte is %02X from %d\n", data, THIS.ptr-1);
                _tapeLoadBits(data);
            }
        }
        return THIS.bit[THIS.ptrBit++];
    } else {
        // todo
        return 0;
    }
} */
    
void tapeWriteBit(WORD timer) {
    BYTE bit = (timer < 256) ? 1 : 0;
    if (THIS.bitTape == 8) {            // special bit Image tape
        if (THIS.wrTick != 0) {				// second part of bit
            THIS.wrByte >>= 1;
            THIS.wrByte |= (bit << 7);
            THIS.ptrBitWr++;
            if (THIS.ptrBitWr == 8) {
#ifdef DEBUGTAPE
                Serial.printf("%02X,", THIS.wrByte);
                if ((THIS.wrPtr % 16) == 0) {
                    Serial.println("");
                }
#endif
                //Log.d(__D_TAG, String.format("got %02X", __tapeWrByte));
                THIS.wrData[THIS.wrPtr++] = THIS.wrByte;
                THIS.wrByte = 0;
                THIS.ptrBitWr = 0;
            }
        } else {						// first part of bit
        }
    } else {
        if (THIS.wrTick != 0) {				// second part of bit
            //Log.d(__D_TAG, "at tock got " + bit);
            if (THIS.ptrBit < 13) {
                THIS.wrRawByte |= bit << THIS.ptrBit;
                THIS.ptrBit++;
                if (THIS.ptrBit == 13) {	// first full byte received
                    //Log.d(__D_TAG, String.format("got 13 bits and %d data bit for %03X", __ptrBit, __tapeWrRawByte));
                    if (THIS.wrRawByte != 0x1C2C) {	//	not a full 0x16 sync, change to bit tape mode
                        THIS.bitTape = 8;
                        THIS.wrPtr = 0;
                        THIS.wrByte = THIS.wrRawByte & 0xFF;
#ifdef DEBUGTAPE
                        Serial.printf("now a BIT TAPE !!\n%02X,", THIS.wrByte);
#endif
                        THIS.wrData[THIS.wrPtr++] = THIS.wrByte;
                        THIS.ptrBitWr = THIS.ptrBit - 8;
                        THIS.wrByte = (THIS.wrRawByte >> 8) << (8 - THIS.ptrBitWr);
                    }
                }
            }
            if (THIS.bitTape == 13) {
                if (THIS.wrWaitZero != 0) {				// wait for start bit
                    if (bit == 0) {				// got a zero, now data
                        THIS.wrWaitZero = 0;
                        THIS.wrWaitData = 1;
                    }
                } else if (THIS.wrWaitData != 0) {		// wait for data bits
                    THIS.wrByte |= bit << THIS.ptrBitWr;
                    THIS.ptrBitWr++;
                    if (THIS.ptrBitWr == 9) {
                        THIS.wrByte &= 0xFF;		// remove parity
#ifdef DEBUGTAPE
                        Serial.printf("%02X,", THIS.wrByte);
                        if ((THIS.wrPtr % 16) == 0) {
                            Serial.println("");
                        }
#endif
                        THIS.wrWaitData = 0;
                        THIS.wrWaitZero = 1;			// now wait for start bit
                        THIS.wrData[THIS.wrPtr++] = THIS.wrByte;
                        THIS.wrByte = 0;
                        THIS.ptrBitWr = 0;
                    }
                }
            } else {
            }
        } else {						// first part of bit
            if (timer > 256) {
                //Log.d(__D_TAG, "at tick got " + ((timer < 256) ? 1 : 0) + " !!");
            }
        }
    }
    THIS.wrTick ^= 1;
}

BOOL _tapeWriteToTape() {
#ifdef DEBUGTAPE
    Serial.printf("Have to write %d bytes on tape\n", THIS.wrPtr);
#endif
    if (THIS.wrPtr > 0) {
        int start = 0;
        if (THIS.bitTape == 13 || THIS.quickWrite) {
            while(THIS.wrData[start] == 0x16) {
                start++;    
            }
            if (start > 4 && start < THIS.wrPtr) {  // keep only 4 0x16 
                start -= 4; 
#ifdef DEBUGTAPE
                Serial.printf("  will write %d bytes on tape\n", THIS.wrPtr-start);
#endif
            } else {
                start = 0;
            }
        }
        if ((THIS.wrPtr - start + 4) < (THIS.length - THIS.size)) {
            int i, j;
            if (THIS.part < THIS.nparts) { 		// not at OET, move datas it
                BYTE *source = THIS.data + THIS.parts[THIS.part].ptr;
                BYTE *dest = source + THIS.wrPtr - start + ((THIS.bitTape == 8) ? 4 : 0);
                size_t len = THIS.size  - THIS.parts[THIS.part].ptr;
                memmove(dest, source, len);
                // for (i = THIS.size + THIS.wrPtr + ((THIS.bitTape == 8) ? 4 : 0), j = THIS.size; j >= THIS.parts[THIS.part].ptr; i--, j--) {
                //    THIS.data[i] = THIS.data[j];
                //}
            }
            i = THIS.parts[THIS.part].ptr;
            if (THIS.bitTape  == 8) {    // bit image part ? mark it
                int n = THIS.wrPtr;
                THIS.data[i++] = 0x61;
                THIS.data[i++] = n >> 16;
                THIS.data[i++] = (n >> 8) & 0xFF;
                THIS.data[i++] = n & 0xFF;
            }
            memcpy(THIS.data+i, THIS.wrData+start, THIS.wrPtr-start);
            // for (j = 0; j < THIS.wrPtr; i++, j++) {
            //    THIS.data[i] = THIS.wrData[j];
            //}
            THIS.size += THIS.wrPtr - start + ((THIS.bitTape == 8) ? 4 : 0);
            i = THIS.part;
            _tapeAnalyse();
            THIS.part = i+1;
            THIS.ptr = THIS.parts[THIS.part].ptr;
            THIS.dataChanged = TRUE;
            return 1;
        } else {
#ifdef DEBUGTAPE
            Serial.println("Tape full, save not done");
#endif
            return 0;
        }
    }
    return 0;
}

BOOL tapeDeleteFromTape() {
    if (THIS.loaded != 0) {
        if (THIS.part < THIS.nparts) {		// ok not EOT
            int i, j;
            if (THIS.part < (THIS.nparts-1)) { // not last one
                for (i = THIS.parts[THIS.part].ptr, j = THIS.parts[THIS.part+1].ptr; j < THIS.size; i++, j++) {
                    THIS.data[i] = THIS.data[j];
                }
            }
            THIS.size -= THIS.parts[THIS.part+1].ptr - THIS.parts[THIS.part].ptr;
            i = THIS.part;
            _tapeAnalyse();
            THIS.part = i;
            THIS.ptr = THIS.parts[THIS.part].ptr;
            THIS.dataChanged = TRUE;
            _tapeRajInfoPart();
            return 1;
        } else {
#ifdef DEBUGTAPE
            Serial.println("Error can\'t delete EOT");
#endif
            return 0;
        }
    }
    return 0;		// can't be
}

void tapeStartOperationRd() {
    THIS.bottomLine[40] = 0x12; // GREEN start reading
    THIS.readWanted = 1;
    if (THIS.loaded) {
        if (THIS.parts[THIS.part].bitImage == 8) {
#ifdef DEBUGTAPE
            Serial.println("special tape op read");
#endif
            THIS.bitTape = 8;
            //Log.d(__D_TAG, String.format("start at %06X (%d),  with %02X %02X %02X %02X", __tapePtr, __tapeType[__tapeParts], __lookDataByte(__tapePtr), __lookDataByte(__tapePtr+1), __lookDataByte(__tapePtr+2), __lookDataByte(__tapePtr+3)));
            if (THIS.parts[THIS.part].type == 0x00) {	// bit mode with header
                THIS.ptr += 4;							// skipp it
            } else {
                THIS.ptr++;
            }
            // THIS.bgLoad.style.backgroundColor = 'yellow';
            _tapeLoadBitsBit(THIS.data[THIS.ptr++]);
        } else {
            THIS.bitTape = 13;
            _tapeLoadBits(0x16);            // first 0x16 in header (should be 4 in all)
            // THIS.bgLoad.style.backgroundColor = 'green';
        }
#ifdef DEBUGTAPE
        Serial.printf("start playing tape part %d at %d/%d\n", THIS.part, THIS.ptr, THIS.size);
#endif
    } else {
        THIS.bitTape = 13;
        _tapeLoadBits(0x00);
    }
}

void tapeStartOperationWr() {
    THIS.bottomLine[40] = 0x11; // RED start writing
    THIS.writeWanted = 1;
#ifdef DEBUGTAPE
    Serial.println("write to tape");
#endif
    THIS.wrPtr = 0;
    THIS.ptrBit = 0;
    THIS.ptrBitWr = 0;
    THIS.wrTick = 0;
    THIS.rawByte = 0;
    THIS.wrByte = 0;
    THIS.wrWaitZero = 1;
    THIS.wrWaitData = 0;
    
    // THIS.bgLoad.style.backgroundColor = 'red';
}

void tapeEndOperation(BOOL read, BOOL write) {
    THIS.bottomLine[40] = 0x20; // all stopped
    if ((read || THIS.quickRead) != 0) {							// end of read op
        _tapeUpdatePart(THIS.ptr);
    } else if ((write || THIS.quickWrite) != 0) {				// end of write op
        if (THIS.bitTape == 8) {		// finish byte
            if (THIS.ptrBitWr > 0) {
                THIS.wrByte = THIS.wrByte >> (8 - THIS.ptrBitWr);
                THIS.wrData[THIS.wrPtr++] = THIS.wrByte;
            }
        }
        if (_tapeWriteToTape() != 0) {
            _tapeRajInfoTape();
        }
    }
    THIS.quickRead = 0;
    THIS.quickWrite = 0;
    THIS.bitTape = 13;      // normal part
    THIS.playing = 0;
    THIS.readWanted = 0;
    THIS.writeWanted = 0;
    THIS.lastPercent = 0;
#ifdef DEBUGTAPE
    Serial.println("end of tape op");
#endif
}

BOOL _tapeLoadFd(File fd) {
    BOOL ret = false;
    size_t size = fd.size();
    if ((size > 0) && (size <= THIS.length)) {
        if (THIS.data) {
            if (fd.readBytes((char *)THIS.data, size) == size) {
                THIS.loaded = TRUE;
                THIS.size = size;
                THIS.dataChanged = FALSE;
                _tapeAnalyse();
                THIS.part = 0;
                THIS.ptr = THIS.parts[THIS.part].ptr;
                ret = true;
            } else {
                Serial.printf("read of %d failed for tape\n", size);
            }
        } else {
            Serial.println("malloc failed for tape");
        }
    }
    fd.close();
    return ret;
}

BOOL tapeLoadSD(const char *name) {
    BOOL ret = FALSE;
    File fd = SD.open(name, "rb");
    if (fd) {
        if (_tapeLoadFd(fd)) {
            strncpy(THIS.fname, name, 255);
            THIS.fname[255] = 0x00;
            size_t pos = strrchr(name, '/') - name;
            strncpy(THIS.path, name, pos + 1);
            THIS.path[pos+1] = 0x00;
            strcpy(THIS.name, strrchr(name, '/') + 1);
            ret = TRUE;
        }
    } 
    _tapeRajInfoTape();
    return ret;
}

BOOL tapeLoadSPI(const char *name) {
    BOOL ret = FALSE;
    File fd = FFat.open(name, "rb");
    if (fd) {
        if (_tapeLoadFd(fd)) {
            strncpy(THIS.fname, name, 255);
            THIS.fname[255] = 0x00;
            size_t pos = strrchr(name, '/') - name;
            strncpy(THIS.path, name, pos + 1);
            THIS.path[pos+1] = 0x00;
            strcpy(THIS.name, strrchr(name, '/') + 1);
            ret = TRUE;
        }
    } 
   _tapeRajInfoTape();
    return ret;
}

BOOL _tapeSaveFd(File fd) {
    size_t wr = fd.write(THIS.data, THIS.size);
    fd.close();
    return (wr == THIS.size);
}

BOOL tapeSaveSD(const char *path) {
    BOOL ret = FALSE;
    strcpy(THIS.fname, path);
    if (strlen(path) > 1) {
        strcat(THIS.fname, "/");
    }
    strcat(THIS.fname, THIS.name);
    File fd = SD.open(THIS.fname, "wb");
    if (fd) {
        if (_tapeSaveFd(fd)) {
            THIS.dataChanged = FALSE;
            ret = TRUE;
            _tapeRajInfoPart();
        }
    } 
    _tapeRajInfoTape();
    return ret;
}

BOOL tapeSaveSPI(const char *path) {
    BOOL ret = FALSE;
    strcpy(THIS.fname, path);
    if (strlen(path) > 1) {
        strcat(THIS.fname, "/");
    }
    strcat(THIS.fname, THIS.name);
    File fd = FFat.open(THIS.fname, "wb");
    if (fd) {
        if (_tapeSaveFd(fd)) {
            THIS.dataChanged = FALSE;
            ret = TRUE;
            _tapeRajInfoPart();
        }
    } 
    _tapeRajInfoTape();
    return ret;
}

void tapeReset() {
    THIS.playing = 0;
    THIS.readWanted = 0;
    THIS.writeWanted = 0;
    _tapeRajInfoTape();
    _tapeRajInfoPart();
}

void tapePrev() {
    if (THIS.loaded) {
        if (THIS.part > 0) {
            THIS.part--;
            THIS.ptr = THIS.parts[THIS.part].ptr;
           _tapeRajInfoPart();
        }
    }
}

void tapeNext() {
    if (THIS.loaded) {
        if (THIS.part < THIS.nparts) {
            THIS.part++;
            THIS.ptr = THIS.parts[THIS.part].ptr;
            _tapeRajInfoPart();
        }
    }
}

void tapeBegin() {
    if (THIS.loaded) {
        THIS.part = 0;
        THIS.ptr = THIS.parts[THIS.part].ptr;
        _tapeRajInfoPart();
    }
}

void tapeEnd() {
    if (THIS.loaded) {
        if (THIS.nparts >= 0) {
            THIS.part = THIS.nparts;
            THIS.ptr = THIS.parts[THIS.part].ptr;
           _tapeRajInfoPart();
        }
    }
}

void tapeEmptyTape() {
    THIS.size = 0;
    THIS.loaded = TRUE;
    _tapeAnalyse();
    _tapeRajInfoTape();
}

void tapeInit() {
    THIS.length = 512*1024;     // max length of a tape (~ 45 minutes tape)
    THIS.data = (BYTE *)ps_malloc(THIS.length);
    THIS.wrData = (BYTE *)ps_malloc(64*1024);
    strcpy(THIS.path, "/");
    THIS.loaded = FALSE;
    memset(THIS.topLine, 33, 41);
    THIS.topLine[0] = 0x07;
    memset(THIS.bottomLine, 33, 41);
    THIS.bottomLine[0] = 0x07;
    tapeEmptyTape();
}
