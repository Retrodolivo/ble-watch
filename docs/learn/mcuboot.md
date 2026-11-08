# MCUboot (Step 4)

## What it is
- A **bootloader**: a small program that runs first at every reset, before our app.
- Its job: check that the app image is valid (signature), and install a new image if one is waiting.
- Without it, a firmware update over BLE would be dangerous: a half-written or bad image would brick the watch.

## How we add it: sysbuild (`app/sysbuild.conf`)
- **Sysbuild** builds several images in one `west build`. Here: `mcuboot` and `app`.
- `SB_CONFIG_...` options are sysbuild Kconfig (for the whole build), not app Kconfig (`CONFIG_...` in `prj.conf`).
- `SB_CONFIG_BOOTLOADER_MCUBOOT=y` is all it takes. Sysbuild then also **signs** the app.
- Build output: `build/mcuboot/` and `build/app/`. The file MCUboot accepts is `build/app/zephyr/zephyr.signed.hex`.
- `west flash` flashes both, in the order in `build/domains.yaml` (mcuboot first).

## Flash layout (from devicetree, `nrf52840_partition.dtsi`)
| Partition | Address | Size | Content |
|---|---|---|---|
| `boot_partition` (mcuboot) | 0x00000 | 48 KB | MCUboot (~32 KB used) |
| `slot0_partition` (image-0) | 0x0C000 | 472 KB | the running app (~149 KB used) |
| `slot1_partition` (image-1) | 0x82000 | 472 KB | where a new image is downloaded |
| `storage_partition` | 0xF8000 | 32 KB | settings (later, for bonding keys) |

- The app is now linked to start at 0xC000 (`zephyr,code-partition = &slot0_partition`), not at 0.
- `CONFIG_ROM_START_OFFSET=0x200`: the first 0x200 bytes of slot 0 are the MCUboot **image header**
  (magic, version, size). The signature and hash go in a **trailer** after the code (the "TLVs").

## Signing
- The build signs the app with a **private key**. MCUboot holds the matching **public key** and checks the signature.
- Type: ECDSA P-256 (NCS default on nRF52840; the CryptoCell 310 hardware can speed it up).
- Right now we use MCUboot's **demo key** `root-ec-p256.pem`. It is public on GitHub, so anyone could sign an
  image for our watch. Fine for learning; we'll generate our own key before relying on updates.

## Swap using move (`CONFIG_BOOT_SWAP_USING_MOVE=y`)
- A new image is written into slot 1. At the next reset MCUboot **swaps** slot 0 and slot 1, sector by sector.
- The old image is now in slot 1. If the new image was sent as a **test** and does not mark itself as
  "confirmed", MCUboot swaps back at the next reset: that is the **rollback** in the plan.

## SMP and MCUmgr: updates over BLE (`app/prj.conf`)
- **MCUmgr** is Zephyr's device management subsystem. It runs in the **app**, not in MCUboot.
  MCUboot never uses the radio: the app receives the new image and writes it to slot 1.
- **SMP** (Simple Management Protocol) is MCUmgr's message format: small requests and responses
  encoded in CBOR. Over BLE it is one GATT service (UUID `8D53DC1D-1DB7-4CD3-868B-8A527460AA84`)
  with one characteristic: the phone writes requests, the watch answers with notifications.
- Command **groups**: `img` (list images, upload, test/confirm), `os` (reset, echo, info).
- `CONFIG_NCS_SAMPLE_MCUMGR_BT_OTA_DFU=y` is an NCS helper that turns on all of this at once
  (MCUmgr, the BT transport, `img` and `os` groups, bigger BT buffers for faster upload).
- `CONFIG_MCUBOOT_IMGTOOL_SIGN_VERSION`: the version in the image header. Device Manager shows it.
- `build/dfu_application.zip` is the upload package for Device Manager (signed bin + manifest).
- Cost: the app grew from ~149 KB to ~170 KB of flash.
