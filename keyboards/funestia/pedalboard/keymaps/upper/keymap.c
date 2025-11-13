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
    [0] = LAYOUT_ortho_5x4(MI_F1, MI_Fs1, MI_G1, MI_Gs1,

                           MI_A1, MI_As1, MI_B1, MI_C2,

                           MI_Cs2, MI_D2, MI_Ds2, MI_E2,

                           MI_F2, MI_Fs2, MI_G2, MI_Gs2,

                           MI_A2, MI_As2, MI_B2, MI_C3

                           )};

void keyboard_post_init_user(void) {
    midi_config.octave = QK_MIDI_OCTAVE_1 - MIDI_OCTAVE_MIN;
}
