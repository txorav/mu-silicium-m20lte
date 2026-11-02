# Samsung Galaxy M20 (`m20ltePkg`) - Mu-Silicium UEFI

Standalone Project Mu / EDK2 UEFI port for the **Samsung Galaxy M20** (`m20lte`, SM-M205F/FN/G).

---

## Hardware Specifications

| Component | Specification |
|---|---|
| **Device Model** | Samsung Galaxy M20 (`SM-M205F`, `SM-M205FN`, `SM-M205G`) |
| **SoC** | Samsung Exynos 7904 (`S5E7904` / `universal7904`) |
| **CPU Architecture** | ARMv8-A 64-bit |
| **Cores** | 8 cores: 2x Cortex-A73 @ 1.8 GHz + 6x Cortex-A53 @ 1.6 GHz |
| **Core MPIDRs** | `0x100`, `0x101`, `0x102`, `0x103`, `0x200`, `0x201`, `0x0`, `0x1` |
| **Memory** | 3 GB / 4 GB LPDDR4x SDRAM |
| **Storage** | eMMC 5.1 (`13500000.dwmmc0`) |
| **Display** | 6.3-inch PLS TFT LCD, 1080 x 2340 pixels |
| **Framebuffer Address** | `0xEC000000` |
| **Interrupt Controller** | ARM GIC-400 (GICv2): Distributor `0x12301000`, CPU Interface `0x12302000` |
| **USB Controller** | Synopsys DesignWare Core 3 (DWC3) at `0x13600000` |
| **Timer** | ARM Generic Timer (virtual & physical counter) |
| **Bootloader** | Samsung S-Boot (unlocked) |

---

## Memory Map

| Region | Physical Base | Size | Description |
|---|---|---|---|
| **UEFI Firmware (FD)** | `0x8F000000` | 2 MiB (`0x200000`) | Main firmware volume (`SILICIUM_UEFI.fd`) |
| **XnuLoader Payload Window** | `0x8F100000` | 512 KiB (`0x80000`) | Embedded EFI payload window (`XNUP`) |
| **Framebuffer** | `0xEC000000` | ~10 MiB | Initialized by S-Boot display engine |
| **GIC Distributor** | `0x12301000` | 4 KiB | GICv2 Distributor registers |
| **GIC CPU Interface** | `0x12302000` | 4 KiB | GICv2 CPU Interface registers |

---

## Building

From the root of `Mu-Silicium`:

```bash
# Setup the build environment
./setup_env.sh

# Build UEFI for m20lte
python3 build_uefi.py -d m20lte
```

### Build Outputs
- Firmware Volume: `Build/m20lte/DEBUG_GCC5/FV/SILICIUM_UEFI.fd`
- Packaged Image: `Mu-m20lte.img`

---

## Boot and Auto-Chaining

1. **BootShim**: Relocates `SILICIUM_UEFI.fd` to `0x8F000000` and transfers execution to the EDK2 SEC core.
2. **Boot Menu**: If no boot option is selected, displays the customized Project Mu Boot Manager Menu (`BootManagerMenuApp`).
3. **Auto-chaining**: If an `XNUP` payload is present at `0x8F100000` (e.g. `xnuloader`), it automatically loads and executes the EFI binary from memory without requiring external storage.

---

## License & Credits

- Ported for `m20lte` by **txorav**.
- Based on [Project Mu](https://microsoft.github.io/mu/) and [Project-Silicium](https://github.com/Project-Silicium/Mu-Silicium).
