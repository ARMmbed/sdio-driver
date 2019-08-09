#include "sdio_device.h"
#include "sdio_device_stm.h"
#include "platform/mbed_debug.h"

#define SD_DBG 0       /*!< 1 - Enable debugging */

#ifndef MBED_CONF_SD_TIMEOUT
#define MBED_CONF_SD_TIMEOUT (30 * 1000) /* ms */
#endif

static SD_Cardinfo_t _card_info;

/**
 * @brief  Initializes the SD card device.
 * @retval SD status
 */
uint8_t SDIO_Device_Init(void)
{
    uint8_t sd_state = SD_Init();

    SD_GetCardInfo(&_card_info);

    debug_if(SD_DBG, "SD initialized: type: %lu  version: %lu  class: %lu\n",
	     _card_info.CardType, _card_info.CardVersion, _card_info.Class);
    debug_if(SD_DBG, "SD size: %lu MB\n",
             _card_info.LogBlockNbr / 2 / 1024);

    return sd_state;
}

/**
 * @brief  DeInitializes the SD card device.
 * @retval SD status
 */
uint8_t SDIO_Device_DeInit(void)
{
    return SD_DeInit();
}

/**
 * @brief  Reads block(s) from a specified address in an SD card, in polling mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  ReadAddr: Address from where data is to be read
 * @param  NumOfBlocks: Number of SD blocks to read
 * @param  Timeout: Timeout for read operation
 * @retval SD status
 */
uint8_t SDIO_Device_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
    // make sure card is ready
    {
        uint32_t tickstart = HAL_GetTick();
        while (SD_GetCardState() != SD_TRANSFER_OK)
        {
            // wait until SD ready
            if ((HAL_GetTick() - tickstart) >= Timeout)
            {
                return MSD_ERROR;
            }
        }
    }

    // receive the data : one block/ multiple blocks is handled in ReadBlocks()
    int status = SD_ReadBlocks_DMA(pData, ReadAddr, NumOfBlocks);

    if (status == MSD_OK)
    {
        // wait until DMA finished
        uint32_t tickstart = HAL_GetTick();
        while (SD_DMA_ReadPending() != SD_TRANSFER_OK)
        {
            if ((HAL_GetTick() - tickstart) >= Timeout)
            {
                return MSD_ERROR;
            }
        }
        // make sure card is ready
        tickstart = HAL_GetTick();
        while (SD_GetCardState() != SD_TRANSFER_OK)
        {
            // wait until SD ready
            if ((HAL_GetTick() - tickstart) >= Timeout)
            {
                return MSD_ERROR;
            }
        }
    }

    return status;
}

/**
 * @brief  Writes block(s) to a specified address in an SD card, in polling mode.
 * @param  pData: Pointer to the buffer that will contain the data to transmit
 * @param  WriteAddr: Address from where data is to be written
 * @param  NumOfBlocks: Number of SD blocks to write
 * @param  Timeout: Timeout for write operation
 * @retval SD status
 */
uint8_t SDIO_Device_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout)
{
    // make sure card is ready
    {
        uint32_t tickstart = HAL_GetTick();
        while (SD_GetCardState() != SD_TRANSFER_OK)
        {
            // wait until SD ready
            if ((HAL_GetTick() - tickstart) >= Timeout)
            {
                return MSD_ERROR;
            }
        }
    }

    int status = SD_WriteBlocks_DMA(pData, WriteAddr, NumOfBlocks);

    if (status == MSD_OK)
    {
        // wait until DMA finished
        uint32_t tickstart = HAL_GetTick();
        while (SD_DMA_WritePending() != SD_TRANSFER_OK)
        {
            if ((HAL_GetTick() - tickstart) >= Timeout)
            {
                return MSD_ERROR;
            }
        }
        // make sure card is ready
        tickstart = HAL_GetTick();
        while (SD_GetCardState() != SD_TRANSFER_OK)
        {
            // wait until SD ready
            if ((HAL_GetTick() - tickstart) >= Timeout)
            {
                return MSD_ERROR;
            }
        }
    }

    return status;
}

/**
 * @brief  Erases the specified memory area of the given SD card.
 * @param  StartAddr: Start byte address
 * @param  EndAddr: End byte address
 * @param  Timeout: Timeout for erase operation
 * @retval SD status
 */
uint8_t SDIO_Device_Erase(uint32_t StartAddr, uint32_t EndAddr, uint32_t Timeout)
{
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_Erase(&hsd, StartAddr, EndAddr) != HAL_OK)
    {
        sd_state = MSD_ERROR;
    }

    if (sd_state == MSD_OK)
    {
        uint32_t tickstart = HAL_GetTick();
        while (SD_GetCardState() != SD_TRANSFER_OK)
        {
            // wait until SD ready
            if ((HAL_GetTick() - tickstart) >= Timeout)
            {
                return MSD_ERROR;
            }
        }
    }

    return sd_state;
}

/**
 * @brief  Get the size of blocks in SD card.
 * @retval Block size
 */
uint32_t SDIO_Device_GetBlockSize(void)
{
    return _card_info.BlockSize;
}

/**
 * @brief  Get the number of blocks in SD card.
 * @retval Block count
 */
uint32_t SDIO_Device_GetBlockCount(void)
{
    return _card_info.BlockNbr;
}
