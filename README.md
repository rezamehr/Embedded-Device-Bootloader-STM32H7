# EDCC Bootloader – STM32H7


[![License](https://img.shields.io/badge/license-MIT-blue)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-STM32H7-blue)](https://www.st.com/en/microcontrollers-microprocessors/stm32h7-series.html)
[![Version](https://img.shields.io/badge/version-1.0.0-green)](https://github.com/rezamehr/Embedded-Device-Bootloader-STM32H7)
[![Protocol](https://img.shields.io/badge/protocol-EDCC%20v1.0-orange)](https://github.com/rezamehr/Embedded-Device-Control-Center/blob/main/docs/bootloader_protocol.md)

Custom bootloader for **STM32H7** series, designed to work with  
[Embedded Device Control Center (EDCC)](https://github.com/rezamehr/Embedded-Device-Control-Center).

This bootloader implements the **EDCC Bootloader Protocol v1.0** and allows  
secure firmware update over UART using a simple framed protocol.

---

## Features

- Full support for EDCC Bootloader Protocol v1.0
- Commands: `IDENTITY`, `READ`, `ERASE`, `WRITE`, `JUMP`
- UART + DMA + IDLE line detection (robust frame reception)
- FreeRTOS based
- Flash operations optimized for STM32H7 (32-byte alignment)
- XOR checksum for frame integrity
- Backup register based boot mode selection
- Jump to application support

---

## Hardware

| Item              | Value                  |
|-------------------|------------------------|
| MCU               | STM32H7 series         |
| UART              | USART6 (PC6/PC7)       |
| Baudrate          | 9600                   |
| Flash word size   | 32 bytes               |
| Bootloader size   | 128 KB (0x08000000)    |
| Application start | 0x08020000             |

> **Note:** Baudrate is set to 9600 for reliability during large firmware writes.  
> Higher speeds caused data loss on the current hardware setup.

---

## Protocol Summary

Frame format:
START (0xAA) + LENGTH + PAYLOAD + CHECKSUM (XOR of payload)
textSupported commands:

| Command     | Code | Description                  |
|-------------|------|------------------------------|
| IDENTITY    | 0x01 | Get device info              |
| READ        | 0x02 | Read memory                  |
| ERASE       | 0x03 | Erase flash range            |
| WRITE       | 0x04 | Write data to flash          |
| JUMP        | 0x05 | Jump to application          |
| ACK         | 0xFF | Success response             |
| NACK        | 0x1F | Error response               |

Full protocol specification:  
➡️ [EDCC Bootloader Protocol v1.0](https://github.com/rezamehr/Embedded-Device-Control-Center/blob/main/docs/bootloader_protocol.md)

---

## Project Structure

```text
Core/
├── Inc/
│   ├── bootloader_config.h
│   ├── bootloader.h
│   ├── flash_operations.h
│   ├── frame_protocol.h
│   ├── aes.h
│   ├── crc32.h
│   ├── FreeRTOSConfig.h
│   └── ...
└── Src/
    ├── main.c
    ├── freertos.c
    ├── flash_operations.c
    ├── frame_protocol.c
    ├── aes.c
    ├── crc32.c
    └── ...

How to Use with EDCC

Flash this bootloader to the STM32H7
Open Embedded Device Control Center
Connect to the device (Serial)
Go to Firmware Update panel
Select .bin file and start the update process

Typical sequence performed by EDCC:

IDENTITY
ERASE
WRITE (chunks)
JUMP


Memory Map
text0x08000000 ─────────────────────────────
│              Bootloader (128 KB)
0x08020000 ─────────────────────────────
│              Application
│              (up to ~1.875 MB)
0x08200000 ─────────────────────────────

Related Projects

Desktop Application:
Embedded Device Control Center


License
MIT License

Author
Reza Mehrabani
Embedded Systems Engineer
