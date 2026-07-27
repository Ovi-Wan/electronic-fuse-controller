#include <Arduino.h>
#include "fuse_model.h"
#include "bus_logic.h"
#include "ui_logic.h"

#define PIN_R 2
#define PIN_G 3
#define PIN_B 4

#define PIN_ENABLE 10
#define PIN_RESET  11
#define PIN_NORMAL 12
#define PIN_HEAVY  13
#define PIN_SHORT  14

fuse_t fuse;
bus_command_t cmd;
bus_status_t status;

void fuse_set_color(int r, int g, int b) {
    digitalWrite(PIN_R, r);
    digitalWrite(PIN_G, g);
    digitalWrite(PIN_B, b);
}

void setup() {
    pinMode(PIN_R, OUTPUT);
    pinMode(PIN_G, OUTPUT);
    pinMode(PIN_B, OUTPUT);

    pinMode(PIN_ENABLE, INPUT_PULLUP);
    pinMode(PIN_RESET,  INPUT_PULLUP);
    pinMode(PIN_NORMAL, INPUT_PULLUP);
    pinMode(PIN_HEAVY,  INPUT_PULLUP);
    pinMode(PIN_SHORT,  INPUT_PULLUP);

    fuse_init(&fuse);
}

void loop() {
    cmd.cmd_enable = digitalRead(PIN_ENABLE) == LOW;
    cmd.cmd_reset  = digitalRead(PIN_RESET)  == LOW;

    if (digitalRead(PIN_NORMAL) == LOW) cmd.load_profile = 0;
    if (digitalRead(PIN_HEAVY)  == LOW) cmd.load_profile = 1;
    if (digitalRead(PIN_SHORT)  == LOW) cmd.load_profile = 2;

    bus_process(&fuse, &cmd, &status);
    ui_update_leds(&status);

    delay(100);
}
