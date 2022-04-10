/*
 * convert ps2 scancode to ...
 */

#include "types.h"

#include "io_d.h"

const BYTE _kPS2Code[][4] = {
    {   0,   0,   0,   0},         // scan 0x00
    {   0,   0,   0,   0},         // scan 0x01    F9
    {   0,   0,   0,   0},         // scan 0x02
    {   0,   0,   0,   0},         // scan 0x03    F5
    {   0,   0,   0,   0},         // scan 0x04    F3
    {   0,   0,   0,   0},         // scan 0x05    F1
    {   0,   0,   0,   0},         // scan 0x06    F2
    {   0,   0,   0,   0},         // scan 0x07    F12
    {   0,   0,   0,   0},         // scan 0x08
    {   0,   0,   0,   0},         // scan 0x09    F10
    {   0,   0,   0,   0},         // scan 0x0A    F8
    {   0,   0,   0,   0},         // scan 0x0B    F6
    {   0,   0,   0,   0},         // scan 0x0C    F4
    {   0,   0,   0,   0},         // scan 0x0D    TAB
    {   0,   0,   0,   0},         // scan 0x0E    ` 
    {   0,   0,   0,   0},         // scan 0x0F
    {   0,   0,   0,   0},         // scan 0x10
    {0x10,   5,0x10,   5},         // scan 0x11    L ALT       R ALT
    {0x10,   4,   0,   0},         // scan 0x12    L SHIFT     PRNT ...
    {   0,   0,   0,   0},         // scan 0x13
    {0x10,   2,0x10,   2},         // scan 0x14    L CTRL      R CTRL      PAUSE 1
    {0x40,   1,   0,   0},         // scan 0x15    Q
    {0x20,   0,   0,   0},         // scan 0x16    1
    {   0,   0,   0,   0},         // scan 0x17
    {   0,   0,   0,   0},         // scan 0x18
    {   0,   0,   0,   0},         // scan 0x19
    {0x20,   2,   0,   0},         // scan 0x1A    Z
    {0x40,   6,   0,   0},         // scan 0x1B    S
    {0x20,   6,   0,   0},         // scan 0x1C    A
    {0x80,   6,   0,   0},         // scan 0x1D    W
    {0x40,   2,   0,   0},         // scan 0x1E    2
    {   0,   0,   0,   0},         // scan 0x1F                L WIN
    {   0,   0,   0,   0},         // scan 0x20
    {0x80,   2,   0,   0},         // scan 0x21    C
    {0x40,   0,   0,   0},         // scan 0x22    X
    {0x80,   1,   0,   0},         // scan 0x23    D
    {0x08,   6,   0,   0},         // scan 0x24    E
    {0x08,   2,   0,   0},         // scan 0x25    4
    {0x80,   0,   0,   0},         // scan 0x26    3
    {   0,   0,   0,   0},         // scan 0x27                R WIN
    {   0,   0,   0,   0},         // scan 0x28
    {0x01,   4,   0,   0},         // scan 0x29    SPACE
    {0x08,   0,   0,   0},         // scan 0x2A    V
    {0x08,   1,   0,   0},         // scan 0x2B    F
    {0x02,   1,   0,   0},         // scan 0x2C    T
    {0x04,   1,   0,   0},         // scan 0x2D    R
    {0x04,   0,   0,   0},         // scan 0x2E    5
    {   0,   0,   0,   0},         // scan 0x2F                APPS
    {   0,   0,   0,   0},         // scan 0x30
    {0x02,   0,   0,   0},         // scan 0x31    N
    {0x04,   2,   0,   0},         // scan 0x32    B
    {0x02,   6,   0,   0},         // scan 0x33    H
    {0x04,   6,   0,   0},         // scan 0x34    G
    {0x01,   6,   0,   0},         // scan 0x35    Y
    {0x02,   2,   0,   0},         // scan 0x36    6
    {   0,   0,   0,   0},         // scan 0x37                POWER
    {   0,   0,   0,   0},         // scan 0x38
    {   0,   0,   0,   0},         // scan 0x39
    {0x01,   2,   0,   0},         // scan 0x3A    M
    {0x01,   1,   0,   0},         // scan 0x3B    J
    {0x01,   5,   0,   0},         // scan 0x3C    U
    {0x01,   0,   0,   0},         // scan 0x3D    7
    {0x01,   7,   0,   0},         // scan 0x3E    8
    {   0,   0,   0,   0},         // scan 0x3F                SLEEP
    {   0,   0,   0,   0},         // scan 0x40
    {0x02,   4,   0,   0},         // scan 0x41    ,
    {0x01,   3,   0,   0},         // scan 0x42    K
    {0x02,   5,   0,   0},         // scan 0x43    I
    {0x04,   5,   0,   0},         // scan 0x44    O
    {0x04,   7,   0,   0},         // scan 0x45    0
    {0x02,   3,   0,   0},         // scan 0x46    9
    {   0,   0,   0,   0},         // scan 0x47
    {   0,   0,   0,   0},         // scan 0x48
    {0x04,   4,   0,   0},         // scan 0x49    .
    {0x08,   7,0x08,   7},         // scan 0x4A    /           KP/
    {0x02,   7,   0,   0},         // scan 0x4B    L
    {0x04,   3,   0,   0},         // scan 0x4C    ;
    {0x08,   5,   0,   0},         // scan 0x4D    P
    {0x08,   3,   0,   0},         // scan 0x4E    -
    {   0,   0,   0,   0},         // scan 0x4F
    {   0,   0,   0,   0},         // scan 0x50
    {   0,   0,   0,   0},         // scan 0x51
    {0x80,   3,   0,   0},         // scan 0x52    '
    {   0,   0,   0,   0},         // scan 0x53
    {0x80,   5,   0,   0},         // scan 0x54    [
    {0x80,   7,   0,   0},         // scan 0x55    =
    {   0,   0,   0,   0},         // scan 0x56
    {   0,   0,   0,   0},         // scan 0x57
    {   0,   0,   0,   0},         // scan 0x58    CAPS
    {0x10,   7,   0,   0},         // scan 0x59    R SHIFT
    {0x20,   7,   0,   0},         // scan 0x5A    ENTER
    {0x40,   5,   0,   0},         // scan 0x5B    ]
    {   0,   0,   0,   0},         // scan 0x5C
    {0x40,   3,   0,   0},         // scan 0x5D    backslash
    {   0,   0,   0,   0},         // scan 0x5E                WAKE
    {   0,   0,   0,   0},         // scan 0x5F
    {   0,   0,   0,   0},         // scan 0x60
    {   0,   0,   0,   0},         // scan 0x61
    {   0,   0,   0,   0},         // scan 0x62
    {   0,   0,   0,   0},         // scan 0x63
    {   0,   0,   0,   0},         // scan 0x64
    {   0,   0,   0,   0},         // scan 0x65
    {0x20,   5,   0,   0},         // scan 0x66    BKSP
    {   0,   0,   0,   0},         // scan 0x67
    {   0,   0,   0,   0},         // scan 0x68
    {0x20,   0,   0,   0},         // scan 0x69    KP1         END
    {   0,   0,   0,   0},         // scan 0x6A
    {0x08,   2,0x20,   4},         // scan 0x6B    KP4         LEFT
    {0x01,   0,   0,   0},         // scan 0x6C    KP7         HOME
    {   0,   0,   0,   0},         // scan 0x6D
    {   0,   0,   0,   0},         // scan 0x6E
    {   0,   0,   0,   0},         // scan 0x6F
    {0x04,   7,   0,   0},         // scan 0x70    KP0         INSERT
    {0x04,   4,   0,   0},         // scan 0x71    KP.         DELETE
    {0x40,   2,0x40,   4},         // scan 0x72    KP2         DOWN
    {0x04,   0,   0,   0},         // scan 0x73    KP5
    {0x02,   2,0x80,   4},         // scan 0x74    KP6         RIGHT
    {0x01,   7,0x08,   4},         // scan 0x75    KP8         UP
    {0x20,   1,   0,   0},         // scan 0x76    ESC
    {   0,   0,   0,   0},         // scan 0x77    NUM                         PAUSE 2
    {   0,   0,   0,   0},         // scan 0x78    F11
    {   0,   0,   0,   0},         // scan 0x79    KP+
    {0x80,   0,   0,   0},         // scan 0x7A    KP3         PG DN
    {0x08,   3,   0,   0},         // scan 0x7B    KP-
    {   0,   0,   0,   0},         // scan 0x7C    KP*         PRNT SCRN
    {0x02,   3,   0,   0},         // scan 0x7D    KP9         PG UP
    {   0,   0,   0,   0},         // scan 0x7E    SCROLL
    {   0,   0,   0,   0},         // scan 0x7F
    {   0,   0,   0,   0},         // scan 0x80
    {   0,   0,   0,   0},         // scan 0x81
    {   0,   0,   0,   0},         // scan 0x82
    {   0,   0,   0,   0},         // scan 0x83    F7
    {   0,   0,   0,   0},         // scan 0x84
    {   0,   0,   0,   0},         // scan 0x85
    {   0,   0,   0,   0},         // scan 0x86    
    {   0,   0,   0,   0},         // scan 0x87
    {   0,   0,   0,   0},         // scan 0x88
    {   0,   0,   0,   0},         // scan 0x89
    {   0,   0,   0,   0},         // scan 0x8A
    {   0,   0,   0,   0},         // scan 0x8B
    {   0,   0,   0,   0},         // scan 0x8C
    {   0,   0,   0,   0},         // scan 0x8D
    {   0,   0,   0,   0},         // scan 0x8E
    {   0,   0,   0,   0},         // scan 0x8F
};

BYTE _kBufScan[32];
int _kBufScanHigh = 0;
int _kBufScanLow = 0;
int _kFsmState = 0;
int _kFsmCnt = 0;

void kPutScancode(BYTE code) {
    _kBufScan[_kBufScanHigh++] = code;
    _kBufScanHigh &= 0x1F;
}

bool kIsScancode() {
    return _kBufScanHigh != _kBufScanLow;
}

BYTE kGetScancode() {
    BYTE code = 0xFF;
    if (_kBufScanHigh != _kBufScanLow) {
        code = _kBufScan[_kBufScanLow++];
        _kBufScanLow &= 0x1F;
    }
    return code;
}

void kDoFSM() {
    BYTE code = 0x00; 
    switch (_kFsmState) {
        case 0: // idle ...
            if (kIsScancode()) {
                code = kGetScancode();
                if (code == 0xF0) {
                    _kFsmState = 1;   // got 0xF0 
                } else if (code == 0xE0) {
                    _kFsmState = 2;   // got 0xE0
                } else if (code == 0xE1) {
                    _kFsmCnt = 0;
                    _kFsmState = 4;   // got 0xE1
                } else if (code < 0x90) {
                    if (_kPS2Code[code][0]) {
                        pressKeyMtx(_kPS2Code[code][1], _kPS2Code[code][0]);
                    }
                }
            }
            break;
        case 1:   // got 0xF0
            if (kIsScancode()) {
                code = kGetScancode();      // release complete
                if (code < 0x90) {
                    if (_kPS2Code[code][0]) {
                        releaseKeyMtx(_kPS2Code[code][1], _kPS2Code[code][0]);
                    }
                }
                _kFsmState = 0;
            }
            break;
        case 2:   // got 0xE0
            if (kIsScancode()) {
                code = kGetScancode();
                if (code == 0xF0) {
                    _kFsmState = 3;   // got 0xF0 
                } else if (code < 0x90) {
                    if (_kPS2Code[code][2]) {
                        pressKeyMtx(_kPS2Code[code][3], _kPS2Code[code][2]);
                    }
                }
            }
            break;
        case 3:   // got 0xE0, then 0xF0
            if (kIsScancode()) {
                code = kGetScancode();      // release complete
                if (code < 0x90) {
                    if (_kPS2Code[code][2]) {
                        releaseKeyMtx(_kPS2Code[code][3], _kPS2Code[code][2]);
                    }
                }
                _kFsmState = 0;
            }
            break;
        case 4:   // got 0xE1
            if (kIsScancode()) {
                code = kGetScancode();
                if (code == 0xF0) {
                    _kFsmState = 5;
                } else {
                    _kFsmCnt++;
                }
                if (_kFsmCnt == 2) {                // Pause pressed ?
                    _kFsmState = 0;
                }
            }
            break;
        case 5:   // got 0xE1, 0xF0
            if (kIsScancode()) {
                code = kGetScancode();
                _kFsmCnt++;
                if (_kFsmCnt == 2) {
                    _kFsmState = 0;                 // Pause released ?
                } else {
                    _kFsmState = 4;                     // wait for next 
                }
            }
            break;
    }
}
