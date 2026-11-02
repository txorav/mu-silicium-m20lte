#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PlatformUsbfnLib.h>
#include <Library/UefiBootServicesTableLib.h>

#define EXYNOS7885_PMU_BASE             0x11C80000
#define EXYNOS7885_PMU_USBDRD_PHY_CTRL  (EXYNOS7885_PMU_BASE + 0x0674)
#define EXYNOS7885_PMU_PHY_ENABLE       BIT0

#define EXYNOS7885_USBDRD_CTRL_BASE     0x13600000
#define EXYNOS7885_USBDRD_PHY_BASE      0x135D0000

// EXYNOS 7885 / 7904 USBDRD PHY registers (from Samsung Linux kernel phy-exynos-usb3p1-reg.h)
#define EXYNOS_USBCON_LINK_CTRL         0x04
#define LINKCTRL_DIS_QACT_ID0           (1u << 11)
#define LINKCTRL_DIS_QACT_VBUS_VALID    (1u << 10)
#define LINKCTRL_DIS_QACT_BVALID        (1u << 9)
#define LINKCTRL_DIS_QACT_LINKGATE      (1u << 12)
#define LINKCTRL_FORCE_QACT             (1u << 8)
#define LINKCTRL_BUS_FILTER_BYPASS(x)   (((x) & 0xFu) << 4)

#define EXYNOS_USBCON_LINK_PORT         0x08
#define LINKPORT_HUB_PORT_SEL_OCD_U3    (1u << 3)
#define LINKPORT_HUB_PORT_SEL_OCD_U2    (1u << 2)

#define EXYNOS_USBCON_CLKRST            0x20
#define CLKRST_LINK_SW_RST              (1u << 0)
#define CLKRST_PORT_RST                 (1u << 1)
#define CLKRST_PHY_RST_SEL              (1u << 2)
#define CLKRST_PHY_SW_RST               (1u << 3)

#define EXYNOS_USBCON_PWR               0x24
#define PWR_FORCE_POWERDOWN             (1u << 2)
#define PWR_TEST_POWERDOWN_SSP          (1u << 1)
#define PWR_TEST_POWERDOWN_HSP          (1u << 0)

#define EXYNOS_USBCON_UTMI              0x50
#define UTMI_FORCE_SLEEP                (1u << 0)
#define UTMI_FORCE_SUSPEND              (1u << 1)
#define UTMI_DM_PULLDOWN                (1u << 2)
#define UTMI_DP_PULLDOWN                (1u << 3)
#define UTMI_FORCE_BVALID               (1u << 4)
#define UTMI_FORCE_VBUSVALID            (1u << 5)

#define EXYNOS_USBCON_HSP               0x54
#define HSP_COMMONONN                   (1u << 8)
#define HSP_EN_UTMISUSPEND              (1u << 9)
#define HSP_VBUSVLDEXT                  (1u << 12)
#define HSP_VBUSVLDEXTSEL               (1u << 13)

#define EXYNOS_USBCON_HSP_TEST          0x5C
#define HSP_TEST_SIDDQ                  (1u << 24)

STATIC
VOID
UsbPhyIsolation (IN BOOLEAN Enable)
{
  UINT32 Register;

  Register = MmioRead32 (EXYNOS7885_PMU_USBDRD_PHY_CTRL);
  if (Enable) {
    Register |= EXYNOS7885_PMU_PHY_ENABLE;
  } else {
    Register &= ~EXYNOS7885_PMU_PHY_ENABLE;
  }
  MmioWrite32 (EXYNOS7885_PMU_USBDRD_PHY_CTRL, Register);
}

EFI_STATUS
UsbfnPlatformGetData (OUT USBFN_PLATFORM_DATA *PlatformData)
{
  if (PlatformData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  PlatformData->ControllerBase        = EXYNOS7885_USBDRD_CTRL_BASE;
  PlatformData->MaximumSpeed          = UsbBusSpeedHigh;
  PlatformData->SuperSpeedPhyDisabled = TRUE;
  PlatformData->UtmiWidth             = UsbfnUtmiWidth16Bit;
  PlatformData->Usb2FreeClockExists   = FALSE;
  PlatformData->FrameLengthAdjustMask = 0;
  PlatformData->FrameLengthAdjustValue = 0;
  PlatformData->NumEndpoints          = 0;
  PlatformData->VendorId              = 0x18D1; // Google
  PlatformData->ProductId             = 0xD00D; // Fastboot

  return EFI_SUCCESS;
}

EFI_STATUS
UsbfnPlatformPhyInit (VOID)
{
  UINT32 Reg;

  // 1. Bring PHY out of isolation in PMU
  UsbPhyIsolation (TRUE);
  gBS->Stall (100);

  // 2. Set force q-channel (WA for Q-channel on Exynos 7885)
  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_CTRL);
  Reg |= LINKCTRL_DIS_QACT_ID0 | LINKCTRL_DIS_QACT_VBUS_VALID |
         LINKCTRL_DIS_QACT_BVALID | LINKCTRL_DIS_QACT_LINKGATE;
  Reg &= ~LINKCTRL_FORCE_QACT;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_CTRL, Reg);
  gBS->Stall (500);

  Reg |= LINKCTRL_FORCE_QACT;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_CTRL, Reg);
  gBS->Stall (500);

  // 3. Link Reset
  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_CLKRST);
  Reg |= CLKRST_LINK_SW_RST;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_CLKRST, Reg);
  gBS->Stall (10);
  Reg &= ~CLKRST_LINK_SW_RST;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_CLKRST, Reg);

  // 4. Set PHY Reset High
  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_CLKRST);
  Reg |= CLKRST_PHY_SW_RST | CLKRST_PHY_RST_SEL;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_CLKRST, Reg);

  // 5. Power enable (clear SIDDQ in HSP_TEST)
  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_HSP_TEST);
  Reg &= ~HSP_TEST_SIDDQ;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_HSP_TEST, Reg);

  // 6. Clear UTMI force suspend/sleep and pulldowns
  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_UTMI);
  Reg &= ~(UTMI_FORCE_SUSPEND | UTMI_FORCE_SLEEP | UTMI_DP_PULLDOWN | UTMI_DM_PULLDOWN);
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_UTMI, Reg);

  // 7. Keep HSP COMMONONN set for analog bias
  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_HSP);
  Reg |= HSP_COMMONONN;
  Reg &= ~HSP_EN_UTMISUSPEND;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_HSP, Reg);

  gBS->Stall (100);

  // 8. Set PHY Reset Low
  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_CLKRST);
  Reg |= CLKRST_PHY_RST_SEL;
  Reg &= ~(CLKRST_PHY_SW_RST | CLKRST_PORT_RST);
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_CLKRST, Reg);

  // 9. Bypass VBUS filter and Force VBUS & B-Valid (required because VBUS pad is not wired to SoC)
  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_CTRL);
  Reg |= LINKCTRL_BUS_FILTER_BYPASS (0xF);
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_CTRL, Reg);

  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_UTMI);
  Reg |= UTMI_FORCE_BVALID | UTMI_FORCE_VBUSVALID;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_UTMI, Reg);

  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_HSP);
  Reg |= HSP_VBUSVLDEXTSEL | HSP_VBUSVLDEXT;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_HSP, Reg);

  // 10. OVC io usage
  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_PORT);
  Reg |= LINKPORT_HUB_PORT_SEL_OCD_U3 | LINKPORT_HUB_PORT_SEL_OCD_U2;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_PORT, Reg);

  // 11. Force PIPE3 signals and PMA low power for USB 2.0-only operation (dwc3 command clock ungate)
  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_CTRL);
  Reg |= (1u << 16) | (1u << 18); // LINKCTRL_PIPE3_FORCE_EN | LINKCTRL_PIPE3_FORCE_RX_ELEC_IDLE
  Reg &= ~(1u << 17);             // ~LINKCTRL_PIPE3_FORCE_PHY_STATUS
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_CTRL, Reg);

  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + 0x48); // EXYNOS_USBCON_COMBO_PMA_CTRL
  Reg |= (1u << 4); // PMA_LOW_PWRN
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + 0x48, Reg);

  gBS->Stall (100);
  return EFI_SUCCESS;
}

EFI_STATUS
UsbfnPlatformPhyExit (VOID)
{
  UINT32 Reg;

  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_UTMI);
  Reg |= UTMI_FORCE_SUSPEND | UTMI_FORCE_SLEEP;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_UTMI, Reg);

  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_HSP_TEST);
  Reg |= HSP_TEST_SIDDQ;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_HSP_TEST, Reg);

  Reg = MmioRead32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_CTRL);
  Reg &= ~LINKCTRL_FORCE_QACT;
  Reg |= LINKCTRL_DIS_QACT_ID0 | LINKCTRL_DIS_QACT_VBUS_VALID |
         LINKCTRL_DIS_QACT_BVALID | LINKCTRL_DIS_QACT_LINKGATE;
  MmioWrite32 (EXYNOS7885_USBDRD_PHY_BASE + EXYNOS_USBCON_LINK_CTRL, Reg);

  UsbPhyIsolation (FALSE);
  return EFI_SUCCESS;
}

BOOLEAN
UsbfnPlatformIsVbusPresent (VOID)
{
  return TRUE;
}
