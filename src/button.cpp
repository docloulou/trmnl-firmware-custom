#include "button.h"

#include "config.h"
extern TRMNL_DEVICE *pDevice;

#include <Arduino.h>
#include <config.h>
#include <misc/buzzer.h>

#include "trmnl_log.h"

static unsigned long wait_for_button_release(unsigned long start_time) {
  pinMode(pDevice->interrupt_pin, INPUT);
  while (digitalRead(pDevice->interrupt_pin) == LOW && millis() - start_time < BUTTON_SOFT_RESET_TIME) {
    delay(10);
  }
  if (millis() - start_time >= BUTTON_SOFT_RESET_TIME) {
    buzzer().beepPattern(3, 100, 100);
  }
  return millis() - start_time;
}

static ButtonPressResult classify_press_duration(unsigned long duration) {
  if (duration >= BUTTON_SOFT_RESET_TIME) {
    Log_info("Button time=%lu detected extra-long press", duration);
    return SoftReset;
  } else if (duration > BUTTON_HOLD_TIME) {
    Log_info("Button time=%lu detected long press", duration);
    return LongPress;
  } else if (duration > BUTTON_MEDIUM_HOLD_TIME) {
    Log_info("Button time=%lu detected long press", duration);
    return DoubleClick;
  }
  return NoAction;
}

static ButtonPressResult wait_for_second_press(unsigned long start_time) {
  auto release_time = millis();

  while (millis() - release_time < BUTTON_DOUBLE_CLICK_WINDOW) {
    if (digitalRead(pDevice->interrupt_pin) == LOW) {
      auto second_press_start = millis();
      auto second_duration = wait_for_button_release(second_press_start);

      ButtonPressResult long_press_result = classify_press_duration(second_duration);
      if (long_press_result != NoAction) {
        return long_press_result;
      }

      Log_info("Button time=%lu detected double-click", millis() - start_time);
      return DoubleClick;
    }
    delay(10);
  }

  return ShortPress;
}

static ButtonPressResult classify_button_presses() {
  auto time_start = millis();
  Log_info("Button time=%lu: start", time_start);
  pinMode(pDevice->interrupt_pin, INPUT);
  if (digitalRead(pDevice->interrupt_pin) == HIGH) {
    if (time_start < 2000) {
      Log_info("Button: already released at start (GPIO wakeup), waiting for second press");
      return wait_for_second_press(time_start);
    } else {
      Log_info("Button: waiting for button press");
      while (digitalRead(pDevice->interrupt_pin) == HIGH) {
        delay(10);
      }
      time_start = millis();
    }
  }

  auto press_duration = wait_for_button_release(time_start);

  ButtonPressResult long_press_result = classify_press_duration(press_duration);
  if (long_press_result != NoAction) {
    return long_press_result;
  }

  if (press_duration > 50) {
    Log_info("Button: first press detected, waiting for second press");
    return wait_for_second_press(time_start);
  }

  return NoAction;
}

ButtonPressResult read_button_presses() {
  auto result = classify_button_presses();
  if (result == ShortPress || result == DoubleClick) {
    buzzer().beep(100);
  }
  return result;
}

const char *ButtonPressResultNames[] = {"LongPress", "DoubleClick", "ShortPress", "SoftReset"};

//
// Multi-button input
//

static const uint8_t button_pins[ButtonCount] = {
#if EE04_HAS_MULTI_BUTTON
    EE04_BUTTON_KEY0,  // ButtonBack
    EE04_BUTTON_KEY1,  // ButtonMiddle
    EE04_BUTTON_KEY2,  // ButtonNext
#else
    // Single-button boards: every logical button maps to the one interrupt pin, so
    // callers get consistent behaviour without per-board #ifdefs at every call site.
    0, 0, 0
#endif
};

// Resolve a logical button to a GPIO. On single-button boards every logical button
// resolves to the device interrupt pin, which hw_config_init() has already set.
static uint8_t button_gpio(ButtonId button) {
#if EE04_HAS_MULTI_BUTTON
  if (button < ButtonCount) return button_pins[button];
  return button_pins[ButtonNext];
#else
  (void)button;
  return button_pins[ButtonBack];
#endif
}

static bool button_is_down(ButtonId button) { return digitalRead(button_gpio(button)) == LOW; }

uint8_t read_buttons_mask(void) {
  uint8_t mask = 0;
  for (uint8_t i = 0; i < ButtonCount; i++) {
    if (button_is_down((ButtonId)i)) mask |= (1 << i);
  }
  return mask;
}

static void buttons_configure(void) {
  for (uint8_t i = 0; i < ButtonCount; i++) {
    // The EE04 already carries hardware pull-ups; INPUT_PULLUP keeps the line
    // defined if deep sleep left it floating, and is harmless otherwise.
    pinMode(button_gpio((ButtonId)i), INPUT_PULLUP);
  }
}

// Waits for the button to be released, then classifies the hold. Mirrors
// wait_for_button_release()/classify_press_duration() but per button.
static ButtonPressResult measure_button_hold(ButtonId button) {
  buttons_configure();
  auto start = millis();
  while (button_is_down(button) && millis() - start < BUTTON_SOFT_RESET_TIME) {
    delay(10);
  }
  auto duration = millis() - start;
  if (duration >= BUTTON_SOFT_RESET_TIME) {
    buzzer().beepPattern(3, 100, 100);
    Log_info("Button %d time=%lu detected extra-long press", (int)button, duration);
    return SoftReset;
  }
  if (duration > BUTTON_HOLD_TIME) {
    Log_info("Button %d time=%lu detected long press", (int)button, duration);
    return LongPress;
  }
  return ShortPress;
}

ButtonPressResult read_button_press(ButtonId button, uint32_t timeout_ms) {
  buttons_configure();

  // A GPIO wakeup resumes here with the button still down, and that press is the
  // gesture the user made. Classify it rather than discarding it; only when nothing
  // is held do we wait for a fresh press (up to timeout_ms).
  if (button_is_down(button)) {
    return measure_button_hold(button);
  }

  auto deadline = millis() + timeout_ms;
  while (!button_is_down(button)) {
    if (millis() >= deadline) return NoAction;
    delay(10);
  }

  auto result = measure_button_hold(button);
  if (result == ShortPress) {
    // A second tap inside the window upgrades to DoubleClick, matching the
    // single-button path.
    auto release_time = millis();
    while (millis() - release_time < BUTTON_DOUBLE_CLICK_WINDOW) {
      if (button_is_down(button)) {
        result = measure_button_hold(button) == ShortPress ? DoubleClick : result;
        break;
      }
      delay(10);
    }
    buzzer().beep(100);
  }
  Log_info("Button %d -> %s", (int)button, ButtonPressResultNames[result]);
  return result;
}

bool read_button_chord(uint8_t required_mask, uint32_t timeout_ms, ButtonHold *out) {
  buttons_configure();

  auto deadline = millis() + timeout_ms;
  while ((read_buttons_mask() & required_mask) != required_mask) {
    if (millis() >= deadline) return false;
    delay(10);
  }

  // Chord complete: hold until every button lets go (or the soft-reset cutoff trips).
  auto start = millis();
  auto mask = read_buttons_mask();
  while (mask != 0 && millis() - start < BUTTON_SOFT_RESET_TIME) {
    delay(10);
    mask = read_buttons_mask();
  }

  if (out) {
    out->mask = mask;
    out->duration_ms = millis() - start;
    out->released = (mask == 0);
  }
  Log_info("Chord %02X released=%d after %lu ms", required_mask, (int)(mask == 0), millis() - start);
  return true;
}
