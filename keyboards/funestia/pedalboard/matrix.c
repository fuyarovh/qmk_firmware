
#include "matrix.h"
#include "atomic_util.h"
#include "gpio.h"
#include "pin_defs.h"
#define ROWS_PER_HAND (MATRIX_ROWS)
#ifdef MATRIX_ROW_PINS_RIGHT
#    define SPLIT_MUTABLE_ROW
#else
#    define SPLIT_MUTABLE_ROW const
#endif
#ifdef MATRIX_COL_PINS_RIGHT
#    define SPLIT_MUTABLE_COL
#else
#    define SPLIT_MUTABLE_COL const
#endif

static SPLIT_MUTABLE_ROW pin_t row_pins[ROWS_PER_HAND] = MATRIX_ROW_PINS;
static SPLIT_MUTABLE_COL pin_t col_pins[MATRIX_COLS]   = MATRIX_COL_PINS;

static inline void gpio_atomic_set_pin_output_high(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_output(pin);
        gpio_write_pin_high(pin);
    }
}
static inline void gpio_atomic_set_pin_input_high(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_input_high(pin);
    }
}
static inline void gpio_atomic_set_pin_input_low(pin_t pin) {
    ATOMIC_BLOCK_FORCEON {
        gpio_set_pin_input_low(pin);
    }
}
static inline uint8_t readMatrixPin(pin_t pin) {
    if (pin != NO_PIN) {
        return (gpio_read_pin(pin) == MATRIX_INPUT_PRESSED_STATE) ? 0 : 1;
    } else {
        return 1;
    }
}
static bool select_col(uint8_t col) {
    pin_t pin = col_pins[col];
    if (pin != NO_PIN) {
        gpio_atomic_set_pin_output_high(pin);
        return true;
    }
    return false;
}
static void unselect_col(uint8_t col) {
    pin_t pin = col_pins[col];
    if (pin != NO_PIN) {
        gpio_atomic_set_pin_input_low(pin);
    }
}

static void unselect_cols(void) {
    for (uint8_t x = 0; x < MATRIX_COLS; x++) {
        unselect_col(x);
    }
}
void matrix_init_pins(void) {
    unselect_cols();
    for (uint8_t x = 0; x < ROWS_PER_HAND; x++) {
        if (row_pins[x] != NO_PIN) {
            gpio_atomic_set_pin_input_high(row_pins[x]);
        }
    }
}

void matrix_read_rows_on_col(matrix_row_t current_matrix[], uint8_t current_col, matrix_row_t row_shifter) {
    bool key_pressed = false;

    // Select col
    if (!select_col(current_col)) { // select col
        return;                     // skip NO_PIN col
    }
    matrix_output_select_delay();

    // For each row...
    for (uint8_t row_index = 0; row_index < ROWS_PER_HAND; row_index++) {
        // Check row pin state
        if (readMatrixPin(row_pins[row_index]) == 0) {
            // Pin LO, set col bit
            current_matrix[row_index] |= row_shifter;
            key_pressed = true;
        } else {
            // Pin HI, clear col bit
            current_matrix[row_index] &= ~row_shifter;
        }
    }

    // Unselect col
    unselect_col(current_col);
    matrix_output_unselect_delay(current_col, key_pressed); // wait for all Row signals to go HIGH
}
