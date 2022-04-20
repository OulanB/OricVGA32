//
//   header file for ym2149 psg 
//

#include "ym2149_d.h"

#undef THIS
#define THIS System.Psg

enum STATE {
    READ_INDEX = 0,
    WRITE_INDEX = 1,
    RING_BUFFER_LENGTH = 2,
};

const static WORD __volTab[] = { 0, 513/6, 828/6, 1239/6, 1923/6, 3238/6, 4926/6, 9110/6, 10344/6, 17876/6, 24682/6, 30442/6, 38844/6, 47270/6, 56402/6, 65535/6};
const static BYTE __envShape0[] = { 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 128+15 };
const static BYTE __envShape4[] = {  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 0, 128+16 };

const static BYTE __envShape8[] = { 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 128+0 };
const static BYTE __envShapeA[] = { 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 128+0 };
const static BYTE __envShapeB[] = { 15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 15, 128+16 };
const static BYTE __envShapeC[] = {  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 128+0 };
const static BYTE __envShapeD[] = {  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15, 128+15 };
const static BYTE __envShapeE[] = {  0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 128+0 };
const static BYTE *__envShapes[] = {  __envShape0, // 0000
                                __envShape0, // 0001
                                __envShape0, // 0010
                                __envShape0, // 0011
                                __envShape4, // 0100
                                __envShape4, // 0101
                                __envShape4, // 0110
                                __envShape4, // 0111
                                __envShape8, // 1000
                                __envShape0, // 1001
                                __envShapeA, // 1010
                                __envShapeB, // 1011
                                __envShapeC, // 1100
                                __envShapeD, // 1101
                                __envShapeE, // 1110
                                __envShape4};// 1111

void ym2149DoAudio() { 
    float dout = 0.0;
    BYTE rbit = 0;
    BYTE a = 0;
    while (THIS.cyclesA >= THIS.audioCyclesA) {     // times 16 predivisor done
        THIS.cyclesA -= THIS.audioCyclesA;
        THIS.cyclesB += THIS.audioCyclesB;                        // this.audioCyclesB;
        
        if (THIS.noiseCount >= THIS.noiseLimit) {
            rbit = (THIS.rnd & 1) ^ ((THIS.rnd >> 2) & 1);
            THIS.rnd = (THIS.rnd >> 1) | (rbit << 16);
            THIS.noiseCount = 0;
            THIS.noiseLevel ^= (rbit & 1);
        } else THIS.noiseCount++;

        if (THIS.toneCount0 >= THIS.toneLimit0) {
            THIS.toneCount0 = 0; THIS.toneLevel0 ^=  1;				// ...and invert the square wave output
        } else THIS.toneCount0++;
        if (THIS.toneCount1 >= THIS.toneLimit1) {
            THIS.toneCount1 = 0; THIS.toneLevel1 ^=  1;				// ...and invert the square wave output
        } else THIS.toneCount1++;
        if (THIS.toneCount2 >= THIS.toneLimit2) {
            THIS.toneCount2 = 0; THIS.toneLevel2 ^=  1;				// ...and invert the square wave output
        } else THIS.toneCount2++;
        
        if (THIS.envCount >= THIS.envLimit) {
            THIS.envCount = 0;
            THIS.envPos++;										// Move to the next envelope position
            if ((THIS.envTab[THIS.envPos] & 0x80) != 0) 			// Reached the end of the envelope?
                THIS.envPos = THIS.envTab[THIS.envPos] & 0x7F;
            if ((THIS.amplitude0 & 0x10) != 0)  					// If the channel is using the envelope generator...
                THIS.toneVol0 = THIS.envTab[THIS.envPos];          // Recalculate its output volume
            if ((THIS.amplitude1 & 0x10) != 0)  					// If the channel is using the envelope generator...
                THIS.toneVol1 = THIS.envTab[THIS.envPos];          // Recalculate its output volume
            if ((THIS.amplitude2 & 0x10) != 0)  					// If the channel is using the envelope generator...
                THIS.toneVol2 = THIS.envTab[THIS.envPos];          // Recalculate its output volume
        } else THIS.envCount++;
    
        if (THIS.cyclesB >= 15625) {	// one data at samplerate Hz to output (is psg frequency >> 2)
            THIS.cyclesB -= 15625;
            dout = 0.0;
            if ((THIS.toneBit0 | THIS.noiseBit0) != 0) {
                a = (THIS.toneBit0 & THIS.toneLevel0) | (THIS.noiseBit0 & THIS.noiseLevel);
                dout += (a != 0) ? __volTab[THIS.toneVol0] : -__volTab[THIS.toneVol0];
            }
            if ((THIS.toneBit1 | THIS.noiseBit1) != 0) {
                a = (THIS.toneBit1 & THIS.toneLevel1) | (THIS.noiseBit1 & THIS.noiseLevel);
                dout += (a != 0) ? __volTab[THIS.toneVol1] : -__volTab[THIS.toneVol1];
            }
            if ((THIS.toneBit2 | THIS.noiseBit2) != 0) {
                a = (THIS.toneBit2 & THIS.toneLevel2) | (THIS.noiseBit2 & THIS.noiseLevel);
                dout += (a != 0) ? __volTab[THIS.toneVol2] : -__volTab[THIS.toneVol2];
            }
//               dout += 0.1*Math.sin(this.bufferPtr*2.0*3.14159*440.0/44100.0);
            // THIS.ringBuffer[THIS.bufferPtr++] = dout;
            if (THIS.bufferPtr == THIS.bufferLength) THIS.bufferPtr = 0;         // loop back ;)
        }
    }
//    THIS.states[WRITE_INDEX] = THIS.bufferPtr; 
}

void ym2149Init(DWORD samplerate) {
    
    THIS.audioEnable = 1;

    // this.wl = wl;
    THIS.samplerate = samplerate;
    THIS.audioCyclesA = 16;      // psg frequency is 62500 Hz
    THIS.audioCyclesB = samplerate >> 2;
    THIS.bufferLength = 48000; // this.samplerate;
    THIS.bufferPtr = 0;
    THIS.envelopeShape = 0;
    THIS.envTab = (BYTE *) __envShapes[THIS.envelopeShape];   // NOT VERY EFFICIENT !!!!
    THIS.envPos = 0;
    //// Atomics.store(this.states, STATE.READ_INDEX, 0);
    //// Atomics.store(this.states, STATE.RING_BUFFER_LENGTH, this.bufferLength);
    //// this.wl.port.postMessage(this.sharedBuffers);       // initialize worklet

}

BYTE ym2149Write(BYTE data) {
    THIS.regs[THIS.reg] = data;
    if (THIS.reg == 14) return 1;
    if (THIS.audioEnable) ym2149DoAudio();
    switch(THIS.reg) {
        case 1:
            THIS.regs[1] &= 0xF;
        case 0:
            THIS.tonePeriod0 = (THIS.regs[1] << 8) | THIS.regs[0];
            THIS.toneLimit0 = (THIS.tonePeriod0 == 0) ? 0 : THIS.tonePeriod0 - 1;
            // console.log('PSG %04X %08d tone A period %d\n", System.Cpu.PC, psg->cyclesA, psg->tonePeriod[0]);
            break;
        case 3:
            THIS.regs[3] &= 0xF;
        case 2:
            THIS.tonePeriod1 = (THIS.regs[3] << 8) | THIS.regs[2];
            THIS.toneLimit1 = (THIS.tonePeriod1 == 0) ? 0 : THIS.tonePeriod1 - 1;
            break;
        case 5:
            THIS.regs[5] &= 0xF;
        case 4:
            THIS.tonePeriod2 = (THIS.regs[5] << 8) | THIS.regs[4];
            THIS.toneLimit2 = (THIS.tonePeriod2 == 0) ? 0 : THIS.tonePeriod2 - 1;
            break;
        case 6:
            THIS.regs[6] &= 0x1F;
            THIS.noisePeriod = THIS.regs[6];
            THIS.noiseLimit = (THIS.noisePeriod == 0) ? 0 : THIS.noisePeriod - 1;
            // console.log('noiseLimit ' + THIS.noiseLimit);
            break;
        case 7:
            THIS.noiseBit2 = ((data & 0x20) == 0) ? 1 : 0;
            THIS.noiseBit1 = ((data & 0x10) == 0) ? 1 : 0;
            THIS.noiseBit0 = ((data & 0x08) == 0) ? 1 : 0;
            THIS.toneBit2 = ((data & 0x04) == 0) ? 1 : 0;
            THIS.toneBit1 = ((data & 0x02) == 0) ? 1 : 0;
            THIS.toneBit0 = ((data & 0x01) == 0) ? 1 : 0;
            //console.log('Channels ' + (255 - data).toString(16));
            break;
        case 8:
            THIS.regs[8] &= 0x1F;
            THIS.amplitude0 = THIS.regs[8];
            THIS.toneVol0 = ((THIS.amplitude0 & 0x10) != 0) ? THIS.envTab[THIS.envPos] : THIS.amplitude0 & 0x0F;
            break;
        case 9:
            THIS.regs[9] &= 0x1F;
            THIS.amplitude1 = THIS.regs[9];
            THIS.toneVol1 = ((THIS.amplitude1 & 0x10) != 0) ? THIS.envTab[THIS.envPos] : THIS.amplitude1 & 0x0F;
            break;
        case 10:
            THIS.regs[10] &= 0x1F;
            THIS.amplitude2 = THIS.regs[10];
            THIS.toneVol2 = ((THIS.amplitude2 & 0x10) != 0) ? THIS.envTab[THIS.envPos] : THIS.amplitude2 & 0x0F;
            break;
        case 12:
        case 11:
            THIS.envelopePeriod = (THIS.regs[12] << 8) | THIS.regs[11];
            THIS.envLimit = (THIS.envelopePeriod == 0) ? 0 : THIS.envelopePeriod-1;
            // console.log('EP ' + THIS.envelopePeriod);
            break;
        case 13:
            THIS.regs[13] &= 0x0F;
            THIS.envelopeShape = THIS.regs[13];
            THIS.envTab = (BYTE *) __envShapes[THIS.envelopeShape];   // NOT VERY EFFICIENT !!!!
            THIS.envPos = 0;
            if ((THIS.amplitude0 & 0x10) != 0) THIS.toneVol0 = THIS.envTab[0];
            if ((THIS.amplitude1 & 0x10) != 0) THIS.toneVol1 = THIS.envTab[0];
            if ((THIS.amplitude2 & 0x10) != 0) THIS.toneVol2 = THIS.envTab[0];
            break;
    }
    return 0;
}
