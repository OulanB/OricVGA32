
//
//   header file for m6502 processor and chipsets for oric system
//

#include "system_d.h"

#undef THIS
#define THIS System

void system_write(WORD addr, BYTE data){
    if ((addr & 0xFF00) == 0x0300) {      // I/O
        io_write((BYTE)(addr & ((THIS.ovlEnable) ? 0xFF : 0x0F)), data);
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

BYTE system_read(WORD addr) {
    if ((addr & 0xFF00) == 0x0300) {
        return io_read((BYTE)(addr & 0x00FF));
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

BYTE system_peek(WORD addr) {
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

WORD system_deek(WORD addr) {
    return system_peek(addr) | (system_peek(addr+1) << 8);
}

void system_Init(BYTE atmos) {
    THIS.atmos = 0;
    THIS.romDis = 0;
    THIS.ovlEnable = 0;
    THIS.rom0 = (BYTE *) basic11bData;
    THIS.rom1 = (BYTE *) basic11bData;
    THIS.ram = (BYTE *)malloc(65536);
    for (DWORD i=0; i<65536; i++) {
        THIS.ram[i] = 0; 
    } 
    m6502_reset();
    ula_init();
}

void system_log(CHAR *str) {
    Serial.println(str);
}