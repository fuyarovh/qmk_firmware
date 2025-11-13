// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdint.h>
#include "gpio.h"
#include "info_config.h"
#include "keyboard.h"
#include "keycodes.h"
#include "led_matrix.h"
#include "quantum.h"
#include "quantum_keycodes.h"
#include "raw_hid.h"
#include "suspend.h"
#include "timer.h"
#include "usb_descriptor.h"
#include QMK_KEYBOARD_H

enum keycodes {
    keys = SAFE_RANGE
};
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┬───┬───┬───┬───┬───┬───┐
     * │ A │ B │ C │ D │ E │ F │ G │ H │
     * ├───┼───┼───┼───┼───┼───┼───┼───┤
     * │ A │ B │ C │ D │ E │ F │ G │ H │
     * ├───┼───┼───┼───┼───┼───┼───┼───┤
     * │ A │ B │ C │ D │ E │ F │ G │ H │
     * ├───┼───┼───┼───┼───┼───┼───┼───┤
     * │ A │ B │ C │ D │ E │ F │ G │ H │
     * └───┴───┴───┴───┴───┴───┴───┴───┘
     */
    [0] = LAYOUT_ortho_4x8(
        keys,
        keys + 1,
        keys + 2,
        keys + 3,
        keys + 4,
        keys + 5,
        keys + 6,
        keys + 7,

        keys + 8,
        keys + 9,
        keys + 10,
        keys + 11,
        keys + 12,
        keys + 13,
        keys + 14,
        keys + 15,

        keys + 16,
        keys + 17,
        keys + 18,
        keys + 19,
        keys + 20,
        keys + 21,
        keys + 22,
        keys + 23,

        keys + 24,
        keys + 25,
        keys + 26,
        keys + 27,
        keys + 28,
        keys + 29,
        keys + 30,
        keys + 31
    )
};

void keyboard_pre_init_user() {
    gpio_set_pin_input_high(GP0);
    gpio_set_pin_input_high(GP1);
    gpio_set_pin_input_high(GP2);
    gpio_set_pin_output(GP3);
    gpio_write_pin_high(GP3);
}

uint8_t id(void) {
    return 7
    - gpio_read_pin(GP0)
    - (gpio_read_pin(GP1) << 1)
    - (gpio_read_pin(GP2) << 2);
}
uint8_t led_data[4] = {0};
bool is_calibrating = false;
void suspend_power_down_user(void) {
        led_data[0] = 0;
        led_data[1] = 0;
        led_data[2] = 0;
        led_data[3] = 0;
}
void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (length != RAW_EPSIZE)
        return;
    if (data[0] == 0xFF) {
        uint8_t send_data[RAW_EPSIZE] = {0};
        send_data[0] = 0xFF;
        send_data[1] = id();
        raw_hid_send(send_data, RAW_EPSIZE);
    } else if (data[0] == 0xFE) {
        is_calibrating = true;
        led_data[0] = 0;
        led_data[1] = 0;
        led_data[2] = 0;
        led_data[3] = 0;
        if (data[1] < 4) {
            led_data[data[1]] = 0xFF;
        } else {
            is_calibrating = false;
        }

    } else if (!is_calibrating && data[0] == id() + 1) {
        led_data[0] = data[1];
        led_data[1] = data[2];
        led_data[2] = data[3];
        led_data[3] = data[4];
    }
}
bool led_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    for (uint8_t i = led_min; i <= led_max; ++i) {
        uint8_t value = LED_MATRIX_MAXIMUM_BRIGHTNESS * ((led_data[i/8] & (1 << i % 8)) != 0);
        if (is_calibrating && timer_read() % 1024 > 512) {
            value = 0;
        }
        led_matrix_set_value(i, value);
    }
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode >= keys && record->event.pressed) {
        uint8_t key_number = keycode - keys;
        uint8_t data[RAW_EPSIZE] = {0};
        data[0] = 1;
        data[1] = id();
        data[2] = key_number / 8;
        data[3] = ( key_number % 8 ) + 8 * id();
        raw_hid_send(data,RAW_EPSIZE);
        return false;
    }
    return true;
}
