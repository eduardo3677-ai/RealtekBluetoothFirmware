# Supported Devices

On Realtek combo cards the Wi-Fi may be PCIe while the **Bluetooth radio is an
internal USB device**. This kext matches that USB Bluetooth device and uploads
its patch firmware.

All controllers that use Realtek's **v2 (`RTBTCore`)** firmware are supported:

| Chip          | lmp_subver | hci_rev | hci_ver | Firmware              |
|---------------|-----------|---------|---------|-----------------------|
| RTL8822C/CE   | 0x8822    | 0x000c  | 0x0a    | `rtl8822cu_fw.bin`    |
| RTL8851B      | 0x8851    | 0x000b  | 0x0c    | `rtl8851bu_fw.bin`    |
| RTL8852B      | 0x8852    | 0x000b  | 0x0b    | `rtl8852bu_fw.bin`    |
| RTL8852C      | 0x8852    | 0x000c  | 0x0c    | `rtl8852cu_fw_v2.bin` |
| RTL8852BT/BE-VT | 0x8852  | 0x0087  | 0x0c    | `rtl8852btu_fw.bin`   |
| RTL8922A      | 0x8922    | 0x000a  | 0x0c    | `rtl8922au_fw.bin`    |

The USB IDs matched for each chip (vendor `0x0BDA` plus the various module
vendors) are taken from the Linux `btusb.c` device table and listed in
`Info.plist`. The actual chip is identified at runtime from its HCI version, so
a board only needs its USB ID present to be picked up.

> RTL8852A and older parts (8822B, 8821C, 8723x, 8761x …) ship **v1
> (`Realtech`)** firmware, which this build does not upload yet.

## Finding your device ID

In macOS, open **System Information → USB** and look for a Realtek "Bluetooth
Radio" entry (vendor `0x0BDA`), or on Linux/Windows run `lsusb` / check Device
Manager. The 4-digit product ID after `0bda:` is what must appear in
`Info.plist`.

If your device reports a different product ID than those listed
above, add it as a new personality in `Info.plist` (the `idProduct` value is in
decimal).

## Adding another Realtek chip

The firmware parser is generic across the Realtek family. To add support for a
new controller:

1. Add its firmware `.bin` (from
   [linux-firmware/rtl_bt](https://gitlab.com/kernel-firmware/linux-firmware/-/tree/main/rtl_bt))
   to `fw/` and run `make fw`.
2. Add a row to `kIcTable` in `BtRtl.cpp` with its `lmp_subver` / `hci_rev` /
   `hci_ver` and firmware name.
3. Add an `idProduct` personality to `Info.plist`.
