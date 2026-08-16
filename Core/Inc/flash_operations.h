/**
  ******************************************************************************
  * @file    flash_operations.h
  * @brief   Flash memory operations for bootloader
  * @version V1.0.0
  * @date    15-August-2026
  ******************************************************************************
  */

#ifndef FLASH_OPERATIONS_H
#define FLASH_OPERATIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "bootloader_config.h"

/* ============================================================================
   Function Prototypes
   ============================================================================ */

/**
  * @brief  Calculate flash sector number from address
  * @param  address: Memory address
  * @retval Sector number (0-127)
  */
uint32_t Flash_GetSectorFromAddress(uint32_t address);

/**
  * @brief  Erase flash sector containing specified address
  * @param  address: Address within sector to erase
  * @retval true if successful, false on error
  */
bool Flash_EraseSector(uint32_t address);

/**
  * @brief  Erase multiple flash sectors
  * @param  start_addr: Starting address
  * @param  size_bytes: Total bytes to erase
  * @retval true if successful, false on error
  */
bool Flash_EraseRange(uint32_t start_addr, uint32_t size_bytes);

/**
  * @brief  Write data to flash memory
  * @param  address: Destination address (must be aligned)
  * @param  data: Pointer to data buffer
  * @param  size_bytes: Number of bytes to write
  * @retval true if successful, false on error
  */
bool Flash_WriteData(uint32_t address, const uint8_t *data, uint32_t size_bytes);

/**
  * @brief  Read data from flash memory
  * @param  address: Source address
  * @param  buffer: Output buffer
  * @param  size_bytes: Number of bytes to read
  * @retval true if successful, false on error
  */
bool Flash_ReadData(uint32_t address, uint8_t *buffer, uint32_t size_bytes);

/**
  * @brief  Verify flash data against buffer
  * @param  address: Flash address
  * @param  data: Buffer to compare
  * @param  size_bytes: Number of bytes to verify
  * @retval true if match, false if mismatch
  */
bool Flash_VerifyData(uint32_t address, const uint8_t *data, uint32_t size_bytes);

/**
  * @brief  Jump to application code
  * @param  app_address: Application start address
  * @retval Does not return
  */
void Flash_JumpToApplication(uint32_t app_address) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif /* FLASH_OPERATIONS_H */
