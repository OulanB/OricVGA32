/*
 * Video for emulators
 */

// #define L320
#ifdef L320
// 320*240 
#define HFRONT          8
#define HSYNC           48
#define HBACK           24
#define HRES            320
#define VFRONT          11
#define VSYNC           2
#define VBACK           31
#define VRES            480
#define VDIV            2
#define PIXELCLOCK      12587500
#define HSYNCPOLARITY   1
#define VSYNCPOLARITY   1
#else
// 256*240
#define HFRONT          12 // 8
#define HSYNC           44 // 32
#define HBACK           24 // 40
#define HRES            256
#define VFRONT          11 // 4
#define VSYNC           2 // 8
#define VBACK           31 // 6
#define VRES            480
#define VDIV            2
#define PIXELCLOCK      10573500 // 12549000
#define HSYNCPOLARITY   1
#define VSYNCPOLARITY   1
#endif

#define BITCOUNT 8

#define INACTIVESAMPLES ((HFRONT+HSYNC+HBACK+3) & 0xFFFFFFFC)

#define USEINTERRUPT false

#include <soc/rtc.h>
#include <driver/rtc_io.h>

#include "rom/lldesc.h"
#include "esp_heap_caps.h"
#include "soc/soc.h"
#include "soc/gpio_sig_map.h"
#include "soc/i2s_reg.h"
#include "soc/i2s_struct.h"
#include "soc/io_mux_reg.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"

typedef lldesc_t DMABuffer_t;

uint8_t _frameBuffer[HRES*VRES/VDIV + 4]; // here is the displayed frame buffer ;)
uint8_t _vSyncInactiveBuffer[INACTIVESAMPLES];
uint8_t _vSyncActiveBuffer[HRES];
uint8_t _inactiveBuffer[INACTIVESAMPLES];
uint8_t _blankActiveBuffer[HRES];

#define DMADESCCOUNT ((VFRONT+VSYNC+VBACK+VRES)*2)
DMABuffer_t _dmaDesc[DMADESCCOUNT];

#define I2SDEV I2S1
#define I2SINDEX 1

#define VSYNCPIN 23
#define HSYNCPIN 15
const int _pinMap[8] = {4, 5, 18, 19, 21, 22, VSYNCPIN, HSYNCPIN};

struct {
    // I2S Part
    intr_handle_t interruptHandle;
    volatile bool stopSignal;
    void (*interruptStaticChild)(void *arg) = 0;

    // video Part
    long vsyncBit;
    long hsyncBit;
    long vsyncBitI;
    long hsyncBitI;
    long SBits;
  
    int totalLines;
    volatile bool vSyncPassed;  
} Video;

void video_i2s_create() {
    const periph_module_t deviceModule[] = {PERIPH_I2S0_MODULE, PERIPH_I2S1_MODULE};
    //enable I2S peripheral
    periph_module_enable(deviceModule[I2SINDEX]);
    Video.interruptHandle = 0;
    Video.stopSignal = false;
}

void video_i2s_reset() {
    const unsigned long lc_conf_reset_flags = I2S_IN_RST_M | I2S_OUT_RST_M | I2S_AHBM_RST_M | I2S_AHBM_FIFO_RST_M;
    I2SDEV.lc_conf.val |= lc_conf_reset_flags;
    I2SDEV.lc_conf.val &= ~lc_conf_reset_flags;

    const uint32_t conf_reset_flags = I2S_RX_RESET_M | I2S_RX_FIFO_RESET_M | I2S_TX_RESET_M | I2S_TX_FIFO_RESET_M;
    I2SDEV.conf.val |= conf_reset_flags;
    I2SDEV.conf.val &= ~conf_reset_flags;
    while (I2SDEV.state.rx_fifo_reset_back)
        ;
}

void video_i2s_i2sStop() {
    esp_intr_disable(Video.interruptHandle);
    video_i2s_reset();
    I2SDEV.conf.rx_start = 0;
    I2SDEV.conf.tx_start = 0;
}

void video_i2s_stop() {
    Video.stopSignal = true;
    while (Video.stopSignal)
        ;
}

void video_i2s_startTX() {
    // DEBUG_PRINTLN("I2S TX");
    esp_intr_disable(Video.interruptHandle);
    video_i2s_reset();
    I2SDEV.lc_conf.val = I2S_OUT_DATA_BURST_EN | I2S_OUTDSCR_BURST_EN;
    I2SDEV.out_link.addr = (uint32_t)(&_dmaDesc[0]);
    I2SDEV.out_link.start = 1;
    I2SDEV.int_clr.val = I2SDEV.int_raw.val;
    I2SDEV.int_ena.val = 0;
      if (USEINTERRUPT) {
        I2SDEV.int_ena.out_eof = 1;
        //enable interrupt
        esp_intr_enable(Video.interruptHandle);
    }
    //start transmission
    I2SDEV.conf.tx_start = 1;
}

void video_i2s_resetDMA() {
    I2SDEV.lc_conf.in_rst = 1;
    I2SDEV.lc_conf.in_rst = 0;
    I2SDEV.lc_conf.out_rst = 1;
    I2SDEV.lc_conf.out_rst = 0;
}

void video_i2s_resetFIFO() {
    I2SDEV.conf.rx_fifo_reset = 1;
    I2SDEV.conf.rx_fifo_reset = 0;
    I2SDEV.conf.tx_fifo_reset = 1;
    I2SDEV.conf.tx_fifo_reset = 0;
}

static void IRAM_ATTR video_i2s_interruptStatic(void *arg) {
    //i2s object not safely accesed in DRAM or IRAM
    //i2s.int_clr.val = i2s.int_raw.val;
    //using REG_WRITE to clear the interrupt instead
    //note: there are still other alternatives, see i2s driver .c file
    //inside the i2s_intr_handler_default() function
    REG_WRITE(I2S_INT_CLR_REG(I2SINDEX), (REG_READ(I2S_INT_RAW_REG(I2SINDEX)) & 0xffffffc0) | 0x3f);
    //the call to the overloaded (or any) non-static member function definitely breaks the IRAM rule
    // causing an exception when concurrently accessing the flash (or flash-filesystem) or wifi
    //the reason is unknown but probably related with the compiler instantiation mechanism
    //(note: defining the code of the [member] interrupt function outside the class declaration,
    // and with IRAM flag does not avoid the crash)
    //((I2S *)arg)->interrupt();

    if (Video.interruptStaticChild)
        Video.interruptStaticChild(arg);
}

bool video_i2s_initParallelOutputMode() {
    //route peripherals
    //in parallel mode only upper 16 bits are interesting in this case
    const int deviceBaseIndex[] = {I2S0O_DATA_OUT0_IDX, I2S1O_DATA_OUT0_IDX};
    const int deviceClockIndex[] = {I2S0O_BCK_OUT_IDX, I2S1O_BCK_OUT_IDX};
    const int deviceWordSelectIndex[] = {I2S0O_WS_OUT_IDX, I2S1O_WS_OUT_IDX};
    const periph_module_t deviceModule[] = {PERIPH_I2S0_MODULE, PERIPH_I2S1_MODULE};
    //works only since indices of the pads are sequential
    for (int i = 0; i < BITCOUNT; i++) {
        if (_pinMap[i] > -1) {
            PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[_pinMap[i]], PIN_FUNC_GPIO);
            gpio_set_direction((gpio_num_t)_pinMap[i], (gpio_mode_t)GPIO_MODE_DEF_OUTPUT);
            //rtc_gpio_set_drive_capability((gpio_num_t)pinMap[i], (gpio_drive_cap_t)GPIO_DRIVE_CAP_3 );
            if (I2SINDEX == 1) {
                gpio_matrix_out(_pinMap[i], deviceBaseIndex[I2SINDEX] + i, false, false);
            }
        }
    }

    //enable I2S peripheral
    periph_module_enable(deviceModule[I2SINDEX]);

    //reset i2s
    I2SDEV.conf.tx_reset = 1;
    I2SDEV.conf.tx_reset = 0;
    I2SDEV.conf.rx_reset = 1;
    I2SDEV.conf.rx_reset = 0;

    video_i2s_resetFIFO();
    video_i2s_resetDMA();

    //parallel mode
    I2SDEV.conf2.val = 0;
    I2SDEV.conf2.lcd_en = 1;
    //from technical datasheet figure 64
    I2SDEV.conf2.lcd_tx_wrx2_en = 1;
    I2SDEV.conf2.lcd_tx_sdx2_en = 0;

    I2SDEV.sample_rate_conf.val = 0;
    I2SDEV.sample_rate_conf.tx_bits_mod = BITCOUNT;
    //clock setup
    int clockN = 2, clockA = 1, clockB = 0, clockDiv = 1;
    if (PIXELCLOCK > 0) {
        //xtal is 40M
        //chip revision 0
        //fxtal * (sdm2 + 4) / (2 * (odir + 2))
        //chip revision 1
        //fxtal * (sdm2 + (sdm1 / 256) + (sdm0 / 65536) + 4) / (2 * (odir + 2))
        //fxtal * (sdm2 + (sdm1 / 256) + (sdm0 / 65536) + 4) needs to be btween 350M and 500M
        //rtc_clk_apll_enable(enable, sdm0, sdm1, sdm2, odir);
        //                           0-255 0-255  0-63  0-31
        //sdm seems to be simply a fixpoint number with 16bits fractional part
        //freq = 40000000L * (4 + sdm) / (2 * (odir + 2))
        //sdm = freq / (20000000L / (odir + 2)) - 4;
        long freq = PIXELCLOCK * 2 * (BITCOUNT / 8);
        log_d("freq : %d", freq);
        long sdmn;
        int odir = -1;
        long fnum, delta, nfreq;
        long eps = 1000000L;
        long bestsdm = 0;
        long bestodir = -1;
        do {
            odir++;
            sdmn = (long) (((double)(freq) / (20000000.0 / (odir + 2)) - 4.0) * 65536.0);
            fnum = (long) (40000000.0 * (((double)(sdmn)/65536.0) + 4.0));
            nfreq = fnum/(2*(odir+2));
            delta = abs(freq-nfreq);
            if ((fnum > 350000000L) && (fnum < 500000000L) && (sdmn < 0x400000L) && (sdmn > 0)) {
                log_d("odir %d fnum %d nfreq %d sdm %d delta %d", odir, fnum, nfreq, sdmn, delta);
                if (delta < eps) {
                    bestsdm = sdmn;
                    bestodir = odir;
                    eps = delta;
                }
            }
        } while (odir <= 31);
        log_d("sdm %d + %d/65536 odir %d", bestsdm >> 16, (bestsdm & 65535), bestodir);
        double f = (40000000L * (bestsdm / 65536.0+ 4)) / (2*(bestodir+2));
        log_d("give freq of %f", f);
        rtc_clk_apll_enable(true, bestsdm & 255, (bestsdm >> 8) & 255, bestsdm >> 16, bestodir);
    } else {
    }

    I2SDEV.clkm_conf.val = 0;
    I2SDEV.clkm_conf.clka_en = PIXELCLOCK > 0 ? 1 : 0;
    I2SDEV.clkm_conf.clkm_div_num = clockN;
    I2SDEV.clkm_conf.clkm_div_a = clockA;
    I2SDEV.clkm_conf.clkm_div_b = clockB;
    I2SDEV.sample_rate_conf.tx_bck_div_num = clockDiv;

    I2SDEV.fifo_conf.val = 0;
    I2SDEV.fifo_conf.tx_fifo_mod_force_en = 1;
    I2SDEV.fifo_conf.tx_fifo_mod = 1;  //byte packing 0A0B_0B0C = 0, 0A0B_0C0D = 1, 0A00_0B00 = 3,
    I2SDEV.fifo_conf.tx_data_num = 32; //fifo length
    I2SDEV.fifo_conf.dscr_en = 1;    //fifo will use dma

    I2SDEV.conf1.val = 0;
    I2SDEV.conf1.tx_stop_en = 0;
    I2SDEV.conf1.tx_pcm_bypass = 1;

    I2SDEV.conf_chan.val = 0;
    I2SDEV.conf_chan.tx_chan_mod = 1;

    //high or low (stereo word order)
    I2SDEV.conf.tx_right_first = 1;

    I2SDEV.timing.val = 0;

    //clear serial mode flags
    I2SDEV.conf.tx_msb_right = 0;
    I2SDEV.conf.tx_msb_shift = 0;
    I2SDEV.conf.tx_mono = 0;
    I2SDEV.conf.tx_short_sync = 0;

    //allocate disabled i2s interrupt
    const int interruptSource[] = {ETS_I2S0_INTR_SOURCE, ETS_I2S1_INTR_SOURCE};
    if (USEINTERRUPT) {
        esp_intr_alloc(interruptSource[I2SINDEX], 
                        ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM, 
                        &video_i2s_interruptStatic, &Video, &Video.interruptHandle);
    }
    return true;
}

void video_initSyncBits() {
    Video.hsyncBitI = HSYNCPOLARITY ? 0x40 : 0;
    Video.vsyncBitI = VSYNCPOLARITY ? 0x80 : 0;
    Video.hsyncBit = Video.hsyncBitI ^ 0x40;
    Video.vsyncBit = Video.vsyncBitI ^ 0x80;
    Video.SBits = Video.hsyncBitI | Video.vsyncBitI;
}

void video_fillDescriptors() {
    memset(_vSyncActiveBuffer, 0, HRES);
    memset(_blankActiveBuffer, 0, HRES);
    memset(_inactiveBuffer, 0, INACTIVESAMPLES);
    memset(_vSyncInactiveBuffer, 0, INACTIVESAMPLES);
    for (int i = 0; i < INACTIVESAMPLES; i++) {
        if (i >= HFRONT && i < (HFRONT + HSYNC)) {
            _vSyncInactiveBuffer[i ^ 2] = Video.hsyncBit | Video.vsyncBit;
            _inactiveBuffer[i ^ 2] = Video.hsyncBit | Video.vsyncBitI;
        } else {
            _vSyncInactiveBuffer[i ^ 2] = Video.hsyncBitI | Video.vsyncBit;
            _inactiveBuffer[i ^ 2] = Video.hsyncBitI | Video.vsyncBitI;
        }
    }
    for (int i = 0; i < HRES; i++) {
        _vSyncActiveBuffer[i ^ 2] = Video.hsyncBitI | Video.vsyncBit;
        _blankActiveBuffer[i ^ 2] = Video.hsyncBitI | Video.vsyncBitI;
    }
	for (int i = 0; i < DMADESCCOUNT; i++) {
        _dmaDesc[i].owner = 1;
        _dmaDesc[i].sosf = 0;
        _dmaDesc[i].offset = 0;
        _dmaDesc[i].eof = 1;
		_dmaDesc[i].qe.stqe_next = &_dmaDesc[(i + 1) % DMADESCCOUNT];
    }
	int d = 0;
	for (int i = 0; i < VFRONT; i++) {
        _dmaDesc[d].length = INACTIVESAMPLES; 
        _dmaDesc[d].size = INACTIVESAMPLES; 
		_dmaDesc[d++].buf = _inactiveBuffer;
        _dmaDesc[d].length = HRES; 
        _dmaDesc[d].size = HRES; 
		_dmaDesc[d++].buf = _blankActiveBuffer;
	}
	for (int i = 0; i < VSYNC; i++) {
        _dmaDesc[d].length = INACTIVESAMPLES; 
        _dmaDesc[d].size = INACTIVESAMPLES; 
		_dmaDesc[d++].buf = _vSyncInactiveBuffer;
        _dmaDesc[d].length = HRES; 
        _dmaDesc[d].size = HRES; 
		_dmaDesc[d++].buf = _vSyncActiveBuffer;
	}
	for (int i = 0; i < VBACK; i++) {
        _dmaDesc[d].length = INACTIVESAMPLES; 
        _dmaDesc[d].size = INACTIVESAMPLES; 
		_dmaDesc[d++].buf = _inactiveBuffer;
        _dmaDesc[d].length = HRES; 
        _dmaDesc[d].size = HRES; 
		_dmaDesc[d++].buf = _blankActiveBuffer;
	}

	for (int i = 0; i < VRES; i++) {
        _dmaDesc[d].length = INACTIVESAMPLES; 
        _dmaDesc[d].size = INACTIVESAMPLES; 
		_dmaDesc[d++].buf = _inactiveBuffer;
        _dmaDesc[d].length = HRES; 
        _dmaDesc[d].size = HRES; 
		_dmaDesc[d++].buf = _frameBuffer + HRES*(i/VDIV);
        memset(_frameBuffer + HRES*(i/VDIV), /*((i>>2) & 63) |*/ Video.SBits, HRES);
	}
}

bool videoInit() {
    video_i2s_create();
	video_initSyncBits();
	video_fillDescriptors();
	video_i2s_initParallelOutputMode();
	video_i2s_startTX();
	return true;
}
