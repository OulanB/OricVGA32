
#ifndef _C_system_h
#define _C_system_h

#include "types.h"

#include "ps2controller_d.h"
#include "sdfat_d.h"
#include "spifat_d.h"

#include "m6502_d.h"
#include "ula_d.h"
#include "io_d.h"
#include "ym2149_d.h"
#include "tape_d.h"
#include "osd_d.h"
#include "keyboard_d.h"

#include "file_d.h"

#define CPU_SLOT_FREQ 50
#define CPU_FREQ 1000000
#define PSG_FREQ 62500
#define SOUND_FREQ 24000
#define SOUND_SLOT_FREQ 100
#define SOUND_SAMPLE_BUFFER_SIZE (SOUND_FREQ/SOUND_SLOT_FREQ)

typedef struct {
    M6502 Cpu;
    ULA Ula;
    IO Io;
    YM2149 Psg;
    TAPE Tape;

    unsigned tNew;
    unsigned tPrev;
    unsigned tDelta;
    unsigned tN;
    unsigned tSum;
    unsigned nSum;

    TaskHandle_t xHandleCore1;
    uint8_t paramCore1;
    PS2Controller *ps2keybd;
    esp_timer_handle_t periodic_timer;
    esp_timer_create_args_t periodic_timer_args;
    
    BYTE vSyncCount;

    BYTE *ram;      // should allocate on heap 
    BYTE *rom0;
    BYTE *rom1;
    BYTE romDis;
    BYTE ovlEnable;
    BYTE atmos;     // 0: oric 1; 1: Oric Atmos; ...

    BOOL quickTape;

    KEYBOARD Keyboard;
    OSD Osd;
    SDFAT SdFat;
    SPIFAT SpiFat;

    DWORD *crt;     // to data in linked list for I2S output
} SYSTEM;

void systemWrite(WORD addr, BYTE data);
BYTE systemRead(WORD addr);
BYTE systemPeek(WORD addr);
WORD systemDeek(WORD addr);
void systemInit(BYTE atmos, BOOL quickTape);
void systemLog(CHAR *str);

void systemTapeSpeed(BOOL quick);

extern SYSTEM System;
extern const unsigned char basic11bData[];
extern const unsigned char charsetData[];

#endif
