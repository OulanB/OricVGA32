
//
//   header file for m6502 processor and chipsets for oric system
//

#include "system_d.h"

#undef THIS
#define THIS System

void systemWrite(WORD addr, BYTE data){
    if ((addr & 0xFF00) == 0x0300) {      // I/O
        ioWrite((BYTE)(addr & ((THIS.ovlEnable) ? 0xFF : 0x0F)), data);
    } else if (addr < 0xC000) {
        THIS.ram[addr] = data;
    } else if (THIS.romDis) {
        if (addr < 0xE000) {
            THIS.ram[addr] = data;
        } else if (THIS.ovlEnable == 0) {
            THIS.ram[addr] = data;
        }
    }
}

BYTE systemRead(WORD addr) {
    if ((addr & 0xFF00) == 0x0300) {
        return ioRead((BYTE)(addr & 0x00FF));
    } else if (addr < 0xC000) {
        return THIS.ram[addr];
    } else {
        if (THIS.romDis) {
            if (addr >= 0xE000) {
                if (THIS.ovlEnable) {
                    return THIS.rom1[addr & 0x1FFF];
                } else {
                    return THIS.ram[addr];
                }
            } else {
                return THIS.ram[addr];
            }
        } else {
            return THIS.rom0[addr & 0x3FFF];
        }
    }    
}

BYTE systemPeek(WORD addr) {
    if ((addr & 0xFF00) == 0x0300) {
        return 0xFF;        // for IO
    } else if (addr < 0xC000) {
        return THIS.ram[addr];
    } else {
        if (THIS.romDis) {
            if (addr >= 0xE000) {
                if (THIS.ovlEnable) {
                    return THIS.rom1[addr & 0x1FFF];
                } else {
                    return THIS.ram[addr];
                }
            } else {
                return THIS.ram[addr];
            }
        } else {
            return THIS.rom0[addr & 0x3FFF];
        }
    }    
}

WORD systemDeek(WORD addr) {
    return systemPeek(addr) | (systemPeek(addr+1) << 8);
}

void systemTapeSpeed(BOOL quick) {
    THIS.quickTape = quick;
    if (THIS.quickTape) {
        THIS.rom0 = (BYTE *) basic11bPData;  // atmos rom
        THIS.rom1 = (BYTE *) basic11bPData;  // atmos rom
    } else {
        THIS.rom0 = (BYTE *) basic11bData;  // atmos rom
        THIS.rom1 = (BYTE *) basic11bData;  // atmos rom
    }
}

void systemInit(BYTE atmos, BOOL quickTape) {
    THIS.atmos = atmos;
    THIS.romDis = 0;
    THIS.ovlEnable = 0;
    systemTapeSpeed(quickTape);
    THIS.ram = (BYTE *) malloc(65536);
    memset(THIS.ram, 0, 65536);
    m6502Reset();
    ulaInit();
    tapeInit();
}

void systemLog(CHAR *str) {
    Serial.println(str);
}
