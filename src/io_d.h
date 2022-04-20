
#ifndef _C_io_h
#define _C_io_h

#include "types.h"

typedef struct {
    // VIA
    BYTE IRA;			// input register A
    BYTE ORA;			// output register A
    BYTE paLatch;
    BYTE IRB;			// input register B
    BYTE ORB;			// output register B
    BYTE pbLatch;
    BYTE DDRA;			// data direction register A
    BYTE DDRB;			// data direction register B
    BYTE SR; 			// shift register

    BYTE ca1Latch;
    BYTE ca2Control;
    BYTE cb1Latch;
    BYTE cb2Control;

    WORD timer1C;      // real timer 1 count
    WORD timer1L;      // real timer1 latch
    WORD timer2C;      // real timer 2 count
    WORD timer2L;      // real timer 2 latch

    BYTE timer1R;       // timer 1 run
    BYTE timer2R;       // timer 2 run

    BYTE timer1M;       // timer 1 mode
    BYTE timer2M;       // timer 2 mode

    WORD T1C;			// timer 1 count for tape op only
    WORD T1L;			// timer 1 latche       ""
    WORD T2C;			// timer 2 count        ""
    WORD T2L;			// timer 2 latche       ""

    BYTE IFR;           // interrupt flag
    BYTE IER;           // interrupt enable flag
    BYTE ACR;			// auxilliary control register
    BYTE PCR;			// peripheral control register

    BYTE srMode;

    WORD timer1l;      // timer1 low latch
    WORD timer2l;      // timer2 low latch

    // IRQ
    BYTE irqVia;
    BYTE irqFdc;
        // TAPE
    BYTE tapeOp;
    BYTE tapeRead;
    BYTE tapeWrite;
    BYTE relay;
        // PRINTER
    BYTE strobe;
        // KEYBOARD
    BYTE kCol;
    BYTE kM[8];
        // SOUND
    BYTE bdir;
    BYTE bc1;
        // KEYBOARD
    BYTE shiftl;
    BYTE shiftr;
    BYTE ctrlr;
    BYTE ctrll;
    BYTE alt;
    BYTE altgr;
    BYTE shiftF;
    BYTE keyDowns;
    BYTE codeRaw;
} IO;

void ioPressKeyMtx(BYTE line, BYTE bcol);
void ioReleaseKeyMtx(BYTE line, BYTE bcol);

void ioReset();

BYTE ioRead(BYTE addr);
void ioWrite(BYTE addr, BYTE data);
void ioDoTimer(WORD cycles);

#endif