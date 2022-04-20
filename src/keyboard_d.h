#ifndef _C_keyboard_h
#define _C_keyboard_h

#include "system_d.h"

typedef struct KEYBOARD {
    BYTE bufScan[32];
    int bufScanHigh = 0;
    int bufScanLow = 0;
    int fsmState = 0;
    int fsmCnt = 0;

    BOOL shift = FALSE;
    int ctrlAltDel = 0;
} KEYBOARD;

void kDoFSM();
void kPutScancode(BYTE code);

#endif