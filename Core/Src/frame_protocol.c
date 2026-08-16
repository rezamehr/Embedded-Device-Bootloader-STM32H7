/**
  ******************************************************************************
  * @file    frame_protocol.c
  * @brief   Frame protocol implementation
  * @version V1.0.0
  * @date    15-August-2026
  ******************************************************************************
  */

#include "frame_protocol.h"
#include "flash_operations.h"
#include "main.h"
#include "usart.h"
#include "string.h"
#include "crc32.h"
#include "aes.h"
#include "FreeRTOS.h"
#include "task.h"

/* ============================================================================
   External Variables and Functions
   ============================================================================ */
extern UART_HandleTypeDef huart6;
extern struct AES_ctx ctx;
extern uint8_t key[16];
extern uint8_t iv[16];

// Forward declaration of Uart_Transmit_DMA (defined in freertos.c)
extern uint8_t Uart_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, TickType_t timeout);

/* ============================================================================
   Private Function Prototypes
   ============================================================================ */
static void ProcessIdentity(const FrameRx_t *pFrame);
static void ProcessRead(const FrameRx_t *pFrame);
static void ProcessErase(const FrameRx_t *pFrame);
static void ProcessWrite(const FrameRx_t *pFrame);
static void ProcessJump(const FrameRx_t *pFrame);

/* ============================================================================
   Public Functions
   ============================================================================ */

/**
  * @brief  Calculate XOR checksum
  */
uint8_t CalculateXorChecksum(const uint8_t *data, uint8_t len) {
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

/**
  * @brief  Send response frame
  */
void SendResponse(uint8_t *payload, uint8_t payload_len) {
    uint8_t frame[MAX_FRAME_SIZE];
    uint8_t idx = 0;

    // START
    frame[idx++] = FRAME_START;

    // LENGTH
    frame[idx++] = payload_len;

    // PAYLOAD
    for (uint8_t i = 0; i < payload_len; i++) {
        frame[idx++] = payload[i];
    }

    // CHECKSUM (XOR PAYLOAD)
    frame[idx++] = CalculateXorChecksum(payload, payload_len);

    // ارسال فریم کامل
    Uart_Transmit_DMA(&huart6, frame, idx, pdMS_TO_TICKS(1000));
}

/**
  * @brief  Send ACK
  */
void SendAck(void) {
    uint8_t payload[] = {CMD_ACK};
    SendResponse(payload, sizeof(payload));
}

/**
  * @brief  Send NACK
  */
void SendNack(void) {
    uint8_t payload[] = {CMD_NACK};
    SendResponse(payload, sizeof(payload));
}

/**
  * @brief  Process received frame
  */
void ProcessReceivedFrame(FrameRx_t *pFrame) {
    if (pFrame == NULL || !pFrame->valid) {
        SendNack();
        return;
    }

    uint8_t cmd = pFrame->payload[0];

    switch (cmd) {
        case CMD_IDENTITY:
            ProcessIdentity(pFrame);
            break;
        case CMD_READ:
            ProcessRead(pFrame);
            break;
        case CMD_ERASE:
            ProcessErase(pFrame);
            break;
        case CMD_WRITE:
            ProcessWrite(pFrame);
            break;
        case CMD_JUMP:
            ProcessJump(pFrame);
            break;
        default:
            SendNack();
            break;
    }
}

/**
  * @brief  Validate and process incoming data
  */
void ReceiveRX(struct rxINFO *pData) {
    if (pData == NULL) return;

    // ============================================================
    // Step 1: Frame Validation
    // ============================================================
    FrameRx_t frame;
    frame.valid = false;

    // Check minimum frame size
    if (pData->Sizebuffer < 3) {
        SendNack();
        return;
    }

    // Check START byte
    if (pData->Databuffer[0] != FRAME_START) {
        SendNack();
        return;
    }
    frame.start = pData->Databuffer[0];

    // Extract LENGTH
    frame.length = pData->Databuffer[1];

    // Verify LENGTH matches buffer size
    if (pData->Sizebuffer != (2 + frame.length + 1)) {
        SendNack();
        return;
    }

    // Copy PAYLOAD
    if (frame.length > MAX_PAYLOAD_SIZE) {
        SendNack();
        return;
    }

    for (uint8_t i = 0; i < frame.length; i++) {
        frame.payload[i] = pData->Databuffer[2 + i];
    }

    // Extract CHECKSUM
    frame.checksum = pData->Databuffer[2 + frame.length];

    // Verify CHECKSUM
    uint8_t calculated_checksum = CalculateXorChecksum(frame.payload, frame.length);
    if (calculated_checksum != frame.checksum) {
        SendNack();
        return;
    }

    // Extract Command
    if (frame.length > 0) {
        frame.cmd = frame.payload[0];
    } else {
        SendNack();
        return;
    }

    frame.valid = true;

    // ============================================================
    // Step 2: Process Valid Frame
    // ============================================================
    ProcessReceivedFrame(&frame);
}

/* ============================================================================
   Private Functions - Command Handlers
   ============================================================================ */

/**
  * @brief  Process IDENTITY command
  */
static void ProcessIdentity(const FrameRx_t *pFrame) {
    (void)pFrame; // Unused parameter

    // PAYLOAD: 12 bytes مطابق داکیومنت
    uint8_t response[12] = {
        CMD_ACK,                    // [0] = 0xFF
        0x50, 0x04,                 // [1..2] Device ID = 0x0450 (LE)
        0x00, 0x00, 0x20, 0x00,     // [3..6] Flash size = 2MB (0x00200000)
        0x00, 0x00, 0x02, 0x00,     // [7..10] Page size = 128KB (0x00020000)
        0x01                        // [11] Version = 1
    };
    SendResponse(response, sizeof(response));
}

/**
  * @brief  Process READ command
  */
static void ProcessRead(const FrameRx_t *pFrame) {
    // Check LENGTH: must be 7 (CMD + Address 4 + Size 2)
    if (pFrame->length != 7) {
        SendNack();
        return;
    }

    // Extract Address (LE)
    uint32_t readAddr = ((uint32_t)pFrame->payload[1]) |
                       ((uint32_t)pFrame->payload[2] << 8) |
                       ((uint32_t)pFrame->payload[3] << 16) |
                       ((uint32_t)pFrame->payload[4] << 24);

    // Extract Size (LE)
    uint16_t readSize = ((uint16_t)pFrame->payload[5]) |
                       ((uint16_t)pFrame->payload[6] << 8);

    // Limit size to prevent overflow
    if (readSize > 256) readSize = 256;

    // Buffer for data + ACK
    uint8_t buffer[512];
    buffer[0] = CMD_ACK;  // First byte = ACK

    // Read data from memory
    for (uint16_t i = 0; i < readSize; i++) {
        buffer[1 + i] = *((uint8_t*)(readAddr + i));
    }

    // Send response: ACK + data
    SendResponse(buffer, readSize + 1);
}

/**
  * @brief  Process ERASE command
  */
static void ProcessErase(const FrameRx_t *pFrame) {
    // Check LENGTH: must be 9 (CMD + Address 4 + Size 4)
    if (pFrame->length != 9) {
        SendNack();
        return;
    }

    // Extract Address (LE)
    uint32_t eraseAddr = ((uint32_t)pFrame->payload[1]) |
                        ((uint32_t)pFrame->payload[2] << 8) |
                        ((uint32_t)pFrame->payload[3] << 16) |
                        ((uint32_t)pFrame->payload[4] << 24);

    // Extract Size (LE)
    uint32_t eraseSize = ((uint32_t)pFrame->payload[5]) |
                        ((uint32_t)pFrame->payload[6] << 8) |
                        ((uint32_t)pFrame->payload[7] << 16) |
                        ((uint32_t)pFrame->payload[8] << 24);

    // Perform erase
    if (Flash_EraseRange(eraseAddr, eraseSize)) {
        SendAck();
    } else {
        SendNack();
    }
}

/**
  * @brief  Process WRITE command
  */
static void ProcessWrite(const FrameRx_t *pFrame) {
    // Minimum size: CMD(1) + Address(4) + Data(1) = 6
    if (pFrame->length < 6) {
        SendNack();
        return;
    }

    // Extract Address (LE)
    uint32_t writeAddr = ((uint32_t)pFrame->payload[1]) |
                        ((uint32_t)pFrame->payload[2] << 8) |
                        ((uint32_t)pFrame->payload[3] << 16) |
                        ((uint32_t)pFrame->payload[4] << 24);

    // Data length (remaining PAYLOAD except CMD and Address)
    uint8_t dataLen = pFrame->length - 5;

    // Check alignment (32 bytes for STM32H7)
    if (dataLen % 32 != 0) {
        SendNack();
        return;
    }

    // Write to flash
    if (Flash_WriteData(writeAddr, &pFrame->payload[5], dataLen)) {
        SendAck();
    } else {
        SendNack();
    }
}

/**
  * @brief  Process JUMP command
  */
static void ProcessJump(const FrameRx_t *pFrame) {
    // Check LENGTH: must be 5 (CMD + Address 4)
    if (pFrame->length != 5) {
        SendNack();
        return;
    }

    // Extract Address (LE)
    uint32_t jumpAddr = ((uint32_t)pFrame->payload[1]) |
                       ((uint32_t)pFrame->payload[2] << 8) |
                       ((uint32_t)pFrame->payload[3] << 16) |
                       ((uint32_t)pFrame->payload[4] << 24);

    // Send ACK (optional)
    SendAck();

    // Set boot flag
    __HAL_RCC_BKPRAM_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();
    *(uint32_t*)(BKP_BASE_ADDR + BKP_BOOT_FLAG_OFFSET) = BOOT_FLAG_APPLICATION;
    __DSB();
    __DMB();

    // Jump to application
    Flash_JumpToApplication(jumpAddr);
}
