# Installing NCS by hand (without Nordic's SDK Manager)

NCS is just git repos plus a compiler. The SDK Manager in VS Code is a convenience that
downloads a prepared bundle. We did the same steps by hand:

1. **Python venv + west**: `python3 -m venv ~/ncs/v3.4.2/.venv`, then `pip install west`.
   A venv keeps the SDK's Python packages apart from the system Python.
2. **`west init -m https://github.com/nrfconnect/sdk-nrf --mr v3.4.2`**: downloads only the
   *manifest repo* (`nrf/`). Its `west.yml` lists every other repo and the exact commit to use.
3. **`west update --narrow -o=--depth=1`**: clones all those repos (zephyr, mcuboot, nrfxlib…).
   `--depth=1` downloads only the latest commit, which saves time and disk space (3.1 GB).
4. **Python requirements**: `pip install -r zephyr/scripts/requirements.txt -r nrf/scripts/requirements.txt ...`.
   The build scripts are Python (devicetree, Kconfig, image signing).
5. **Compiler: Zephyr SDK**. `zephyr/SDK_VERSION` says which version this Zephyr expects (1.0.1).
   We downloaded the "minimal" SDK and only the `arm-zephyr-eabi` toolchain, checked the SHA-256,
   and ran `setup.sh -c`. `-c` registers the SDK in `~/.cmake/packages/`, which is how
   CMake finds the compiler without any environment variables.

## Using it from a terminal
```
source ~/ncs/v3.4.2/.venv/bin/activate   # or the alias `ncs` (in ~/.zshrc); `deactivate` to leave
cd ~/ncs/v3.4.2      # or anywhere: west finds the workspace via ZEPHYR_BASE or the app's location
west build -b nrf52840dk/nrf52840 <app>
```

## Using it from VS Code
- The `nrf-connect.west.env` user setting puts the venv's `bin/` on `PATH` so the extension finds `west`.
- **`west zephyr-export`** registers Zephyr in the CMake package registry
  (`~/.cmake/packages/Zephyr`). The extension reads that registry to find SDKs, so after a
  window reload it lists `~/ncs/v3.4.2`. (It also finds an SDK that is open as a workspace
  folder, or one set in the `ZEPHYR_BASE` environment variable.)
- Toolchain: the extension looks for `~/zephyr-sdk-*` folders, so it finds `~/zephyr-sdk-1.0.1`.

## Flash runners
`west flash` uses a *runner*, a small Python wrapper around a flashing tool. The board lists
the ones it supports in `build/<app>/zephyr/runners.yaml`. The default on the nRF52840 DK is
`nrfutil`, but we don't have nRF Util's core, so we use `jlink` (SEGGER's `JLinkExe`):
`west flash --runner jlink`.
