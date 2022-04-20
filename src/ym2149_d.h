
#ifndef _C_ym2149_h
#define _C_ym2149_h

#include "types.h"

typedef struct {
    BYTE    reg;
    BYTE    regs[16];
    
    WORD    tonePeriod0;                          // clock divided by 16 first
    WORD    tonePeriod1;                          // clock divided by 16 first
    WORD    tonePeriod2;                          // clock divided by 16 first
    WORD    noisePeriod;                            // clock divided by 16 first
    WORD    noiseLimit;                             // noise limit
    WORD    channelsEnable;
    WORD    amplitude0;
    WORD    amplitude1;
    WORD    amplitude2;
    WORD    envelopePeriod;                         // clock divided by 256 first
    WORD    envLimit;
    BYTE    envelopeShape;                          // 16 steps per EP
    
    BYTE    noiseCount;                             // counter for noise period
    BYTE    noiseLevel;                             // noise level
    BOOL    noiseMute;
    DWORD   rnd;                                    // for noise generation
    BOOL    noiseNew;                               // new output on noise
    
    BYTE    toneNew;                                // new output on tone channels
    WORD    toneCount0;                           // counter for square wave period
    WORD    toneCount1;                           // counter for square wave period
    WORD    toneCount2;                           // counter for square wave period
    WORD    toneLimit0;                           // tone limit of square wave
    WORD    toneLimit1;                           // tone limit of square wave
    WORD    toneLimit2;                           // tone limit of square wave
    BYTE    toneLevel0;                           // tone level of square wave
    BYTE    toneLevel1;                           // tone level of square wave
    BYTE    toneLevel2;                           // tone level of square wave
    SWORD   toneVol0;                             // tone volume on 16 bits signed
    SWORD   toneVol1;                             // tone volume on 16 bits signed
    SWORD   toneVol2;                             // tone volume on 16 bits signed
    SWORD   toneOut0;                             // tone output in 16 bits pcm
    SWORD   toneOut1;                             // tone output in 16 bits pcm
    SWORD   toneOut2;                             // tone output in 16 bits pcm
    BYTE    toneMute0;
    BYTE    toneMute1;
    BYTE    toneMute2;
    BOOL    noiseBit0;                            // noise mixer bit
    BOOL    noiseBit1;                            // noise mixer bit
    BOOL    noiseBit2;                            // noise mixer bit
    BOOL    toneBit0;                             // tone mixer bit
    BOOL    toneBit1;                             // tone mixer bit
    BOOL    toneBit2;                             // tone mixer bit
    WORD    envCount;                               // counter for envelope period
    BYTE    envPos;                                 // envelope position
    BYTE    *envTab;                                //  = __envShape0;

    WORD    output;
    
    DWORD   cyclesA;
    DWORD   cyclesB;
    
    DWORD   audioCyclesA;
    DWORD   audioCyclesB;
    
    BYTE    audioEnable;

    DWORD   samplerate;
    WORD    sharedBuffers;
    DWORD   states[16*4];
    // float   ringBuffer[48000];
    float   bufferPtr;
    DWORD   bufferLength;
} YM2149;

BYTE ym2149Write(BYTE data);
void ym2149DoAudio();
void ym2149Init(DWORD samplerate);

#endif