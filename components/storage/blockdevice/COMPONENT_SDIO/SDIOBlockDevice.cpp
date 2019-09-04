/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
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


#include "sdio_api.h"
#if DEVICE_SDIO_ASYNC
#include "us_ticker_api.h"
#endif
#include "SDIOBlockDevice.h"
#include "platform/mbed_debug.h"

using namespace mbed;

#if DEVICE_SDIO

/*
 *  defines
 */

#define SDIO_DBG 0       /*!< 1 - Enable debugging */
#define SDIO_CMD_TRACE 0 /*!< 1 - Enable SD command tracing */

#define SDIO_BLOCK_DEVICE_ERROR_WOULD_BLOCK -5001           /*!< operation would block */
#define SDIO_BLOCK_DEVICE_ERROR_UNSUPPORTED -5002           /*!< unsupported operation */
#define SDIO_BLOCK_DEVICE_ERROR_PARAMETER -5003             /*!< invalid parameter */
#define SDIO_BLOCK_DEVICE_ERROR_NO_INIT -5004               /*!< uninitialized */
#define SDIO_BLOCK_DEVICE_ERROR_NO_DEVICE -5005             /*!< device is missing or not connected */
#define SDIO_BLOCK_DEVICE_ERROR_WRITE_PROTECTED -5006       /*!< write protected */
#define SDIO_BLOCK_DEVICE_ERROR_UNUSABLE -5007              /*!< unusable card */
#define SDIO_BLOCK_DEVICE_ERROR_NO_RESPONSE -5008           /*!< No response from device */
#define SDIO_BLOCK_DEVICE_ERROR_CRC -5009                   /*!< CRC error */
#define SDIO_BLOCK_DEVICE_ERROR_ERASE -5010                 /*!< Erase error: reset/sequence */
#define SDIO_BLOCK_DEVICE_ERROR_WRITE -5011                 /*!< SPI Write error: !SPI_DATA_ACCEPTED */
#define SDIO_BLOCK_DEVICE_ERROR_UNSUPPORTED_BLOCKSIZE -5012 /*!< unsupported blocksize, only 512 byte supported */
#define SDIO_BLOCK_DEVICE_ERROR_READ_BLOCKS -5013           /*!< read data blocks from SD failed */
#define SDIO_BLOCK_DEVICE_ERROR_WRITE_BLOCKS -5014          /*!< write data blocks to SD failed */
#define SDIO_BLOCK_DEVICE_ERROR_ERASE_BLOCKS -5015          /*!< erase data blocks to SD failed */

#define BLOCK_SIZE_HC 512 /*!< Block size supported for SD card is 512 bytes  */

// Types
#define SDCARD_NONE 0  /**< No card is present */
#define SDCARD_V1 1    /**< v1.x Standard Capacity */
#define SDCARD_V2 2    /**< v2.x Standard capacity SD card */
#define SDCARD_V2HC 3  /**< v2.x High capacity SD card */
#define CARD_UNKNOWN 4 /**< Unknown or unsupported card */

SDIOBlockDevice::SDIOBlockDevice(PinName card_detect) : _card_detect(card_detect),
                                                        _is_initialized(0),
                                                        _sectors(0),
                                                        _init_ref_count(0)
{
    // Only HC block size is supported.
    _block_size = BLOCK_SIZE_HC;
    _erase_size = BLOCK_SIZE_HC;
}

SDIOBlockDevice::~SDIOBlockDevice()
{
    if (_is_initialized) {
        deinit();
    }
}

int SDIOBlockDevice::init()
{
    debug_if(SDIO_DBG, "init Card...\r\n");

    lock();

    if (!_is_initialized) {
        _init_ref_count = 0;
    }

    _init_ref_count++;

    if (_init_ref_count != 1) {
        unlock();
        return BD_ERROR_OK;
    }

    if (is_present() == false) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_NO_DEVICE;
    }

    int status = sdio_init();
    if (BD_ERROR_OK != status) {
        unlock();
        return BD_ERROR_DEVICE_ERROR;
    }

    sdio_get_card_info(&_card_info);
    _is_initialized = true;
    debug_if(SDIO_DBG, "SDIO initialized: type: %lu  version: %lu  class: %lu\n",
             _card_info.card_type, _card_info.card_version, _card_info.card_class);
    debug_if(SDIO_DBG, "SDIO size: %lu MB\n",
             _card_info.log_block_count / 2 / 1024);

    // get sectors count from card_info
    _sectors = _card_info.log_block_count;
    if (BLOCK_SIZE_HC != _card_info.block_size) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_UNSUPPORTED_BLOCKSIZE;
    }

    unlock();
    return status;
}

int SDIOBlockDevice::deinit()
{
    debug_if(SDIO_DBG, "deinit SDIO Card...\r\n");
    lock();

    if (!_is_initialized) {
        _init_ref_count = 0;
        unlock();
        return BD_ERROR_OK;
    }

    _init_ref_count--;

    if (_init_ref_count) {
        unlock();
        return BD_ERROR_OK;
    }

    int status = sdio_deinit();
    _is_initialized = false;

    _sectors = 0;

    unlock();
    return status;
}

int SDIOBlockDevice::read(void *buffer, bd_addr_t addr, bd_size_t size)
{
    int status = 0;
    lock();
    if (is_present() == false) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_NO_DEVICE;
    }

    if (!_is_initialized) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_NO_INIT;
    }

    if (!is_valid_read(addr, size)) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_PARAMETER;
    }

    uint8_t *_buffer = static_cast<uint8_t *>(buffer);

    // ReadBlocks uses byte unit address
    // SDHC and SDXC Cards different addressing is handled in ReadBlocks()
    bd_addr_t block_count = size / _block_size;
    addr = addr / _block_size;

#if DEVICE_SDIO_ASYNC
    // make sure card is ready
    {
        uint32_t tickstart = us_ticker_read();
        while (sdio_get_card_state() != SD_TRANSFER_OK) {
            // wait until SD ready
            if ((us_ticker_read() - tickstart) >= MBED_CONF_SDIO_CMD_TIMEOUT) {
                unlock();
                return SDIO_BLOCK_DEVICE_ERROR_READ_BLOCKS;
            }
        }
    }

    // receive the data : one block/ multiple blocks is handled in ReadBlocks()
    status = sdio_read_blocks_async(_buffer, addr, block_count);
    debug_if(SDIO_DBG, "SDIO read blocks dbgtest addr: %lld  block_count: %lld \n", addr, block_count);

    if (status == MSD_OK) {
        // wait until DMA finished
        uint32_t tickstart = us_ticker_read();
        while (sdio_read_pending() != SD_TRANSFER_OK) {
            if ((us_ticker_read() - tickstart) >= MBED_CONF_SDIO_CMD_TIMEOUT) {
                unlock();
                return SDIO_BLOCK_DEVICE_ERROR_READ_BLOCKS;
            }
        }
        // make sure card is ready
        tickstart = us_ticker_read();
        while (sdio_get_card_state() != SD_TRANSFER_OK) {
            // wait until SD ready
            if ((us_ticker_read() - tickstart) >= MBED_CONF_SDIO_CMD_TIMEOUT) {
                unlock();
                return SDIO_BLOCK_DEVICE_ERROR_READ_BLOCKS;
            }
        }
    } else {
        debug_if(SDIO_DBG, "SDIO read_blocks failed! addr: %lld  block_count: %lld \n", addr, block_count);
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_READ_BLOCKS;
    }
#else
    status = sdio_read_blocks(_buffer, addr, block_count);
    debug_if(SDIO_DBG, "SDIO read blocks dbgtest addr: %lld  block_count: %lld \n", addr, block_count);

    if (status != MSD_OK) {
        debug_if(SDIO_DBG, "SDIO read blocks failed! addr: %lld  block_count: %lld \n", addr, block_count);
        status = SDIO_BLOCK_DEVICE_ERROR_READ_BLOCKS;
    }
#endif

    unlock();
    return status;
}

int SDIOBlockDevice::program(const void *buffer, bd_addr_t addr, bd_size_t size)
{
    int status = 0;
    lock();

    if (is_present() == false) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_NO_DEVICE;
    }

    if (!_is_initialized) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_NO_INIT;
    }

    if (!is_valid_program(addr, size)) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_PARAMETER;
    }

    uint8_t *_buffer = (uint8_t *)(buffer);

    // Get block count
    bd_size_t block_count = size / _block_size;
    addr = addr / _block_size;

#if DEVICE_SDIO_ASYNC
    // make sure card is ready
    {
        uint32_t tickstart = us_ticker_read();
        while (sdio_get_card_state() != SD_TRANSFER_OK) {
            // wait until SD ready
            if ((us_ticker_read() - tickstart) >= MBED_CONF_SDIO_CMD_TIMEOUT) {
                unlock();
                return SDIO_BLOCK_DEVICE_ERROR_WRITE_BLOCKS;
            }
        }
    }

    status = sdio_write_blocks_async(_buffer, addr, block_count);
    debug_if(SDIO_DBG, "SDIO write blocks async dbgtest addr: %lld  block_count: %lld \n", addr, block_count);

    if (status == MSD_OK) {
        // wait until DMA finished
        uint32_t tickstart = us_ticker_read();
        while (sdio_write_pending() != SD_TRANSFER_OK) {
            if ((us_ticker_read() - tickstart) >= MBED_CONF_SDIO_CMD_TIMEOUT) {
                unlock();
                return SDIO_BLOCK_DEVICE_ERROR_WRITE_BLOCKS;
            }
        }
        // make sure card is ready
        tickstart = us_ticker_read();
        while (sdio_get_card_state() != SD_TRANSFER_OK) {
            // wait until SD ready
            if ((us_ticker_read() - tickstart) >= MBED_CONF_SDIO_CMD_TIMEOUT) {
                unlock();
                return SDIO_BLOCK_DEVICE_ERROR_WRITE_BLOCKS;
            }
        }
    } else {
        debug_if(SDIO_DBG, "SDIO write blocks async failed! addr: %lld  block_count: %lld \n", addr, block_count);
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_WRITE_BLOCKS;
    }
#else
    status = sdio_write_blocks(_buffer, addr, block_count);

    debug_if(SDIO_DBG, "SDIO write blocks dbgtest addr: %lld  block_count: %lld \n", addr, block_count);

    if (status != MSD_OK) {
        debug_if(SDIO_DBG, "SDIO write blocks failed! addr: %lld  block_count: %lld \n", addr, block_count);
        status = SDIO_BLOCK_DEVICE_ERROR_WRITE_BLOCKS;
    }
#endif

    unlock();
    return status;
}

int SDIOBlockDevice::trim(bd_addr_t addr, bd_size_t size)
{
    debug_if(SDIO_DBG, "SDIO trim Card...\r\n");
    lock();
    if (is_present() == false) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_NO_DEVICE;
    }

    if (!_is_initialized) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_NO_INIT;
    }

    if (!_is_valid_trim(addr, size)) {
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_PARAMETER;
    }

    bd_size_t block_count = size / _block_size;
    addr = addr / _block_size;

    int status = sdio_erase(addr, block_count);
    if (status != 0) {
        debug_if(SDIO_DBG, "SDIO erase blocks failed! addr: %lld  block_count: %lld \n", addr, block_count);
        unlock();
        return SDIO_BLOCK_DEVICE_ERROR_ERASE_BLOCKS;
#if DEVICE_SDIO_ASYNC
    } else {
        uint32_t tickstart = us_ticker_read();
        while (sdio_get_card_state() != SD_TRANSFER_OK) {
            // wait until SD ready
            if ((us_ticker_read() - tickstart) >= MBED_CONF_SDIO_CMD_TIMEOUT) {
                unlock();
                return SDIO_BLOCK_DEVICE_ERROR_ERASE_BLOCKS;
            }
        }
#endif
    }

    unlock();
    return status;
}

bd_size_t SDIOBlockDevice::get_read_size() const
{
    return _block_size;
}

bd_size_t SDIOBlockDevice::get_program_size() const
{
    return _block_size;
}

bd_size_t SDIOBlockDevice::size() const
{
    return _block_size * _sectors;
}

void SDIOBlockDevice::debug(bool dbg)
{
}

bool SDIOBlockDevice::_is_valid_trim(bd_addr_t addr, bd_size_t size) const
{
    return (
               addr % _erase_size == 0 &&
               size % _erase_size == 0 &&
               addr + size <= this->size());
}

bool SDIOBlockDevice::is_present(void)
{
    if (_card_detect.is_connected()) {
        return (_card_detect.read() == 0);
    } else {
        return true;
    }
}

const char *SDIOBlockDevice::get_type() const
{
    return "SDIO";
}

#endif //DEVICE_SDIO
