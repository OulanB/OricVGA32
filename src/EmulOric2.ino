#include "Arduino.h"

/*
 * statically allocated ram is limited to ~170KB
 * so ram is allocated on the heap with malloc
 */ 

#pragma GCC optimize("O3")

#include "log.h"

#include "video.h"

#include "system_d.h"

SYSTEM System;

#include "esp_timer.h"
#include "freertos/task.h"
#include "sdfat.h"
#include "spifat.h"

#include "keyboard.h"

#include "basic11b.h"
#include "basic11bP.h"
#include "charset.h"

#include "m6502.h"
#include "ula.h"
#include "io.h"
#include "ym2149.h"
#include "tape.h"
#include "system.h"

#include "osd.h"

void periodic_timer_callback(void *arg) {
    // put your main code here, to run repeatedly:
    System.tPrev = esp_timer_get_time(); // xthal_get_ccount();

    System.nSum += m6502DoOps(20000);
    if (System.Osd.display) {
        osdDisplay2((System.vSyncCount++ >> 5) & 1);
        osdTopDisplay2(0);
        osdBottomDisplay2(0);
//        osdFsm();
    } else {
        if (System.Osd.topDisplay) {
            osdTopDisplay2(0);
            osdBottomDisplay2(0);
        }
        ulaDisplay2((System.vSyncCount++ >> 5) & 1);
    }
    while (System.ps2keybd->dataAvailable(0)) {
        BYTE scancode = System.ps2keybd->getData(0, 1);
//        log_d("got %d", scancode);
        kPutScancode(scancode);
        kDoFSM(); 
    }

    if (System.tN == 200) {
        // log_d("loop avg %d %d", tSum / tN, nSum / tN);
        System.tSum = 0;
        System.tN = 0;
        System.nSum = 0;
    }
    System.tDelta = esp_timer_get_time() - System.tPrev;
    if (System.tDelta & 0x80000000) System.tDelta += 0xFFFFFFFF;
    System.tN++;
    System.tSum += System.tDelta;
}

void taskCore1(void *parameters) {
    System.periodic_timer_args.callback = &periodic_timer_callback;
    System.periodic_timer_args.dispatch_method = ESP_TIMER_TASK;
    System.periodic_timer_args.name = "Per-Cpu";
    ESP_ERROR_CHECK(esp_timer_create(&System.periodic_timer_args, &System.periodic_timer));
    /* The timer has been created but is not running yet */
    ESP_ERROR_CHECK(esp_timer_start_periodic(System.periodic_timer, 20000));    
    
    for( ;; ) {
        yield();
        delay(100);
    }
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(200);
    log_d("Start");
    videoInit();
    tapeInit();
    sdFatInit();
    spiFatInit();
    osdInit("Please mount an Fs");       // at root of

/*
    Serial.println("Init Micro SD card Fat FS");
    sdFatInit();
    Serial.println("Init SpiFlash Fat FS");
    spiFatInit();
*/

/*
    FFat.mkdir("/Tapes");
    FFat.mkdir("/Disks");
    FFat.mkdir("/Tapes/Games");
    FFat.mkdir("/Tapes/Demos");
    FFat.mkdir("/Tapes/Languages");
*/
    /* 
     * to initialize spi fat from sd fat
     */
    /*
    #ifdef FILESYSSDCARD
        spiFatInit();
    #else 
        sdFatInit();
    #endif
     fileCopySdFatToSpiFat("/Oric/tapes", "/");
    */
    log_d("Total heap: %d", ESP.getHeapSize());
    log_d("Free heap: %d", ESP.getFreeHeap());
    log_d("Total PSRAM: %d", ESP.getPsramSize());
    log_d("Free PSRAM: %d", ESP.getFreePsram()); 

    systemInit(1, 1);  // atmos quick read patch

    // tapeLoadSD("/Oric/tapes/Defense Force (19xx)(Tansoft).tap");
    // tapeLoadSPI("/Defense Force (19xx)(Tansoft).tap");

    log_d("ROM0 %08X ROM1 %08X RAM %08X", System.rom0, System.rom1, System.ram);
    System.tSum = 0;
    System.tN = 0;
    System.ps2keybd = new(PS2Controller);
    System.ps2keybd->begin(PS2Preset::KeyboardPort0, KbdMode::NoVirtualKeys);

    xTaskCreatePinnedToCore(taskCore1, "ORIC-1", 10240, &System.paramCore1, 1, &System.xHandleCore1, 1);

//    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    /* The timer has been created but is not running yet */
//    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 20000));    
}

void loop() {

    for(;;) {
        if (System.Osd.display) {
            osdFsm();
            delay(10);
        } else {
            delay(100);
            yield();
        }
    }
}
