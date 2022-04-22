//
//   header file for ym2149 psg 
//

#include "ym2149_d.h"

#include "driver/i2s.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#undef THIS
#define THIS System.Psg

static const int8_t sinTable[257] = {
   0,    3,    6,    9,   12,   16,   19,   22,   25,   28,   31,   34,   37,   40,   43,   46,
  49,   51,   54,   57,   60,   63,   65,   68,   71,   73,   76,   78,   81,   83,   85,   88,
  90,   92,   94,   96,   98,  100,  102,  104,  106,  107,  109,  111,  112,  113,  115,  116,
 117,  118,  120,  121,  122,  122,  123,  124,  125,  125,  126,  126,  126,  127,  127,  127,
 127,  127,  127,  127,  126,  126,  126,  125,  125,  124,  123,  122,  122,  121,  120,  118,
 117,  116,  115,  113,  112,  111,  109,  107,  106,  104,  102,  100,   98,   96,   94,   92,
  90,   88,   85,   83,   81,   78,   76,   73,   71,   68,   65,   63,   60,   57,   54,   51,
  49,   46,   43,   40,   37,   34,   31,   28,   25,   22,   19,   16,   12,    9,    6,    3,
   0,   -3,   -6,   -9,  -12,  -16,  -19,  -22,  -25,  -28,  -31,  -34,  -37,  -40,  -43,  -46,
 -49,  -51,  -54,  -57,  -60,  -63,  -65,  -68,  -71,  -73,  -76,  -78,  -81,  -83,  -85,  -88,
 -90,  -92,  -94,  -96,  -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116,
-117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127,
-127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118,
-117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100,  -98,  -96,  -94,  -92,
 -90,  -88,  -85,  -83,  -81,  -78,  -76,  -73,  -71,  -68,  -65,  -63,  -60,  -57,  -54,  -51,
 -49,  -46,  -43,  -40,  -37,  -34,  -31,  -28,  -25,  -22,  -19,  -16,  -12,   -9,   -6,   -3,
   0,
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

void _ym2149DoSamples() {   // for real-time audio asynchronous with cpu
    WORD *buf = THIS.m_sampleBuffer;
    int ptr = 0;
    do {
        THIS.cyclesB += SOUND_FREQ;
        // psg state machine
        if (THIS.noiseCount >= THIS.noiseLimit) {
            BYTE rbit = (THIS.rnd & 1) ^ ((THIS.rnd >> 2) & 1);
            THIS.rnd = (THIS.rnd >> 1) | ((DWORD)(rbit) << 16);
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
        // samples generation
        while (THIS.cyclesB >= PSG_FREQ) {	// one data at samplerate Hz to output (is psg frequency >> 2)
            WORD dout = 32768;
            THIS.cyclesB -= PSG_FREQ;
            if ((THIS.toneBit0 | THIS.noiseBit0) != 0) {
                BYTE a = (THIS.toneBit0 & THIS.toneLevel0) | (THIS.noiseBit0 & THIS.noiseLevel);
                dout += (a != 0) ? __volTab[THIS.toneVol0] : -__volTab[THIS.toneVol0];
            }
            if ((THIS.toneBit1 | THIS.noiseBit1) != 0) {
                BYTE a = (THIS.toneBit1 & THIS.toneLevel1) | (THIS.noiseBit1 & THIS.noiseLevel);
                dout += (a != 0) ? __volTab[THIS.toneVol1] : -__volTab[THIS.toneVol1];
            }
            if ((THIS.toneBit2 | THIS.noiseBit2) != 0) {
                BYTE a = (THIS.toneBit2 & THIS.toneLevel2) | (THIS.noiseBit2 & THIS.noiseLevel);
                dout += (a != 0) ? __volTab[THIS.toneVol2] : -__volTab[THIS.toneVol2];
            }
            buf[ptr + (ptr & 1 ? -1 : 1)] = dout;
            ptr++;
            if (ptr == SOUND_SAMPLE_BUFFER_SIZE) break;   // leave
        }
    } while (ptr < SOUND_SAMPLE_BUFFER_SIZE);
}

void ym2149DoAudio() {  // for synchronous audio with cpu
    WORD dout = 0;
    BYTE a = 0;
    while (THIS.cyclesA >= THIS.audioCyclesA) {     // times 16 predivisor done
        THIS.cyclesA -= THIS.audioCyclesA;
        THIS.cyclesB += THIS.audioCyclesB;                        // this.audioCyclesB;
        
        if (THIS.noiseCount >= THIS.noiseLimit) {
            BYTE rbit = (THIS.rnd & 1) ^ ((THIS.rnd >> 2) & 1);
            THIS.rnd = (THIS.rnd >> 1) | ((DWORD)(rbit) << 16);
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
    
        if (THIS.cyclesB >= PSG_FREQ) {	// one data at samplerate Hz to output (is psg frequency >> 2)
            THIS.cyclesB -= PSG_FREQ;
            dout = 32768;
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
            // dout += 0.1*Math.sin(this.bufferPtr*2.0*3.14159*440.0/44100.0);
            THIS.buffer[THIS.bufferWritePtr + (THIS.bufferWritePtr & 1 ? -1 : 1)] = dout;
            THIS.bufferWritePtr++;
            if (THIS.bufferWritePtr == THIS.bufferLength) THIS.bufferWritePtr = 0;         // loop back ;)
        }
    }
}

#ifdef ESP_IDF_VERSION
   #define FABGL_ESP_IDF_VERSION_VAL                      ESP_IDF_VERSION_VAL
   #define FABGL_ESP_IDF_VERSION                          ESP_IDF_VERSION
 #else
   #define FABGL_ESP_IDF_VERSION_VAL(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
   #define FABGL_ESP_IDF_VERSION                          FABGL_ESP_IDF_VERSION_VAL(0, 0, 0)
 #endif

void _ym2149I2SInit() {
    i2s_config_t i2s_config;
    i2s_config.mode                 = (i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN);
    i2s_config.sample_rate          = SOUND_FREQ;
    i2s_config.bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT;
    #if FABGL_ESP_IDF_VERSION <= FABGL_ESP_IDF_VERSION_VAL(4, 1, 1)
    i2s_config.communication_format = (i2s_comm_format_t) I2S_COMM_FORMAT_I2S_MSB;
    #else
    i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
    #endif
    i2s_config.channel_format       = I2S_CHANNEL_FMT_ONLY_RIGHT;
    i2s_config.intr_alloc_flags     = 0;
    i2s_config.dma_buf_count        = 3;
    i2s_config.dma_buf_len          = SOUND_SAMPLE_BUFFER_SIZE * sizeof(uint16_t);
    i2s_config.use_apll             = false;
    i2s_config.tx_desc_auto_clear   = 0;
    i2s_config.fixed_mclk           = 0;
    // install and start i2s driver
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    // init DAC pad
    i2s_set_dac_mode(I2S_DAC_CHANNEL_RIGHT_EN); // GPIO25

    THIS.m_play = false;
    THIS.m_sampleRate = SOUND_FREQ;
    THIS.m_sampleBuffer = (uint16_t*) heap_caps_malloc(SOUND_SAMPLE_BUFFER_SIZE * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
}

void IRAM_ATTR _ym2149I2SWaveGenTask(void *arg) {
    YM2149 *self = (YM2149*) arg;

    i2s_set_clk(I2S_NUM_0, self->m_sampleRate, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);

    while (true) {
        // suspend?
        if (self->m_state == RequestToStop || self->m_state == Stop) {
            self->m_state = Stop;
            while (self->m_state == Stop) {
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // wait for "give"
            }
        }
/*
        // mutize output?
        if (self->m_channels == nullptr && muteCyclesCount >= 8) {
            self->m_state = Stop;
            while (self->m_state == Stop) {
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);  // wait for "give"
            }
        }
*/
        self->m_state = Playing;

        if (THIS.audioEnable == 2) {
            _ym2149DoSamples();
        } else if (THIS.audioEnable == 1) {
            int delta = self->bufferWritePtr - self->bufferReadPtr;
            if (delta < 0) delta += self->bufferLength;
            if (delta < SOUND_SAMPLE_BUFFER_SIZE) { // missing samples ??
                self->cyclesA += (SOUND_SAMPLE_BUFFER_SIZE - delta) * self->audioCyclesA * 3;
                // ym2149DoAudio();
            }
            delta = self->bufferWritePtr - self->bufferReadPtr;
            if (delta < 0) delta += self->bufferLength;
            int l = self->bufferLength - self->bufferReadPtr;
            if (l < SOUND_SAMPLE_BUFFER_SIZE) {   // copy in 2 parts;
                memcpy(self->m_sampleBuffer, self->buffer + self->bufferReadPtr, l * sizeof(uint16_t));
                memcpy(self->m_sampleBuffer + l * sizeof(uint16_t), self->buffer, (SOUND_SAMPLE_BUFFER_SIZE - l) * sizeof(uint16_t));
                self->bufferReadPtr = SOUND_SAMPLE_BUFFER_SIZE - l;
            } else {
                memcpy(self->m_sampleBuffer, self->buffer + self->bufferReadPtr, SOUND_SAMPLE_BUFFER_SIZE * sizeof(uint16_t));
                self->bufferReadPtr += SOUND_SAMPLE_BUFFER_SIZE;
            }
        }
        size_t bytes_written;
        i2s_write(I2S_NUM_0, self->m_sampleBuffer, SOUND_SAMPLE_BUFFER_SIZE * sizeof(uint16_t), &bytes_written, portMAX_DELAY);
    }
}

bool _ym2149I2SActualPlaying() {
  return THIS.m_waveGenTaskHandle && THIS.m_state == Playing;
}

bool _ym2149I2SForcePlay(bool value) {
  bool isPlaying = _ym2149I2SActualPlaying();
  if (value) {
    // play
    if (!isPlaying) {
      if (!THIS.m_waveGenTaskHandle)
        xTaskCreate(_ym2149I2SWaveGenTask, "", 2000, &THIS, 5, &THIS.m_waveGenTaskHandle);
      THIS.m_state = RequestToPlay;
      xTaskNotifyGive(THIS.m_waveGenTaskHandle);
    }
  } else {
    // stop
    if (isPlaying) {
      // request task to suspend itself when possible
      THIS.m_state = RequestToStop;
      // wait for task switch to suspend state (TODO: is there a better way?)
      while (THIS.m_state != Stop)
        vTaskDelay(1);
    }
  }
  return isPlaying;
}

void _ym2149I2SMutizeOutput(){
    for (int i = 0; i < SOUND_SAMPLE_BUFFER_SIZE; ++i)
        THIS.m_sampleBuffer[i] = 127 << 8;
    size_t bytes_written;
    for (int i = 0; i < 4; ++i)
        i2s_write(I2S_NUM_0, THIS.m_sampleBuffer, SOUND_SAMPLE_BUFFER_SIZE * sizeof(uint16_t), &bytes_written, portMAX_DELAY);
}

bool ym2149I2SPlay(bool value) {
//    AutoSemaphore autoSemaphore(THIS.m_mutex);
    THIS.m_play = value;
    if (_ym2149I2SActualPlaying() != value) {
        bool r = _ym2149I2SForcePlay(value);
        if (!value)
            _ym2149I2SMutizeOutput();
        return r;
    } else {
        return value;
    }
}

void ym2149I2SStop() {
    ym2149I2SPlay(false);
    vTaskDelete(THIS.m_waveGenTaskHandle);
}
int ym2149Delta() {
    int delta = THIS.bufferWritePtr - THIS.bufferReadPtr;
    if (delta < 0) delta += THIS.bufferLength;
    return delta;
}

void ym2149Init(BYTE mode) {
    
    THIS.audioEnable = mode;    // 2 real time, 1 synchronous
    // this.wl = wl;
    THIS.samplerate = SOUND_FREQ;
    THIS.cyclesA = THIS.cyclesB = 0;
    THIS.audioCyclesA = CPU_FREQ/PSG_FREQ;                  // psg frequency is 62500 Hz
    THIS.audioCyclesB = SOUND_FREQ;
    THIS.bufferLength = SOUND_SAMPLE_BUFFER_SIZE * 6;       // 6 buffers ?
    THIS.buffer = (WORD *)heap_caps_malloc(THIS.bufferLength * sizeof(SWORD), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);      // 3 slots sound buffers ?
    THIS.bufferWritePtr = 0;
    THIS.bufferReadPtr = 0;
    THIS.envelopePeriod = 0;
    THIS.envelopeShape = 0;
    THIS.envCount = THIS.envLimit = THIS.envPos = 0;
    THIS.envTab = (BYTE *) __envShapes[THIS.envelopeShape];   // NOT VERY EFFICIENT !!!!
    THIS.rnd = 1;
    THIS.toneBit0 = THIS.toneBit1 = THIS.toneBit2 = 0;
    THIS.toneCount0 = THIS.toneCount1 = THIS.toneCount2 = 0;
    THIS.tonePeriod0 = THIS.tonePeriod1 = THIS.tonePeriod2 = 0;
    THIS.toneLimit0 = THIS.toneLimit1 = THIS.toneLimit2 = 0;
    THIS.toneLevel0 = THIS.toneLevel1 = THIS.toneLevel2 = 0;
    THIS.amplitude0 = THIS.amplitude1 = THIS.amplitude2 = 0;
    THIS.noiseBit0 = THIS.noiseBit1 = THIS.noiseBit2 = 0;
    THIS.noisePeriod = THIS.noiseCount = THIS.noiseLimit = 0;
    //// Atomics.store(this.states, STATE.READ_INDEX, 0);
    //// Atomics.store(this.states, STATE.RING_BUFFER_LENGTH, this.bufferLength);
    //// this.wl.port.postMessage(this.sharedBuffers);       // initialize worklet
    _ym2149I2SInit();

    if (THIS.audioEnable == 2) {
        ym2149I2SPlay(true);
    }
}

BYTE ym2149Write(BYTE data) {
    THIS.regs[THIS.reg] = data;
    if (THIS.reg == 14) return 1;
    if (THIS.audioEnable == 1) ym2149DoAudio();
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
