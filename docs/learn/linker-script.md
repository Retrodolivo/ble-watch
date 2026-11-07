# The linker script (and the map file)

## Where it comes from
- Nobody writes it by hand. The template is `zephyr/include/zephyr/arch/arm/cortex_m/scripts/linker.ld`,
  run through the **C preprocessor** with devicetree + Kconfig macros.
- Result: `build/app/zephyr/linker.cmd` (~430 lines). `linker_zephyr_pre0.cmd` is the same for a first, temporary link.
- FLASH/RAM origin and size come from devicetree: `zephyr,code-partition` / `zephyr,flash` and `zephyr,sram`.
  Today FLASH starts at `0x0`. With MCUboot (step 4) the app partition moves and this number changes by itself.

## MEMORY: the regions
```
FLASH (rx) : ORIGIN = 0x0,        LENGTH = 1 MB     code + constants
RAM   (wx) : ORIGIN = 0x20000000, LENGTH = 256 KB   variables, stacks
```

## SECTIONS: what goes where (from zephyr.map, this build)
| Output section | Address     | Contains |
|----------------|-------------|----------|
| `rom_start`    | 0x00000000  | vector table (initial stack pointer + reset handler first) |
| `text`         | 0x00000100  | all code (`.text.*`), e.g. `main` at 0x400 |
| `device_area`  | 0x00009b70  | one `struct device` per enabled DT node (`__device_dts_ord_N`) |
| `rodata`       | 0x00009f10  | `const` data, strings |
| `datas`        | 0x20000000  | initialized variables; **stored** in flash at 0xb0a4, copied to RAM at boot |
| `bss`          | 0x20000368  | zero-initialized variables, e.g. `framebuf` (0x1388 = 5000 B) at 0x2000108c |
| `noinit`       | 0x20002440  | thread stacks etc., not cleared at boot |

## Zephyr tricks
- `*_area` sections with `KEEP(*(SORT(...)))`: macros like `DEVICE_DT_DEFINE`, `LOG_MODULE_REGISTER`,
  `SYS_INIT` put structs in named sections; the linker collects them into **arrays** with start/end symbols
  (`_device_list_start` ... `_device_list_end`). That's how the kernel finds all devices without a registry in C.
- `-ffunction-sections` + `--gc-sections`: each function is its own `.text.<name>`; unused ones are dropped.
- Small `static` functions (`draw_test_pattern`, `set_black`) were **inlined** into `main`, so they have no symbol.

## Tools
- `build/app/zephyr/zephyr.map`: every input section, its address, size and source object.
- `arm-zephyr-eabi-nm -S --size-sort zephyr.elf`: symbols with sizes.
- `west build -t rom_report` / `-t ram_report`: size tree per file.
