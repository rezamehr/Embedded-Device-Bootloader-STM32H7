/**
  ******************************************************************************
  * @file    flash_operations.c
  * @brief   Flash operations implementation
  * @version V1.0.0
  * @date    15-August-2026
  ******************************************************************************
  */

#include "flash_operations.h"
#include "main.h"
#include "bootloader_config.h"
#include <string.h>

/* ============================================================================
   External Variables
   ============================================================================ */
extern UART_HandleTypeDef huart6;

/* ============================================================================
   Private Constants
   ============================================================================ */
// FLASH_SECTOR_SIZE is defined in stm32h743xx.h
// We use it directly

/* ============================================================================
   Public Functions
   ============================================================================ */

/**
  * @brief  Calculate flash sector from address
  */
uint32_t Flash_GetSectorFromAddress(uint32_t address) {
    uint32_t sector = (address - 0x08000000UL) / FLASH_SECTOR_SIZE;
    return (sector < 128) ? sector : 0;
}

/**
  * @brief  Erase flash sector
  */
bool Flash_EraseSector(uint32_t address) {
    HAL_StatusTypeDef status;
    uint32_t sector_error = 0;

    uint32_t sector = Flash_GetSectorFromAddress(address);

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase_config = {
        .TypeErase     = FLASH_TYPEERASE_SECTORS,
        .Banks         = FLASH_BANK_1,
        .Sector        = sector,
        .NbSectors     = 1,
        .VoltageRange  = FLASH_VOLTAGE_RANGE_1
    };

    status = HAL_FLASHEx_Erase(&erase_config, &sector_error);
    HAL_FLASH_Lock();

    return (status == HAL_OK);
}

/**
  * @brief  Erase multiple sectors
  */
bool Flash_EraseRange(uint32_t start_addr, uint32_t size_bytes) {
    uint32_t sector_start = Flash_GetSectorFromAddress(start_addr);
    uint32_t sector_end = Flash_GetSectorFromAddress(start_addr + size_bytes - 1);
    uint32_t sector_count = sector_end - sector_start + 1;

    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase_config = {
        .TypeErase     = FLASH_TYPEERASE_SECTORS,
        .Banks         = FLASH_BANK_1,
        .Sector        = sector_start,
        .NbSectors     = sector_count,
        .VoltageRange  = FLASH_VOLTAGE_RANGE_1
    };

    uint32_t sector_error = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase_config, &sector_error);
    HAL_FLASH_Lock();

    return (status == HAL_OK);
}

/**
  * @brief  Write data to flash
  */
bool Flash_WriteData(uint32_t address, const uint8_t *data, uint32_t size_bytes) {
    HAL_StatusTypeDef status;

    // Check alignment
    if ((address % 32) != 0) {
        return false;
    }

    HAL_FLASH_Unlock();

    // Write in 32-byte chunks
    uint32_t words_to_write = size_bytes / 32;
    for (uint32_t i = 0; i < words_to_write; i++) {
        uint32_t offset = i * 32;
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                                   address + offset,
                                   (uint32_t)(data + offset));
        if (status != HAL_OK) {
            HAL_FLASH_Lock();
            return false;
        }
    }

    HAL_FLASH_Lock();
    return true;
}

/**
  * @brief  Read data from flash
  */
bool Flash_ReadData(uint32_t address, uint8_t *buffer, uint32_t size_bytes) {
    if (buffer == NULL || size_bytes == 0) {
        return false;
    }

    for (uint32_t i = 0; i < size_bytes; i++) {
        buffer[i] = *((uint8_t*)(address + i));
    }
    return true;
}

/**
  * @brief  Verify flash data
  */
bool Flash_VerifyData(uint32_t address, const uint8_t *data, uint32_t size_bytes) {
    for (uint32_t i = 0; i < size_bytes; i++) {
        uint8_t flash_byte = *((uint8_t*)(address + i));
        if (flash_byte != data[i]) {
            return false;
        }
    }
    return true;
}

/**
  * @brief  Jump to application
  */
void Flash_JumpToApplication(uint32_t app_address) {
    // Check if application vector table is valid
    if (*(uint32_t*)app_address == 0xFFFFFFFF) {
        while(1);
    }

    // Set stack pointer
    __set_MSP(*(uint32_t*)app_address);

    // Jump to reset handler
    void (*app_reset)(void) = (void (*)(void))(*(uint32_t*)(app_address + 4));
    app_reset();

    while(1);
}
