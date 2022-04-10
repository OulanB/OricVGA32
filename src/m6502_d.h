
#ifndef _C_m6502_h
#define _C_m6502_h

#include "types.h"

typedef struct M6502 {
    BYTE A;
    BYTE X;
    BYTE Y;
    union {
        WORD w;
        BYTE b[2];
    } S;
    union {
        WORD w;
        BYTE b[2];
    } PC;
    BYTE IRQ;
    BYTE NMI;
    BYTE RESET;
    BYTE C;
    BYTE Z;
    BYTE I;
    BYTE D;
    BYTE B;
    BYTE V;
    BYTE N;

    WORD dcycles;  // cycles for current op
    SDWORD cycles; // cycles for current time slice (signed to get overtime)
    BYTE wantDesass;
    BYTE wantBreak;
    BYTE desassOk;
    DWORD desassCounter;
    BYTE desassOne;
} M6502;

void m6502_doOps(SDWORD cycles); // from JS
void m6502_reset();
void m6502_desass(BYTE val);

#endif