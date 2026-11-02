# Project Silicium UEFI for ARM64-Based Devices

<!-- ![Banner](https://github.com/Project-Silicium/Mu-Silicium/blob/main/Resources/Pictures/Banner.png) -->

## Description

This Project focuses on Making ARM-Based Devices Boot Operating Systems like Windows or Linux using Custom UEFI Firmware.

## Resources

- [Status](Status.md)
- [Building](Building.md)
- [Guides](https://github.com/Project-Silicium/Guides/blob/main/README.md)
- [Discord Server](https://discord.gg/Dx2QgMx7Sv)

## Featured Device: Samsung Galaxy M20 (`m20lte`)

The Samsung Galaxy M20 (SM-M205F/FN/G, Exynos 7904 / S5E7904) is fully supported with standalone UEFI firmware:
- **Build Target:** `m20lte` (`Platforms/Samsung/m20ltePkg`)
- **Display:** 1080x2340 Framebuffer at `0xEC000000`
- **USB:** Synopsys DWC3 Peripheral/Device Mode (`UsbfnDwc3Dxe`)
- **Firmware Base:** `0x8F000000` (Size: 2 MiB)
- **Auto-Boot:** Boots directly into EDK2 Boot Manager or auto-chains XnuLoader at `0x8F100000`

### Quick Build for `m20lte`:
```bash
./setup_env.sh
python3 build_uefi.py -d m20lte
```
Output artifact is generated at `Build/m20lte/DEBUG_GCC5/FV/SILICIUM_UEFI.fd` and packaged into `Mu-m20lte.img`.

## Frequently asked Questions

### **Is my Device Supported in this Project?**
- To Check if your Device is Supported, Take a Look at the Status Page Referenced in the Section Above.

### **What do I do if my Device isn't Supported in this Project?**
- We made some Guides that Explain How to Port your Device that are Referenced in the Section Above.

## Credits

[Microsoft](https://github.com/microsoft/) for their [Project Mu](https://microsoft.github.io/mu/) which Powers this Project. <br>
[WOA-Project](https://github.com/WOA-Project/) for their [mu_andromeda_platforms](https://github.com/WOA-Project/mu_andromeda_platforms/) which is used as Base for this Project. <br>
[Project Aloha](https://github.com/Project-Aloha/) for their [mu_aloha_platforms](https://github.com/Project-Aloha/mu_aloha_platforms/) which is used as Reference for some SoCs. <br>
[Open Surface RT](https://openrt.gitbook.io/open-surfacert/) for their Help on Tegra Related Stuff.

[sonic011gamer](https://github.com/sonic011gamer/) and [halal-beef](https://github.com/halal-beef/) for their Amazing Work on Exynos Related Stuff.

## License

All code except drivers in `GPLDrivers` & `GPLApplications` & `GPLLibrary` directories are licensed under BSD-2-Clause. <br />
GPL Drivers are licensed under `GPLv2` or later license.
