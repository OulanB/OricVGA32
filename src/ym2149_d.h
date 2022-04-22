
#ifndef _C_ym2149_h
#define _C_ym2149_h

#include "types.h"

enum SoundGeneratorState {
  Stop,             /**<  */
  RequestToPlay,    /**<  */
  Playing,          /**<  */
  RequestToStop,    /**<  */
};

typedef struct {
    BYTE    reg;
    BYTE    regs[16];
    
    WORD    tonePeriod0;                          // clock divided by 16 first
    WORD    tonePeriod1;                          // clock divided by 16 first
    WORD    tonePeriod2;                          // clock divided by 16 first
    BYTE    noisePeriod;                            // clock divided by 16 first
    BYTE    noiseLimit;                             // noise limit
    WORD    channelsEnable;
    WORD    amplitude0;
    WORD    amplitude1;
    WORD    amplitude2;
    WORD    envelopePeriod;                         // clock divided by 256 first
    WORD    envLimit;
    BYTE    envelopeShape;                          // 16 steps per EP
    
    BYTE    noiseCount;                             // counter for noise period
    BYTE    noiseLevel;                             // noise level
    // BOOL    noiseMute;
    DWORD    rnd;                                   // for noise generation
    // BOOL    noiseNew;                               // new output on noise
    
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
    BYTE    noiseBit0;                            // noise mixer bit
    BYTE    noiseBit1;                            // noise mixer bit
    BYTE    noiseBit2;                            // noise mixer bit
    BYTE    toneBit0;                             // tone mixer bit
    BYTE    toneBit1;                             // tone mixer bit
    BYTE    toneBit2;                             // tone mixer bit
    WORD    envCount;                               // counter for envelope period
    BYTE    envPos;                                 // envelope position
    BYTE    *envTab;                                //  = __envShape0;

    WORD    output;
    
    DWORD   cyclesA;
    DWORD   cyclesB;
    
    DWORD   audioCyclesA;
    DWORD   audioCyclesB;
    
    BYTE    audioEnable;                        // 0 no 1 synchro 2 real-time

    DWORD   samplerate;
//    WORD    sharedBuffers;
//    DWORD   states[16*4];
    WORD   *buffer;
    // float   ringBuffer[48000];
    int     bufferReadPtr;
    int     bufferWritePtr;
    int     bufferLength;

    TaskHandle_t        m_waveGenTaskHandle;
    uint16_t            *m_sampleBuffer;
    int8_t              m_volume;
    int            m_sampleRate;
    bool                m_play;
    SoundGeneratorState m_state;
    SemaphoreHandle_t   m_mutex;

} YM2149;

BYTE ym2149Write(BYTE data);
void ym2149DoAudio();
void ym2149Init(BYTE mode);

int ym2149Delta();

bool ym2149I2SPlay(bool value);

#endif