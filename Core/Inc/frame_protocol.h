/**
  ******************************************************************************
  * @file    frame_protocol.h
  * @brief   Frame protocol definitions and handlers
  * @version V1.0.0
  * @date    15-August-2026
  ******************************************************************************
  */

#ifndef FRAME_PROTOCOL_H
#define FRAME_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "bootloader_config.h"

/* ============================================================================
   Forward Declaration of rxINFO
   ============================================================================ */
struct rxINFO {
    uint8_t Sizebuffer;
    uint8_t Databuffer[RX_BUF_SIZE];
};

/* ============================================================================
   Type Definitions
   ============================================================================ */

/**
  * @brief  Frame information structure
  */
typedef struct {
    uint8_t start;              //0xAA
    uint8_t length;             // length PAYLOAD
    uint8_t payload[MAX_PAYLOAD_SIZE];
    uint8_t checksum;           // XOR َAll PAYLOAD
    uint8_t cmd;                //  First byte PAYLOAD = Command
    bool valid;                 // frame valid?
} FrameRx_t;

/* ============================================================================
   Function Prototypes
   ============================================================================ */

/**
  * @brief  Calculate XOR checksum of data
  * @param  data: Pointer to data buffer
  * @param  len: Length of data
  * @retval XOR checksum value
  */
uint8_t CalculateXorChecksum(const uint8_t *data, uint8_t len);

/**
  * @brief  Send response frame
  * @param  payload: Pointer to payload data
  * @param  payload_len: Length of payload
  */
void SendResponse(uint8_t *payload, uint8_t payload_len);

/**
  * @brief  Send ACK response
  */
void SendAck(void);

/**
  * @brief  Send NACK response
  */
void SendNack(void);

/**
  * @brief  Process received frame
  * @param  pFrame: Pointer to frame structure
  */
void ProcessReceivedFrame(FrameRx_t *pFrame);

/**
  * @brief  Validate and process incoming data
  * @param  pData: Pointer to received data structure
  */
void ReceiveRX(struct rxINFO *pData);

#ifdef __cplusplus
}
#endif

#endif /* FRAME_PROTOCOL_H */
