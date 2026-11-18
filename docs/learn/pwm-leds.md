# PWM LEDs, pinctrl and the LED API (step 1)

## PWM in one line
The pin switches on and off very fast (here 1 kHz). The LED looks steady, and its brightness is the
**duty cycle**: the share of each period it is on. 50 % duty = about half the light and half the current.

## The nRF52840 PWM peripheral
- 4 instances (`pwm0`..`pwm3`), 4 channels each. All channels of one instance share the period.
- Any GPIO can be a PWM output. The pin is chosen in a **PSEL** register, not fixed in silicon.
- It reads duty values from RAM by itself (EasyDMA), so the CPU does nothing while an LED is lit.
- Zephyr's driver (`zephyr/drivers/pwm/pwm_nrfx.c`, look for `pwm_needed`) **stops the peripheral**
  when every channel is at 0 % or 100 %. So `face_off()` costs nothing extra in sleep.

## pinctrl: why the overlay has real pin numbers
- PWM, I²C, SPI and UART pins go through **pinctrl** nodes: `NRF_PSEL(PWM_OUT0, 1, 8)` = output 0 on P1.08.
- The `&arduino_header` names only work for plain GPIOs (they use `gpio-map`). That's why the
  overlay writes the Arduino name in a comment instead.
- `default` state = running; `sleep` state with `low-power-enable` = pins disconnected for low power.

## pwm-leds and the LED API
- One `pwm-leds` node = one LED device. Its children are LEDs 0, 1, 2, ... in order.
  We have two nodes, `hour_leds` and `minute_leds`, and order the children by bit, so index == bit.
- `pwms = <&pwm0 3 PWM_MSEC(1) PWM_POLARITY_NORMAL>` = instance, channel, period, polarity.
  NORMAL because our pins drive the LED high; the DK's own LEDs are wired the other way (INVERTED).
- `CONFIG_LED=y` is the only line in `prj.conf`. The `pwm-leds` nodes turn on `CONFIG_LED_PWM`,
  and that *selects* `CONFIG_PWM`.
- Code: `led_set_brightness(dev, index, percent)` in `app/src/face.c`.

## Removing a board node
The board used `pwm0` for DK LED1. The overlay removes that with `/delete-node/ &{/pwmleds};` and
`/delete-property/ pwm-led0;` in `aliases`. Check the result in `build/app/zephyr/zephyr.dts`.

## Our own Kconfig option
`app/Kconfig` adds `CONFIG_FACE_BRIGHTNESS` (1–100, default 50) and must end with
`source "Kconfig.zephyr"`. Change it in `prj.conf`, or try it in `west build -t menuconfig`.
