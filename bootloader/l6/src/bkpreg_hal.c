#if 0
// pull in the sources from the HAL. It's a bit of a hack, but is simpler than trying to link the
// full hal library.
#include "../src/lora/bkpreg_hal.c"

#else

#include "bkpreg_hal.h"
#include "hw_config.h"

extern RTC_HandleTypeDef RtcHandle;
/*
 * @brief Find RTC backup registers according the number.
 * @param BKP_DR: backup register number, 0-19.
 * @retval RTC Backup register definition.
 */
int32_t HAL_Core_Backup_Register(uint32_t BKP_DR)
{
    switch (BKP_DR)
    {
        case BKP_DR_01:
            return RTC_BKP_DR1;
            break;
        case BKP_DR_02:
            return RTC_BKP_DR2;
            break;
        case BKP_DR_03:
            return RTC_BKP_DR3;
            break;
        case BKP_DR_04:
            return RTC_BKP_DR4;
            break;
        default:
            break;
    }
    return -1;
}

/**
* @brief Write data to RTC backup regitster.
* @param BKP_DR: Backup register number.
* @param data: The data to write.
* @retval None
*/
void HAL_Core_Write_Backup_Register(uint32_t BKP_DR, uint32_t Data)
{
    uint32_t BKP_DR_Address = HAL_Core_Backup_Register(BKP_DR);
    if (BKP_DR_Address != -1)
    {
        HAL_RTCEx_BKUPWrite(&RtcHandle, BKP_DR_Address, Data);
    }
}

/**
* @brief Read data from RTC backup regitster.
* @param BKP_DR: Backup register number.
* @retval The data in the specify the register.
*/
uint32_t HAL_Core_Read_Backup_Register(uint32_t BKP_DR)
{
    uint32_t BKP_DR_Address = HAL_Core_Backup_Register(BKP_DR);
    if (BKP_DR_Address != -1)
    {
        return HAL_RTCEx_BKUPRead(&RtcHandle, BKP_DR_Address);
    }
    return 0xFFFFFFFF;
}

#endif
