// SPDX-License-Identifier: MIT
// PowerFinger — HID report descriptor
//
// This file exists as documentation and a shared reference.
// The actual descriptor bytes are embedded in hal_ble_esp.c where the
// GATT service is registered. Both must stay in sync.
//
// Report format (4 bytes):
//   Byte 0: buttons (bit 0 = left, bit 1 = right, bit 2 = middle, bits 3-7 = padding)
//   Byte 1: X delta (-127 to 127, relative)
//   Byte 2: Y delta (-127 to 127, relative)
//   Byte 3: Wheel  (-127 to 127, relative, vertical scroll)
//
// This descriptor is the contract between the ring and every host OS.
// Changing it requires re-pairing on all hosts.
