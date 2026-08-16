/**
  ******************************************************************************
  * @file    bootloader_config.h
  * @brief   Bootloader configuration and constants
  * @version V1.0.0
  * @date    15-August-2026
  ******************************************************************************
  */

#ifndef BOOTLOADER_CONFIG_H
#define BOOTLOADER_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
   Bootloader Version Information
   ============================================================================ */
#define BOOTLOADER_VERSION_MAJOR    1
#define BOOTLOADER_VERSION_MINOR    0
#define BOOTLOADER_VERSION_PATCH    0

/* ============================================================================
   Memory Map Definitions
   ============================================================================ */
#define APP_START_ADDRESS           0x08020000UL
#define APP_SIZE_MAX                0x001E0000UL      // ~1.875MB
#define BOOTLOADER_START_ADDRESS    0x08000000UL
#define BOOTLOADER_SIZE             0x00020000UL      // 128KB

/* ============================================================================
   Backup Register Definitions (for boot mode)
   ============================================================================ */
#define BKP_BASE_ADDR               0x38800000UL
#define BKP_BOOT_FLAG_OFFSET        4
#define BOOT_FLAG_BOOTLOADER        0xAAAAAAAAUL
#define BOOT_FLAG_APPLICATION       0x00000000UL

/* ============================================================================
   Communication Protocol Definitions
   ============================================================================ */
#define FRAME_START                 0xAA
#define MAX_PAYLOAD_SIZE            256
#define MAX_FRAME_SIZE              (1 + 1 + MAX_PAYLOAD_SIZE + 1)

#define CMD_IDENTITY                0x01
#define CMD_READ                    0x02
#define CMD_ERASE                   0x03
#define CMD_WRITE                   0x04
#define CMD_JUMP                    0x05
#define CMD_ACK                     0xFF
#define CMD_NACK                    0x1F

/* ============================================================================
   UART Configuration
   ============================================================================ */
#define UART_TIMEOUT_MS             1000
#define RX_BUF_SIZE                 255
#define TX_BUF_SIZE                 1024

/* ============================================================================
   Flash Operation Parameters
   ============================================================================ */
#define FLASH_WRITE_ALIGNMENT       32          // STM32H7 Flash word size
// Note: FLASH_SECTOR_SIZE is already defined in stm32h743xx.h
// We use it directly instead of redefining
#define MAX_ERASE_SECTORS           128

/* ============================================================================
   Debug/Testing Options
   ============================================================================ */
//#define ENABLE_DEBUG_LOG
#ifdef ENABLE_DEBUG_LOG
    #define DEBUG_LOG(msg, ...)    HAL_UART_Transmit(&huart6, (uint8_t*)msg, strlen(msg), 100)
#else
    #define DEBUG_LOG(msg, ...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* BOOTLOADER_CONFIG_H */
