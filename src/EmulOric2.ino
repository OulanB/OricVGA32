#include "Arduino.h"

/*
 * statically allocated ram is limited to ~170KB
 * so ram is allocated on the heap with malloc
 */ 

#pragma GCC optimize("O3")

#include "log.h"
#include "video.h"

#include "ps2controller_d.h"

#include "m6502_d.h"
#include "ula_d.h"
#include "io_d.h"
#include "ym2149_d.h"
#include "tape_d.h"
#include "system_d.h"

SYSTEM System;

#include "keyboard.h"

#include "basic11B.h"

#include "m6502.h"
#include "ula.h"
#include "io.h"
#include "ym2149.h"
#include "tape.h"
#include "system.h"

#include "esp_timer.h"

static void periodic_timer_callback(void* arg);

esp_timer_handle_t periodic_timer;
const esp_timer_create_args_t periodic_timer_args = {
    &periodic_timer_callback,
    0,
    ESP_TIMER_TASK,
    ""
};

unsigned tNew, tPrev, tDelta;
unsigned tN, tSum;

PS2Controller *keyboard;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    delay(200);
    log_d("Start");
    videoInit();
    system_Init(1);
    log_d("ROM0 %08X ROM1 %08X RAM %08X", System.rom0, System.rom1, System.ram);
    tSum = 0;
    tN = 0;
    keyboard = new(PS2Controller);
    keyboard->begin(PS2Preset::KeyboardPort0, KbdMode::NoVirtualKeys);

    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    /* The timer has been created but is not running yet */
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 20000));    
}

BYTE vRefreshCounter = 0;   // for Blinking

void periodic_timer_callback(void *arg) {
    // put your main code here, to run repeatedly:
    tPrev = esp_timer_get_time(); // xthal_get_ccount();

    m6502_doOps(20000);
    ula_display2((vRefreshCounter++ >> 5) & 1);

    if (keyboard->dataAvailable(0)) {
        BYTE scancode = keyboard->getData(0, 1);
//        log_d("got %d", scancode);
        kPutScancode(scancode);
        kDoFSM(); 
    }

    if (tN == 1000) {
        log_d("loop avg %d", tSum / tN);
        tSum = 0;
        tN = 0;
    }
    tDelta = esp_timer_get_time() - tPrev;
    if (tDelta & 0x80000000) tDelta += 0xFFFFFFFF;
    tN++;
    tSum += tDelta;
}

void loop() {
    delay(1000);
}
