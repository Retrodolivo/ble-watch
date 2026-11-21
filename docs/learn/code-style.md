# Code style: clang-format and naming

- **clang-format** reads the nearest `.clang-format` file, walking up from the source file.
  `app/src/.clang-format` sets the style for our C code. `app/boards/.clang-format` and
  `hw/.clang-format` contain `DisableFormat: true`, so overlays and atopile files are left alone.
- Run it: `clang-format -i app/src/*.c app/src/*.h` (the NCS venv has it:
  `~/ncs/v3.4.2/.venv/bin/clang-format`). VS Code can also format on save with the same file.
- Our names differ from Zephyr's snake_case on purpose:
  - functions: `camelCase`, public ones start with the module: `faceShowBits()`, `clockSet()`, `accelInit()`
  - `const` variables: `UpperCamelCase`: `HourLeds`, `DoubleTapTrig`, `CtsCb`
  - macros: `UPPER_CASE`
- What we can't rename: Zephyr functions (`k_msleep`), struct fields set in initialisers
  (`.connected = onConnected`), and devicetree labels (`DT_NODELABEL(hour_leds)`): those
  names come from Zephyr or from the overlay, not from our code.
