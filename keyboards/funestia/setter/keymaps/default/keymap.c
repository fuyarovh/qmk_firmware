// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "_wait.h"
#include "gpio.h"
#include "info_config.h"
#include "keyboard.h"
#include "keycodes.h"
#include "quantum.h"
#include "quantum_keycodes.h"
#include "raw_hid.h"
#include "suspend.h"
#include "usb_descriptor.h"
#include QMK_KEYBOARD_H
enum keycodes {
    zero = SAFE_RANGE,
    calibration,
    left,
    setleft,
    right,
    setright,
    letter,
    setletter = letter + 8,
    number = setletter + 8,
    setnumber = number + 8,
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT_ortho_5x4(
        MO(1), zero, left, right,
        letter, letter + 1, number, number + 1,
        letter + 2, letter + 3, number + 2, number + 3,
        letter + 4, letter + 5, number + 4, number + 5,
        letter + 6, letter + 7, number + 6, number + 7
    ),
    [1] = LAYOUT_ortho_5x4(
        KC_TRNS, calibration, setleft, setright,
        setletter, setletter + 1, setnumber, setnumber + 1,
        setletter + 2, setletter + 3, setnumber + 2, setnumber + 3,
        setletter + 4, setletter + 5, setnumber + 4, setnumber + 5,
        setletter + 6, setletter + 7, setnumber + 6, setnumber + 7
    )
};
uint8_t id(void) {
    return 0x7F;
}

uint8_t letter_buffer = 0;
uint8_t number_current = 0;
uint8_t letter_current = 0;
uint8_t cathode_rows[8] = {
    GP11,
    GP10,
    GP18,
    GP19,

    GP7,
    GP6,
    GP5,
    GP4
};

uint8_t anode_cols[4] = {
    GP13,
    GP12,

    GP9,
    GP8
};

void update_leds(void) {
    uint8_t letter_cat = letter_current/2;
    uint8_t letter_anode = letter_current%2;
    uint8_t number_cat = 4 + number_current/2;
    uint8_t number_anode = 2 + number_current%2;
    for (size_t i = 0; i!= 4; ++i) {
        gpio_write_pin_low(anode_cols[i]);
    }

    for (size_t i = 0; i!= 8; ++i) {
        gpio_write_pin_high(cathode_rows[i]);
    }
    wait_ms(1);
    gpio_write_pin_low(cathode_rows[letter_cat]);
    gpio_write_pin_low(cathode_rows[number_cat]);
    gpio_write_pin_high(anode_cols[number_anode]);
    gpio_write_pin_high(anode_cols[letter_anode]);

}
void keyboard_pre_init_user() {
    gpio_set_pin_output(GP28); //usb relay
    gpio_write_pin_high(GP28);
    for (size_t i = 0; i!= 4; ++i) {
        gpio_set_pin_output(anode_cols[i]);
    }

    for (size_t i = 0; i!= 8; ++i) {
        gpio_set_pin_output(cathode_rows[i]);
    }
}

bool init = false;
void suspend_power_down_user(void) {
    gpio_write_pin_high(GP28);
    //turn off leds
    for (size_t i = 0; i!= 4; ++i) {
        gpio_write_pin_low(anode_cols[i]);
    }

    for (size_t i = 0; i!= 8; ++i) {
        gpio_write_pin_high(cathode_rows[i]);
    }
    init = false;
}
void keyboard_post_init_user() {
    update_leds();
}
void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (length != RAW_EPSIZE)
        return;
    if (data[0] == 0xFF) {
        uint8_t send_data[RAW_EPSIZE] = {0};
        if (!init) {
            gpio_write_pin_low(GP28);
        }
        init = true;
        send_data[0] = 0xFF;
        send_data[1] = id();
        raw_hid_send(send_data, RAW_EPSIZE);
    } else if (data[0] == 1) {
        letter_current = data[1];
        letter_buffer = data[1];
        number_current = data[2];
        update_leds();
    }
}
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode >= SAFE_RANGE && record->event.pressed) {
        uint8_t data[RAW_EPSIZE] = {0};
        data[1] = id();
        if (keycode == zero) {
            data[0] = 2; //Clear
        } else if (keycode == calibration) {
            data[0] = 5;
        } else if (keycode == left) {
            data[0] = 3;
            data[2] = 0xFF;
            data[3] = 0;
        } else if (keycode == right) {
            data[0] = 3;
            data[2] = 1;
            data[3] = 0;
        } else if (keycode == setleft) {
            data[0] = 3;
            data[2] = -1;
            data[3] = 0;
        } else if (keycode == setright) {
            data[0] = 3;
            data[2] = 1;
            data[3] = 1;
        } else if (keycode >= setletter && keycode < setletter + 8) {
            letter_buffer = keycode - setletter;
        } else if (keycode >= setnumber && keycode < setnumber + 8) {
            data[0] = 6;
            data[2] = letter_buffer;
            data[3] = keycode - setnumber;
        } else if (keycode >= number && keycode < number + 8) {
            data[0] = 4;
            data[2] = letter_current;
            data[3] = keycode - number;
        } else if (keycode >= letter && keycode < letter + 8) {
            data[0] = 4;
            data[2] = keycode - letter;
            data[3] = 0;
        } else {
            return true;
        }
        raw_hid_send(data,RAW_EPSIZE);
        return false;
    }
    return true;
}
