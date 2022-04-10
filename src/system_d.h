
#ifndef _C_system_h
#define _C_system_h

#include "types.h"

#include "m6502_d.h"
#include "ula_d.h"
#include "io_d.h"
#include "ym2149_d.h"
#include "tape_d.h"

typedef struct {
    M6502 Cpu;
    ULA Ula;
    IO Io;
    YM2149 Psg;
    // TAPE Tape;

    BYTE *ram;  // should allocate on heap 
    BYTE *rom0;
    BYTE *rom1;
    BYTE romDis;
    BYTE ovlEnable;
    BYTE atmos;

    DWORD *crt; // [(2+240*2+2)*(2+224+2)];
} SYSTEM;

void system_write(WORD addr, BYTE data);
BYTE system_read(WORD addr);
BYTE system_peek(WORD addr);
WORD system_deek(WORD addr);
void system_log(CHAR *str);

extern SYSTEM System;
extern const unsigned char basic11bData[];

#endif