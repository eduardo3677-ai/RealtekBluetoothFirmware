# Supported Devices

On Realtek combo cards the Wi-Fi may be PCIe while the **Bluetooth radio is an
internal USB device**. This kext matches that USB Bluetooth device and uploads
its patch firmware.

| Chip        | lmp_subver | hci_rev | hci_ver | Firmware           | USB IDs matched |
|-------------|-----------|---------|---------|--------------------|-----------------|
| RTL8822C/CE | 0x8822    | 0x000c  | 0x0a    | `rtl8822cu_fw.bin` | `0bda:b00c`, `0bda:c123`, `0bda:c822` |

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
