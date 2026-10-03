#include <Arduino.h>
#include <config.h>
#ifndef BOARD_TRMNL_X
extern TRMNL_DEVICE *pDevice;
#endif

void pins_init(void) {
  gpio_config_t io_conf = {};
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
#if EE04_HAS_MULTI_BUTTON
  // All three user buttons (KEY0/KEY1/KEY2), so every line is defined before the first
  // button read and stays pulled high across wakeups.
  io_conf.pin_bit_mask = (1ULL << EE04_BUTTON_KEY0) | (1ULL << EE04_BUTTON_KEY1) | (1ULL << EE04_BUTTON_KEY2);
#else
#ifdef BOARD_TRMNL_X
  io_conf.pin_bit_mask = (1ULL << PIN_INTERRUPT);
#else
  io_conf.pin_bit_mask = (1ULL << pDevice->interrupt_pin);
#endif
#endif
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);
}
