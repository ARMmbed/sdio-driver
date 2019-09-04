/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
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
#include "fsl_sd.h"
#include "pinmap.h"
#include "sdio_api.h"

#if DEVICE_SDIO

static sd_card_t g_sd;

extern void sdio_clock_setup(void);

int sdio_init(void)
{
    uint32_t reg;

    /*! @brief SDMMC host detect card configuration */
    sdmmchost_detect_card_t s_sdCardDetect = {
        .cdType = kSDMMCHOST_DetectCardByGpioCD,
        .cdTimeOut_ms = (~0U),
    };

    sdio_clock_setup();

    /* SD POW_EN */
    pin_function(P0_9, 2);
    pin_mode(P0_9, PullNone);

    /* SD DAT3 */
    pin_function(P1_0, 2);
    pin_mode(P1_0, PullNone);
    reg = IOCON->PIO[1][0];
    reg |= IOCON_PIO_SLEW_MASK;
    IOCON->PIO[1][0] = reg;

    /* SD DAT2 */
    pin_function(P0_31, 2);
    pin_mode(P0_31, PullNone);
    reg = IOCON->PIO[0][31];
    reg |= IOCON_PIO_SLEW_MASK;
    IOCON->PIO[0][31] = reg;

    /* SD DAT1 */
    pin_function(P0_25, 2);
    pin_mode(P0_25, PullNone);
    reg = IOCON->PIO[0][25];
    reg |= IOCON_PIO_SLEW_MASK;
    IOCON->PIO[0][25] = reg;

    /* SD DAT0 */
    pin_function(P0_24, 2);
    pin_mode(P0_24, PullNone);
    reg = IOCON->PIO[0][24];
    reg |= IOCON_PIO_SLEW_MASK;
    IOCON->PIO[0][24] = reg;

    /* SD CLK */
    pin_function(P0_7, 2);
    pin_mode(P0_7, PullNone);
    reg = IOCON->PIO[0][7];
    reg |= IOCON_PIO_SLEW_MASK;
    IOCON->PIO[0][7] = reg;

    /* SD CMD */
    pin_function(P0_8, 2);
    pin_mode(P0_8, PullNone);
    reg = IOCON->PIO[0][8];
    reg |= IOCON_PIO_SLEW_MASK;
    IOCON->PIO[0][8] = reg;

    g_sd.host.base = SD_HOST_BASEADDR;
    g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
    /* card detect type */
    g_sd.usrParam.cd = &s_sdCardDetect;
#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
    g_sd.usrParam.pwr = &s_sdCardPwrCtrl;
#endif

    /* SD host init function */
    if (SD_Init(&g_sd) != kStatus_Success) {
        return MSD_ERROR;
    }

    return MSD_OK;
}

int sdio_deinit(void)
{
    SD_Deinit(&g_sd);

    return MSD_OK;
}

int sdio_read_blocks(uint8_t *data, uint32_t address, uint32_t block_count)
{
    int sd_state = MSD_OK;

    if (SD_ReadBlocks(&g_sd, data, address, block_count) != kStatus_Success) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

int sdio_write_blocks(uint8_t *data, uint32_t address, uint32_t block_count)
{
    int sd_state = MSD_OK;

    if (SD_WriteBlocks(&g_sd, data, address, block_count) != kStatus_Success) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

int sdio_erase(uint32_t start_address, uint32_t end_address)
{
    int sd_state = MSD_OK;

    uint32_t blocks = (end_address - start_address) / g_sd.blockSize;
    if (SD_EraseBlocks(&g_sd, start_address, blocks) != kStatus_Success) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

int sdio_get_card_state(void)
{
    int sd_state = MSD_OK;

    if (!SD_CheckReadOnly(&g_sd)) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

void sdio_get_card_info(sdio_card_info_t *card_info)
{
    card_info->card_type = 4;
    card_info->card_version = g_sd.version;
    card_info->card_class = 0;
    card_info->rel_card_addr = g_sd.relativeAddress;
    card_info->block_count = g_sd.blockCount;
    card_info->block_size = g_sd.blockSize;
    card_info->log_block_count = g_sd.blockCount;
    card_info->log_block_size = g_sd.blockSize;
}

#endif // DEVICE_SDIO
