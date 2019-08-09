/* mbed Microcontroller Library
 * Copyright (c) 2017 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __SDIO_DEVICE_H
#define __SDIO_DEVICE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <inttypes.h>

/* Exported constants */
/**
  * @brief  SD status structure definition
  */
#define MSD_OK ((uint8_t)0x00)
#define MSD_ERROR ((uint8_t)0x01)

/**
  * @brief  SD transfer state definition
  */
#define SD_TRANSFER_OK ((uint8_t)0x00)
#define SD_TRANSFER_BUSY ((uint8_t)0x01)

  /* Exported functions */
  uint8_t SDIO_Device_Init(void);
  uint8_t SDIO_Device_DeInit(void);
  uint8_t SDIO_Device_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout);
  uint8_t SDIO_Device_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout);
  uint8_t SDIO_Device_Erase(uint32_t StartAddr, uint32_t EndAddr, uint32_t Timeout);
  uint32_t SDIO_Device_GetBlockSize(void);
  uint32_t SDIO_Device_GetBlockCount(void);

#ifdef __cplusplus
}
#endif

#endif /* __SDIO_DEVICE_H */
