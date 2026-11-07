# Debugging with J-Link and GDB

## Pieces
- **J-Link OB** on the DK talks to the nRF52840 over **SWD** (2 wires: clock + data).
- **JLinkGDBServer** (SEGGER) turns that into a GDB server on localhost:2331.
- **arm-zephyr-eabi-gdb** (Zephyr SDK) is the debugger. It reads symbols from `zephyr.elf`.
- **Cortex-Debug** (VS Code) starts the server and GDB for you: `.vscode/launch.json`.
- **SVD file** (`nrf52840.svd`): describes every peripheral register, so VS Code can show them by name.
- **RTOS plugin "Zephyr"**: lets the debugger list Zephyr threads (needs `CONFIG_DEBUG_THREAD_INFO`).

## Debug build
`app/debug.conf` is an extra Kconfig fragment, added on top of `prj.conf`:
```
west build -b nrf52840dk/nrf52840 app -p -- -DEXTRA_CONF_FILE=debug.conf
```
- `CONFIG_DEBUG_OPTIMIZATIONS`: `-Og` instead of `-Os`, so stepping follows the source.
- `CONFIG_ASSERT`: failed `__ASSERT()` stops the program (CMake prints a warning about it, that's expected).
- Normal build again: run the build command without `-DEXTRA_CONF_FILE` (with `-p`).

## Terminal alternative
`west debug` flashes and opens GDB in the terminal; `west attach` connects without flashing.
Useful GDB commands: `break cts_time_write`, `continue`, `next`, `step`, `print base_ms`, `bt`, `info threads`.

## BLE and breakpoints
- The SoftDevice Controller has strict radio timing. If the CPU stops at a breakpoint while
  BLE is active, the controller misses its deadlines; after you continue, the link drops
  or the controller asserts (MPSL fault).
- So: after a breakpoint with BLE running, expect to reset the board. To look at values without
  stopping, use logs or watch variables in "attach" mode.
- The RTC keeps counting while the CPU is halted, so our clock doesn't lose time at a breakpoint.
