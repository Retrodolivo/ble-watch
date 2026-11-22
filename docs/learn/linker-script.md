# The linker script (and the map file)

## Where it comes from
- Nobody writes it by hand. The template is `zephyr/include/zephyr/arch/arm/cortex_m/scripts/linker.ld`,
  run through the **C preprocessor** with devicetree + Kconfig macros.
- Result: `build/app/zephyr/linker.cmd` (~430 lines). `linker_zephyr_pre0.cmd` is the same for a first, temporary link.
- FLASH/RAM origin and size come from devicetree: `zephyr,code-partition` / `zephyr,flash` and `zephyr,sram`.
  With MCUboot, FLASH is the app slot (`slot0_partition`, 0xC000), not the whole chip. The numbers follow the
  partition table by themselves.

## MEMORY: the regions
```
FLASH (rx) : ORIGIN = 0xC000,     LENGTH = 472 KB - trailer   code + constants (slot 0)
RAM   (wx) : ORIGIN = 0x20000000, LENGTH = 256 KB             variables, stacks
```

## SECTIONS: what goes where (from zephyr.map, this build)
| Output section | Address     | Contains |
|----------------|-------------|----------|
| `rom_start`    | 0x0000c000  | MCUboot image header (0x200), then the vector table (initial stack pointer + reset handler first) |
| `text`         | 0x0000c300  | all code (`.text.*`), e.g. `main` at 0x1a358 |
| `device_area`  | 0x00033878  | one `struct device` per enabled DT node (`__device_dts_ord_N`) |
| `rodata`       | 0x00033fa0  | `const` data, strings |
| `datas`        | 0x20000000  | initialized variables; **stored** in flash at 0x37f0c, copied to RAM at boot |
| `bss`          | 0x20000928  | zero-initialized variables |
| `noinit`       | 0x20003e00  | thread stacks etc., not cleared at boot |

## Zephyr tricks
- `*_area` sections with `KEEP(*(SORT(...)))`: macros like `DEVICE_DT_DEFINE`, `LOG_MODULE_REGISTER`,
  `SYS_INIT` put structs in named sections; the linker collects them into **arrays** with start/end symbols
  (`_device_list_start` ... `_device_list_end`). That's how the kernel finds all devices without a registry in C.
- `-ffunction-sections` + `--gc-sections`: each function is its own `.text.<name>`; unused ones are dropped.
- Small `static` functions are often **inlined** into their caller, so they have no symbol of their own in the map.

## Tools
- `build/app/zephyr/zephyr.map`: every input section, its address, size and source object.
- `arm-zephyr-eabi-nm -S --size-sort zephyr.elf`: symbols with sizes.
- `west build -t rom_report` / `-t ram_report`: size tree per file.
