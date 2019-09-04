/** \addtogroup hal */
/** @{*/
/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
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
#ifndef MBED_SDIO_API_H
#define MBED_SDIO_API_H

#include "device.h"
#include "pinmap.h"

#if DEVICE_SDIO

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \defgroup SDIO Configuration Functions
 * @{
 */

typedef struct {
    uint32_t card_type;        /* Specifies the card Type                         */
    uint32_t card_version;     /* Specifies the card version                      */
    uint32_t card_class;       /* Specifies the class of the card class           */
    uint32_t rel_card_addr;    /* Specifies the Relative Card Address             */
    uint32_t block_count;      /* Specifies the Card Capacity in blocks           */
    uint32_t block_size;       /* Specifies one block size in bytes               */
    uint32_t log_block_count;  /* Specifies the Card logical Capacity in blocks   */
    uint32_t log_block_size;   /* Specifies logical block size in bytes           */
} sdio_card_info_t;

/**
  * @brief  SD status structure definition
  */
#define MSD_OK ((int)0x00)
#define MSD_ERROR ((int)0x01)

/**
  * @brief  SD transfer state definition
  */
#define SD_TRANSFER_OK ((int)0x00)
#define SD_TRANSFER_BUSY ((int)0x01)

/**
 * @brief  Initializes the SD card device.
 * @retval SD status
 */
int sdio_init(void);

/**
 * @brief  DeInitializes the SD card device.
 * @retval SD status
 */
int sdio_deinit(void);

/**
 * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
 * @param  data: Pointer to the buffer that will contain the data to transmit
 * @param  address: Address from where data is to be read
 * @param  block_count: Number of SD blocks to read
 * @retval SD status
 */
int sdio_read_blocks(uint8_t *data, uint32_t address, uint32_t block_count);

/**
 * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
 * @param  data: Pointer to the buffer that will contain the data to transmit
 * @param  address: Address from where data is to be written
 * @param  block_count: Number of SD blocks to write
 * @retval SD status
 */
int sdio_write_blocks(uint8_t *data, uint32_t address, uint32_t block_count);

#if DEVICE_SDIO_ASYNC

/**
 * @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
 * @param  data: Pointer to the buffer that will contain the data to transmit
 * @param  address: Address from where data is to be read
 * @param  block_count: Number of SD blocks to read
 * @retval SD status
 */
int sdio_read_blocks_async(uint8_t *data, uint32_t address, uint32_t block_count);

/**
 * @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
 * @param  data: Pointer to the buffer that will contain the data to transmit
 * @param  address: Address from where data is to be written
 * @param  block_count: Number of SD blocks to write
 * @retval SD status
 */
int sdio_write_blocks_async(uint8_t *data, uint32_t address, uint32_t block_count);

/**
 * @brief  Check if a DMA operation is pending
 * @retval DMA operation is pending
 *          This value can be one of the following values:
 *            @arg  SD_TRANSFER_OK: No data transfer is acting
 *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
 */
int sdio_read_pending(void);

/**
 * @brief  Check if a DMA operation is pending
 * @retval DMA operation is pending
 *          This value can be one of the following values:
 *            @arg  SD_TRANSFER_OK: No data transfer is acting
 *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
 */
int sdio_write_pending(void);

#endif // DEVICE_SDIO_ASYNC

/**
 * @brief  Erases the specified memory area of the given SD card.
 * @param  start_address: Start byte address
 * @param  end_address: End byte address
 * @retval SD status
 */
int sdio_erase(uint32_t start_address, uint32_t end_address);

/**
 * @brief  Gets the current SD card data status.
 * @param  None
 * @retval Data transfer state.
 *          This value can be one of the following values:
 *            @arg  SD_TRANSFER_OK: No data transfer is acting
 *            @arg  SD_TRANSFER_BUSY: Data transfer is acting
 */
int sdio_get_card_state(void);

/**
 * @brief  Get SD information about specific SD card.
 * @param  card_info: Pointer to HAL_SD_CardInfoTypedef structure
 * @retval None
 */
void sdio_get_card_info(sdio_card_info_t *card_info);

/**@}*/

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // DEVICE_SDIO

#endif // MBED_SDIO_API_H
