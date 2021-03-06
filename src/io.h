
//
//   header file for ula  chipsets 
//

#include "io_d.h"

#undef THIS
#define THIS System.Io

void ioReset() {
    memset(&THIS, 0, sizeof(IO));
}

void ioPressKeyMtx(BYTE line, BYTE bcol) {
//    log_d("Press %02X at %d", bcol, line);
    if (line < 8) {
        THIS.kM[line] |= bcol;
    } else {    // overlay subsystem
        if (line == 8) {
            if (bcol == 12) {           // F12 key
                System.Osd.display = 1 - System.Osd.display;
                System.Osd.topDisplay = 1;
            } else if (bcol == 11) {    // F11 key
                System.Osd.topDisplay = 1 - System.Osd.topDisplay;
                if (!System.Osd.topDisplay) {
                    osdTopDisplay2(1);
                    osdBottomDisplay2(1);
                }
            }
        }
    }
}
void ioReleaseKeyMtx(BYTE line, BYTE bcol) {
//    log_d("Release %02X at %d", bcol, line);
    if (line < 8) {
        THIS.kM[line] &= ~bcol;
    } else {

    }
}

void ioChangeKeyMod() {   // not used 
    if (THIS.keyDowns != 0) return;
    System.ram[0x209] = 0x38;
    if (THIS.shiftl != 0) {
        ioPressKeyMtx(4, 0x10);                                  // then shiftL
        System.ram[0x209] = 0xA4;
    } else {
        ioReleaseKeyMtx(4, 0x10);                                  // then shiftL
    }
    if (THIS.shiftr != 0) {
        ioPressKeyMtx(7, 0x10);                                  // then shiftR
        System.ram[0x209] = 0xA7;
    } else {
        ioReleaseKeyMtx(7, 0x10);                                  // then shiftR
    }
    if (THIS.ctrll != 0) {
        ioPressKeyMtx(2, 0x10);                                  // then ctrl
        System.ram[0x209] = 0xA2;
    } else {
        ioReleaseKeyMtx(2, 0x10);                                  // then ctrl
    }
    if (THIS.alt != 0) {
        ioPressKeyMtx(5, 0x10);                                  // then funct is alt
        System.ram[0x209] = 0xA5;
    } else {
        ioReleaseKeyMtx(5, 0x10);                                  // then funct
    }
}

inline void _ioDoIrqVia() {
    THIS.irqVia = (THIS.IER & THIS.IFR & 0x7F) ? 1 : 0;
    System.Cpu.IRQ = THIS.irqVia | THIS.irqFdc;
//        if (THIS.cpu.IRQ) {
//            console.log('IRQ demand');
//        }
//        if (THIS.cpu.irqVia) {
//            console.log('IRQvia demand');
//        }
}

void ioDoTimer(WORD cycles) {
    if (THIS.timer1L > 0) {
        if (cycles >= THIS.timer1C) {   // elapsed
            THIS.timer1C += THIS.timer1L - cycles;
            if (THIS.timer1R != 0) {
                THIS.IFR |= 0x40;
                _ioDoIrqVia();
            }
            THIS.timer1R = THIS.timer1M & 1;	// one shoot or continuous
        }
        THIS.timer1C -= cycles;
    }
    if (THIS.timer2L > 0) {
        if (cycles >= THIS.timer2C) {
            THIS.timer2C += THIS.timer2L - cycles;
            if (THIS.timer2R != 0) {
                THIS.IFR |= 0x20;
                _ioDoIrqVia();
            }
            THIS.timer2R = 0;				// one shoot always
        }
        THIS.timer2C -= cycles;
    }
}

static void _ioPsgBus() {
    if (THIS.bdir) {
        if (THIS.bc1) {
            System.Psg.reg = THIS.ORA & THIS.DDRA & 0x0F;
        } else {
            if (ym2149Write(THIS.ORA & THIS.DDRA)) {
                THIS.kCol = (~(THIS.ORA & THIS.DDRA)) & 0xFF;
            }
        }
    } else if (THIS.bc1) {
        THIS.IRA = System.Psg.regs[System.Psg.reg];
    }
}

BYTE ioRead(BYTE addr) {
    BYTE i = 0;
    switch(addr) {
        case 0x00:      // input register B
            THIS.IFR &= ~0x18;							// reset CB2 CB1 interrupt flag
            if ((THIS.DDRB & 0x08) == 0) {						// PB3 as input
                THIS.IRB &= ~0x08;                                    // PB3 reset
                THIS.IRB |= ((THIS.kM[THIS.ORB & 0x07] & THIS.kCol)) ? 0x08 : 0x00;
            }
            if (THIS.tapeOp) {
                if ((THIS.tapeRead == 0) && (THIS.tapeWrite == 0)) {			// first acces -> read op
                    tapeStartOperationRd();
                    Serial.println("IO read op");
                    THIS.tapeRead = 1;
                }
            }
            //				Log.d(__D_TAG, String.format("read IRB = %02X", __IRB));
            return (THIS.IRB & (~THIS.DDRB)) | (THIS.ORB & THIS.DDRB);
        case 0x01:		// input register A
            THIS.IFR &= ~0x03;						// reset CA2 CA1 interrupt flag
            _ioPsgBus();
            return (THIS.IRA & (~THIS.DDRA)) | (THIS.ORA & THIS.DDRA);
        case 0x02:		// data direction register B
            return THIS.DDRB;
        case 0x03:		// data direction register A
            return THIS.DDRA;
        case 0x04:		// Timer 1 counter low
            THIS.IFR &= ~0x40;						// reset T1 interrupt flag
            _ioDoIrqVia();
            if (THIS.tapeOp) {
                if ((THIS.tapeRead == 0) && (THIS.tapeWrite == 0)) {			// first acces -> write op
                    tapeStartOperationWr();
                    Serial.println("write op");
                    THIS.tapeWrite = 1;
                }
                return (BYTE)(THIS.T1C & 0x00FF);
            }
            return (BYTE)(THIS.timer1C & 0x00FF);
        case 0x05:		// Timer 1 counter high
            if (THIS.tapeOp) return (BYTE)(THIS.T1C >> 8);
            return (BYTE)(THIS.timer1C >> 8);
        case 0x06:		// Timer 1 latch low
            return (BYTE)(THIS.timer1L & 0x00FF);
        case 0x07:		// Timer 1 latch high
            return (BYTE)(THIS.timer1L >> 8);
        case 0x08:		// Timer 2 counter low
            THIS.IFR &= ~0x20;						// reset T2 interrupt flag
            _ioDoIrqVia();
            if (THIS.tapeOp) return (BYTE)(THIS.T2C & 0x00FF);
            return (BYTE)(THIS.timer2C & 0x00FF);
        case 0x09:		// Timer 2 counter high
            if (THIS.tapeOp) return (BYTE)(THIS.T2C >> 8);
            return (BYTE)(THIS.timer2C >> 8);
        case 0x0A:		// shift register
            THIS.IFR &= ~0x04;	// reset SR interrupt flag
            _ioDoIrqVia();
            return THIS.SR;
        case 0x0B:		// auxilliary control register
            return THIS.ACR;
        case 0x0C:		// peripheral control register
            return THIS.PCR;
        case 0x0D:		// interrupt flag register
            if (THIS.tapeOp) {
                if (THIS.tapeRead) {	// LDA $30D at E720 ...
                    i = tapeGetFromTape();
                    THIS.IFR |= 0x10;      // force CB1
                    _ioDoIrqVia();
                    // Serial.printf("IO bit is %d\n", i);
                    THIS.T2C = (i == 1) ? 0xFE80 : 0xFD80;
                } else if (THIS.tapeWrite) {	// force Timer 1 elapsed
                    // Serial.println("force IFR for write");
                    THIS.IFR |= 0x40;      // force T1 interrupt flag
                    _ioDoIrqVia();
                }
            }
            THIS.IFR &= 0x7F;
            return THIS.IFR | ((THIS.IFR & THIS.IER & 0x7F) ? 0x80 : 0x00);
        case 0x0E:		// interrupt enable register
            return THIS.IER | 0x80;
        case 0x0F:		// input register A (no ack)
            _ioPsgBus();
            return (THIS.IRA & (~THIS.DDRA)) | (THIS.ORA & THIS.DDRA);

    }
    return 0x00;
}

void ioWrite(BYTE addr, BYTE data) {
    BYTE tmp = 0;
    switch (addr) {
        case 0x00:      // output register B
            THIS.IFR &= ~0x18;
            _ioDoIrqVia();
            THIS.ORB = data;
            tmp = (data & 0x40) ? 1 : 0;
            if ((THIS.relay == 0) && (tmp)) {						// start of tape operation
                Serial.println("IO relay on");
                THIS.tapeOp = 1;
                tapeInitOp();
            }
            if ((THIS.relay) && (tmp == 0)) {						// end of tape operation
                Serial.println("IO relay off");
                tapeEndOperation(THIS.tapeRead, THIS.tapeWrite);
                THIS.tapeOp = 0;
                THIS.tapeRead = 0;
                THIS.tapeWrite = 0;
            }
            THIS.relay = tmp;
            // do scan keyboard, out strobe
            tmp = (data & 0x10) ? 1 : 0;
            //				if (__strobe && !strobe) {	// /strobe output print data from port A
            //					Log.d(__D_TAG, "FE strobe");
            //
            //				}
            if ((THIS.strobe == 0) && tmp) {	// /strobe output print data from port A
                //					Log.d(__D_TAG, "RE strobe");
                //Util.Print(getORA());
                THIS.IFR |= 0x02;		// ca1 set
                _ioDoIrqVia();
            }
            THIS.strobe = tmp;
            return;
        case 0x01:		// output register A
            THIS.IFR &= ~0x03;			// reset CA2 CA1 interrupt flag
            THIS.ORA = data;
            _ioPsgBus();
            return;
        case 0x02:		// data direction register B
            THIS.DDRB = data;
            return;
        case 0x03:		// data direction register A
            THIS.DDRA = data;
            _ioPsgBus();
            return;
        case 0x04:		// Timer 1 counter low
            THIS.timer1l = data;
            return;
        case 0x05:		// Timer 1 counter high
            THIS.timer1L = THIS.timer1l | (data << 8);
            THIS.timer1C = THIS.timer1L;
            THIS.IFR &= ~0x40;	// clear T1 interrupt
            THIS.timer1R = 1;
            _ioDoIrqVia();
            //__android_log_print(ANDROID_LOG_ERROR, "jni", "%04X T1L %06X T1C %06X", __pc, __timer1L, __timer1C);
            return;
        case 0x06:		// Timer 1 latch low
            THIS.timer1l = data;
            return;
        case 0x07:		// Timer 1 latch high
            THIS.timer1L = THIS.timer1l | (data << 8);
            if (THIS.tapeOp) {			// want to do something
                if (THIS.tapeWrite) {		// write
                    tapeWriteBit(data << 8);
                }
            }
            return;
        case 0x08:		// Timer 2 counter low
            THIS.timer2l = data;
            return;
        case 0x09:		// Timer 2 counter high
            THIS.timer2L = THIS.timer2l | (data << 8);
            THIS.IFR &= ~0x20;	// clear T2 interrupt
            THIS.timer2R = 1;
            _ioDoIrqVia();
            if (THIS.tapeOp == 0) {	// no rom tape op, do the real timer
                THIS.timer2C = THIS.timer2L;
            }
            return;
        case 0x0A:
            THIS.IFR &= ~0x04;						// reset SR interrupt flag
            THIS.SR = data;
            _ioDoIrqVia();
            return;
        case 0x0B:		// auxilliary control register
            THIS.ACR = data;
            THIS.timer2M = (THIS.ACR >> 5) & 0x01;
            THIS.timer1M = (THIS.ACR >> 6);
            THIS.timer1R = 0;				// no interrupt allowed until write
            THIS.srMode = (THIS.ACR >> 2) & 0x07;
//                if (THIS.srMode != 0) {
//                    console.log('SRMode ' + THIS.srMode + ' ' + THIS.SR);
//                }
            THIS.IFR &= ~(0x60);			// clear current interrupt
            THIS.pbLatch = (THIS.ACR >> 1) & 0x01;
            return;
        case 0x0C:		// peripheral control register
            THIS.PCR = data;
            //__ca1Latch = (__PCR & 0x01);			// printer ack
            THIS.ca2Control = (THIS.PCR >> 1) & 0x07;
            if (THIS.ca2Control == 7) {
                THIS.bc1 = 1;
            } else if (THIS.ca2Control == 6) {
                THIS.bc1 = 0;
            } else if (THIS.ca2Control) {
                //NSLog(@"ca2control %d", _ca2Control);
            }
            //			__cb1Latch = (__PCR >> 4) & 0x01;		no use in oric (printer ??)
            THIS.cb2Control = (THIS.PCR >> 5) & 0x07;
            if (THIS.cb2Control == 7) {
                THIS.bdir = 1;
            } else if (THIS.cb2Control == 6) {
                THIS.bdir = 0;
            } else if (THIS.cb2Control) {
                // NSLog(@"cb2control %d", _cb2Control);
            }
            _ioPsgBus();
            return;
        case 0x0D:		// IFR Timer1 / Timer 2 / CB1 / CB2 / SR / CA1 / CA2
            THIS.IFR &= ((~data) & 0x7F);
            _ioDoIrqVia();
            return;
        case 0x0E:		// IER Timer1 / Timer 2 / CB1 / CB2 / SR / CA1 / CA2
            if (data & 0x80) {   // set
                THIS.IER |= (data & 0x7F);
            } else {					// clear
                THIS.IER &= ((~data) & 0x7F);
            }
            _ioDoIrqVia();
            return;
        case 0xF:
            THIS.ORA = data;
            _ioPsgBus();
            return;
        default:
            return;
    }
}
