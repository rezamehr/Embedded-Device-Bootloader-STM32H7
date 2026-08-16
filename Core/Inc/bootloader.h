/**
  ******************************************************************************
  * @file    bootloader.h
  * @brief   Main bootloader management
  * @version V1.0.0
  * @date    15-August-2026
  ******************************************************************************
  */

#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "bootloader_config.h"
#include "frame_protocol.h"
#include "flash_operations.h"

/* ============================================================================
   Type Definitions
   ============================================================================ */

/**
  * @brief  Bootloader status enumeration
  */
typedef enum {
    BOOT_STATUS_OK = 0,
    BOOT_STATUS_ERROR_FRAME_INVALID,
    BOOT_STATUS_ERROR_CHECKSUM,
    BOOT_STATUS_ERROR_FLASH_ERASE,
    BOOT_STATUS_ERROR_FLASH_WRITE,
    BOOT_STATUS_ERROR_INVALID_COMMAND,
    BOOT_STATUS_ERROR_INVALID_ADDRESS,
    BOOT_STATUS_ERROR_TIMEOUT
} BootStatus_t;

/**
  * @brief  Bootloader context structure
  */
typedef struct {
    bool is_initialized;
    uint32_t app_start_address;
    uint32_t app_size;
    uint8_t version_major;
    uint8_t version_minor;
    uint8_t version_patch;
    BootStatus_t last_error;
} BootloaderContext_t;

/* ============================================================================
   Function Prototypes
   ============================================================================ */

/**
  * @brief  Initialize bootloader
  * @param  ctx: Pointer to bootloader context
  * @retval true if successful, false on error
  */
//bool Bootloader_Init(BootloaderContext_t *ctx);

/**
  * @brief  Check if application is valid and jump to it
  * @param  ctx: Pointer to bootloader context
  * @retval true if jumped, false if invalid application
  */
//bool Bootloader_CheckAndJumpToApp(BootloaderContext_t *ctx);

/**
  * @brief  Get bootloader version string
  * @retval Pointer to version string
  */
//const char* Bootloader_GetVersionString(void);

/**
  * @brief  Get last error status
  * @param  ctx: Pointer to bootloader context
  * @retval Last error status
  */
//BootStatus_t Bootloader_GetLastError(const BootloaderContext_t *ctx);

/**
  * @brief  Reset bootloader error status
  * @param  ctx: Pointer to bootloader context
  */
//void Bootloader_ClearError(BootloaderContext_t *ctx);

void ReceiveRX(struct rxINFO *pData);
uint8_t Uart_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, TickType_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* BOOTLOADER_H */
