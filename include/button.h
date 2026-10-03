#include <stdint.h>

enum ButtonPressResult { LongPress, DoubleClick, ShortPress, SoftReset, NoAction };
extern const char *ButtonPressResultNames[4];

ButtonPressResult read_button_presses();

ButtonPressResult read_long_press();

//
// Multi-button input (boards with more than one user button, e.g. the XIAO ePaper EE04).
// Boards that have a single button fall back to the interrupt pin, so callers can use
// these unconditionally.
//

// Logical buttons. EE04 hardware: Back = KEY0/GPIO2, Middle = KEY1/GPIO3, Next = KEY2/GPIO5.
enum ButtonId { ButtonBack, ButtonMiddle, ButtonNext, ButtonCount };

/**
 * @brief Blocking read of a press on one logical button
 * @param button button to read (waits for this button to be released first if still held)
 * @param timeout_ms maximum time to wait for a press; NoAction once elapsed
 * @return LongPress / DoubleClick / SoftReset when the hold duration crosses a threshold,
 *         ShortPress otherwise. NoAction if nothing was pressed within the timeout.
 */
ButtonPressResult read_button_press(ButtonId button, uint32_t timeout_ms);

/**
 * @brief Which buttons were held down, and for how long
 */
struct ButtonHold {
  uint8_t mask;          // bit i set when button i is still physically held
  uint32_t duration_ms;  // how long the press lasted
  bool released;         // true when every pressed button was released before the cutoff
};

/**
 * @brief Read a simultaneous multi-button chord
 * @param timeout_ms how long to wait for the chord to be complete
 * @param required_mask bitmask of buttons that must be held together
 * @param out receives the hold description; untouched when nothing was pressed
 * @return true when @p required_mask was satisfied
 */
bool read_button_chord(uint8_t required_mask, uint32_t timeout_ms, ButtonHold *out);

/**
 * @brief Raw state of every button, bit i = button i held
 */
uint8_t read_buttons_mask();
