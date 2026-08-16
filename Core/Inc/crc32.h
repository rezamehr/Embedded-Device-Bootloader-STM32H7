/*
 * crc32.h
 *
 *  Created on: Jun 1, 2026
 *      Author: Notebook
 */

#ifndef INC_CRC32_H_
#define INC_CRC32_H_

#include "stdint.h"


extern void updCRC32byte(uint8_t byteVal, uint32_t *crcVal);
extern void updCRC32(void* data,uint32_t cnt, uint32_t *crcVal);

#endif /* INC_CRC32_H_ */
