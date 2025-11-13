// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#include "keyboard.h"
#include "keycodes.h"
#include "quantum_keycodes.h"
#include QMK_KEYBOARD_H

extern midi_config_t midi_config;


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    /*
     * ┌───┬───┬───┬───┐
     * │TG1│ / │ * │ - │
     * ├───┼───┼───┼───┤
     * │ 7 │ 8 │ 9 │ + │
     * ├───┼───┼───┼───┤
     * │ 4 │ 5 │ 6 │ % │
     * ├───┼───┼───┼───┤
     * │ 1 │ 2 │ 3 │ = │
     * ├───┼───┼───┼───┤
     * │ 0 │00 │ . │Ent│
     * └───┴───┴───┴───┘
     */
    [0] = LAYOUT_ortho_5x4(MI_C, MI_Cs, MI_D, MI_Ds,

                           MI_E, MI_F, MI_Fs, MI_G,

                           MI_Gs, MI_A, MI_As, MI_B,

                           MI_C1, MI_Cs1, MI_D1, MI_Ds1,

                           MI_E1, MI_F1, MI_Fs1, MI_G3)};

void keyboard_post_init_user(void) {
    midi_config.octave = QK_MIDI_OCTAVE_1 - MIDI_OCTAVE_MIN;
}
