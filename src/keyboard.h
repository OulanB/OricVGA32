/*
 * convert ps2 scancode to ...
 */

#include "system_d.h"

const BYTE __kPS2Code[][7] = {
// PS/2 scancodes    
//   code      E0 + code  code     E0+code
//    col  row  col  row  osd +shift
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x00
    {   9,   8,   0,   0, 137, 137,   0},         // scan 0x01    F9
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x02
    {   8,   8,   0,   0, 133, 133,   0},         // scan 0x03    F5
    {   2,   8,   0,   0, 131, 131,   0},         // scan 0x04    F3
    {   1,   8,   0,   0, 129, 129,   0},         // scan 0x05    F1
    {   2,   8,   0,   0, 130, 130,   0},         // scan 0x06    F2
    {  12,   8,   0,   0, 140, 140,   0},         // scan 0x07    F12  -> overlay susbsystem
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x08
    {  10,   8,   0,   0, 138, 138,   0},         // scan 0x09    F10
    {   8,   8,   0,   0, 136, 136,   0},         // scan 0x0A    F8
    {   6,   8,   0,   0, 134, 134,   0},         // scan 0x0B    F6
    {   4,   8,   0,   0, 132, 132,   0},         // scan 0x0C    F4
    {   0,   0,   0,   0,  10,  10,   0},         // scan 0x0D    TAB
    {   0,   0,   0,   0,  96, 126,   0},         // scan 0x0E    ` 
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x0F
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x10
    {0x10,   5,0x10,   5,   0,   0,   0},         // scan 0x11    L ALT       R ALT
    {0x10,   4,   0,   0,   0,   0,   0},         // scan 0x12    L SHIFT     PRNT ...
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x13
    {0x10,   2,0x10,   2,   0,   0,   0},         // scan 0x14    L CTRL      R CTRL      PAUSE 1
    {0x40,   1,   0,   0, 113,  81,   0},         // scan 0x15    Q
    {0x20,   0,   0,   0,  49,  33,   0},         // scan 0x16    1
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x17
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x18
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x19
    {0x20,   2,   0,   0, 122,  90,   0},         // scan 0x1A    Z
    {0x40,   6,   0,   0, 115,  83,   0},         // scan 0x1B    S
    {0x20,   6,   0,   0,  97,  65,   0},         // scan 0x1C    A
    {0x80,   6,   0,   0, 119,  87,   0},         // scan 0x1D    W
    {0x40,   2,   0,   0,  50,  64,   0},         // scan 0x1E    2
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x1F                L WIN
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x20
    {0x80,   2,   0,   0,  99,  67,   0},         // scan 0x21    C
    {0x40,   0,   0,   0, 120,  88,   0},         // scan 0x22    X
    {0x80,   1,   0,   0, 100,  68,   0},         // scan 0x23    D
    {0x08,   6,   0,   0, 101,  69,   0},         // scan 0x24    E
    {0x08,   2,   0,   0,  52,  36,   0},         // scan 0x25    4
    {0x80,   0,   0,   0,  51,  35,   0},         // scan 0x26    3
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x27                R WIN
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x28
    {0x01,   4,   0,   0,  32,  32,   0},         // scan 0x29    SPACE
    {0x08,   0,   0,   0, 118,  86,   0},         // scan 0x2A    V
    {0x08,   1,   0,   0, 102,  70,   0},         // scan 0x2B    F
    {0x02,   1,   0,   0, 116,  84,   0},         // scan 0x2C    T
    {0x04,   1,   0,   0, 114,  82,   0},         // scan 0x2D    R
    {0x04,   0,   0,   0,  53,  37,   0},         // scan 0x2E    5
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x2F                APPS
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x30
    {0x02,   0,   0,   0, 110,  78,   0},         // scan 0x31    N
    {0x04,   2,   0,   0,  98,  66,   0},         // scan 0x32    B
    {0x02,   6,   0,   0, 104,  72,   0},         // scan 0x33    H
    {0x04,   6,   0,   0, 103,  71,   0},         // scan 0x34    G
    {0x01,   6,   0,   0, 121,  89,   0},         // scan 0x35    Y
    {0x02,   2,   0,   0,  54,  94,   0},         // scan 0x36    6
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x37                POWER
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x38
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x39
    {0x01,   2,   0,   0, 109,  77,   0},         // scan 0x3A    M
    {0x01,   1,   0,   0, 106,  74,   0},         // scan 0x3B    J
    {0x01,   5,   0,   0, 117,  85,   0},         // scan 0x3C    U
    {0x01,   0,   0,   0,  55,  38,   0},         // scan 0x3D    7
    {0x01,   7,   0,   0,  56,  42,   0},         // scan 0x3E    8
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x3F                SLEEP
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x40
    {0x02,   4,   0,   0,  44,  60,   0},         // scan 0x41    ,
    {0x01,   3,   0,   0, 107,  75,   0},         // scan 0x42    K
    {0x02,   5,   0,   0, 105,  73,   0},         // scan 0x43    I
    {0x04,   5,   0,   0, 111,  79,   0},         // scan 0x44    O
    {0x04,   7,   0,   0,  48,  41,   0},         // scan 0x45    0
    {0x02,   3,   0,   0,  57,  40,   0},         // scan 0x46    9
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x47
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x48
    {0x04,   4,   0,   0,  46,  62,   0},         // scan 0x49    .
    {0x08,   7,0x08,   7,  47,  63,  47},         // scan 0x4A    /           KP/
    {0x02,   7,   0,   0, 108,  76,   0},         // scan 0x4B    L
    {0x04,   3,   0,   0,  59,  58,   0},         // scan 0x4C    ;
    {0x08,   5,   0,   0, 112,  80,   0},         // scan 0x4D    P
    {0x08,   3,   0,   0,  45,  95,   0},         // scan 0x4E    -
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x4F
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x50
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x51
    {0x80,   3,   0,   0,  39,  34,   0},         // scan 0x52    '
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x53
    {0x80,   5,   0,   0,  91, 123,   0},         // scan 0x54    [
    {0x80,   7,   0,   0,  61,  43,   0},         // scan 0x55    =
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x56
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x57
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x58    CAPS
    {0x10,   7,   0,   0,   0,   0,   0},         // scan 0x59    R SHIFT
    {0x20,   7,0x20,   7,  13,  13,  13},         // scan 0x5A    ENTER       KPENTER
    {0x40,   5,   0,   0,  93, 125,   0},         // scan 0x5B    ]
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x5C
    {0x40,   3,   0,   0,  92, 124,   0},         // scan 0x5D    backslash
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x5E                WAKE
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x5F
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x60
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x61
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x62
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x63
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x64
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x65
    {0x20,   5,   0,   0,   8,   8,   0},         // scan 0x66    BKSP
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x67
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x68
    {0x20,   0,   0,   0,  49, 157, 157},         // scan 0x69    KP1         END
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x6A
    {0x08,   2,0x20,   4,  52, 150, 150},         // scan 0x6B    KP4         LEFT
    {0x01,   0,   0,   0,  55, 156, 156},         // scan 0x6C    KP7         HOME
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x6D
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x6E
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x6F
    {0x04,   7,   0,   0,  48,   0,   0},         // scan 0x70    KP0         INSERT
    {0x04,   4,   0,   0,  46, 127, 127},         // scan 0x71    KP.         DELETE
    {0x40,   2,0x40,   4,  50, 153, 153},         // scan 0x72    KP2         DOWN
    {0x04,   0,   0,   0,  53,   0,   0},         // scan 0x73    KP5
    {0x02,   2,0x80,   4,  54, 151, 151},         // scan 0x74    KP6         RIGHT
    {0x01,   7,0x08,   4,  56, 152, 152},         // scan 0x75    KP8         UP
    {0x20,   1,   0,   0,  27,   0,   0},         // scan 0x76    ESC
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x77    NUM                         PAUSE 2
    {  11,   8,   0,   0, 139, 139,   0},         // scan 0x78    F11
    {   0,   0,   0,   0,  43,   0,   0},         // scan 0x79    KP+
    {0x80,   0,   0,   0,  51, 155, 155},         // scan 0x7A    KP3         PG DN
    {0x08,   3,   0,   0,  45,   0,   0},         // scan 0x7B    KP-
    {   0,   0,   0,   0,  42,   0,   0},         // scan 0x7C    KP*         PRNT SCRN
    {0x02,   3,   0,   0,  57, 154, 154},         // scan 0x7D    KP9         PG UP
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x7E    SCROLL
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x7F
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x80
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x81
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x82
    {   7,   8,   0,   0, 134, 134,   0},         // scan 0x83    F7
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x84
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x85
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x86    
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x87
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x88
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x89
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x8A
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x8B
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x8C
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x8D
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x8E
    {   0,   0,   0,   0,   0,   0,   0},         // scan 0x8F
};

#undef THIS
#define THIS System.Keyboard

void keyboardInit() {
    THIS.bufScanHigh = 0;
    THIS.bufScanLow = 0;
    THIS.ctrlAltDel = 0;
    THIS.fsmCnt = 0;
    THIS.fsmState = 0;
}

void kPutScancode(BYTE code) {
    THIS.bufScan[THIS.bufScanHigh++] = code;
    THIS.bufScanHigh &= 0x1F;
}

bool _kIsScancode() {
    return THIS.bufScanHigh != THIS.bufScanLow;
}

BYTE _kGetScancode() {
    BYTE code = 0xFF;
    if (THIS.bufScanHigh != THIS.bufScanLow) {
        code = THIS.bufScan[THIS.bufScanLow++];
        THIS.bufScanLow &= 0x1F;
    }
    return code;
}

void kDoFSM() {
    BYTE code = 0x00; 
    switch (THIS.fsmState) {
        case 0: // idle ...
            if (_kIsScancode()) {
                code = _kGetScancode();
                if (code == 0xF0) {
                    THIS.fsmState = 1;   // got 0xF0 
                } else if (code == 0xE0) {
                    THIS.fsmState = 2;   // got 0xE0
                } else if (code == 0xE1) {
                    THIS.fsmCnt = 0;
                    THIS.fsmState = 4;   // got 0xE1
                } else if (code < 0x90) {
                    if (System.Osd.display) {
                        osdPutScancode(THIS.shift ? __kPS2Code[code][5] : __kPS2Code[code][4]);
                    } else {
                        if (__kPS2Code[code][0]) {
                            ioPressKeyMtx(__kPS2Code[code][1], __kPS2Code[code][0]);
                        }
                    }
                    if (code == 0x14) {             // CTRL
                        THIS.ctrlAltDel |= 0x01;
                    } else if (code == 0x11) {      // ALT
                        THIS.ctrlAltDel |= 0x02;
                    }
                    if (THIS.ctrlAltDel == 0x07) {     // RESET !!
                        THIS.ctrlAltDel = 0;
                        m6502Reset();
                    }
                    if ((code == 0x12 || code == 0x59)) {
                        THIS.shift = TRUE;
                    }
                }
            }
            break;
        case 1:   // got 0xF0
            if (_kIsScancode()) {
                code = _kGetScancode();      // release complete
                if (code < 0x90) {
                    if (System.Osd.display) {
                    } else {
                        if (__kPS2Code[code][0]) {
                            ioReleaseKeyMtx(__kPS2Code[code][1], __kPS2Code[code][0]);
                        }
                    }
                    if (code == 0x14) {             // CTRL
                        THIS.ctrlAltDel &= ~0x01;
                    } else if (code == 0x11) {      // ALT
                        THIS.ctrlAltDel &= ~0x02;
                    }
                    if ((code == 0x12 || code == 0x59)) {
                        THIS.shift = FALSE;
                    }
                }
                THIS.fsmState = 0;
            }
            break;
        case 2:   // got 0xE0
            if (_kIsScancode()) {
                code = _kGetScancode();
                if (code == 0xF0) {
                    THIS.fsmState = 3;   // got 0xF0 
                } else if (code < 0x90) {
                    if (System.Osd.display) {
                        osdPutScancode(__kPS2Code[code][6]);     // ascii in osd
                    } else {
                        if (__kPS2Code[code][2]) {
                            ioPressKeyMtx(__kPS2Code[code][3], __kPS2Code[code][2]);
                        }
                    }
                    if (code == 0x71) {      // DEL
                        THIS.ctrlAltDel |= 0x04;
                    }
                    if (THIS.ctrlAltDel == 0x07) {     // RESET !!
                        THIS.ctrlAltDel = 0;
                        m6502Reset();
                    }
                }
            }
            break;
        case 3:   // got 0xE0, then 0xF0
            if (_kIsScancode()) {
                code = _kGetScancode();      // release complete
                if (code < 0x90) {
                    if (System.Osd.display) {
                    } else {
                        if (__kPS2Code[code][2]) {
                            ioReleaseKeyMtx(__kPS2Code[code][3], __kPS2Code[code][2]);
                        }
                        if (code == 0x71) {             // DEL
                            THIS.ctrlAltDel &= ~0x04;
                        }
                    }
                }
                THIS.fsmState = 0;
            }
            break;
        case 4:   // got 0xE1
            if (_kIsScancode()) {
                code = _kGetScancode();
                if (code == 0xF0) {
                    THIS.fsmState = 5;
                } else {
                    THIS.fsmCnt++;
                }
                if (THIS.fsmCnt == 2) {                // Pause pressed ?
                    THIS.fsmState = 0;
                }
            }
            break;
        case 5:   // got 0xE1, 0xF0
            if (_kIsScancode()) {
                code = _kGetScancode();
                THIS.fsmCnt++;
                if (THIS.fsmCnt == 2) {
                    THIS.fsmState = 0;                 // Pause released ?
                } else {
                    THIS.fsmState = 4;                     // wait for next 
                }
            }
            break;
    }
}
