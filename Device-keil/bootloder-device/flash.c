#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_flash.h"
#include "stm32f3xx_hal_flash_ex.h"
#include "flash.h"

uint16_t config_buf[CONFIG_LEN] = {0};  /* map to last page of flash */

/**
  * @brief  Program a half-word (16-bit) at a specified address.
  * @param  Address specify the address to be programmed.
  * @param  Data    specify the data to be programmed.
  * @retval None
  */
static void FLASH_Program_HalfWord(uint32_t Address, uint16_t Data)
{
  /* Clean the error context */
//  pFlash.ErrorCode = HAL_FLASH_ERROR_NONE;
  
  /* Proceed to program the new data */
  SET_BIT(FLASH->CR, FLASH_CR_PG);

  /* Write data in the address */
  *(__IO uint16_t*)Address = Data;
}


int32_t flash_erase_bytes(uint32_t addr, uint32_t bytes)
{
  int32_t retval = 0;
  
  FLASH_EraseInitTypeDef e;
  uint32_t err = 0;
  e.TypeErase = FLASH_TYPEERASE_PAGES;
  e.PageAddress = addr;
  e.NbPages = (FLASH_PAGE_SIZE + bytes)/FLASH_PAGE_SIZE;
  
  HAL_FLASH_Unlock();
  
  if (HAL_FLASHEx_Erase(&e, &err) == HAL_OK)
    retval = 1;
  
  HAL_FLASH_Lock();
  return retval;
}

int32_t flash_read_bytes(uint32_t addr, uint8_t* pData, uint32_t len)
{
  uint32_t srcAddr = addr;
  for (uint32_t i = 0; i < len; i++)
  {
    *(pData + i) = *(__IO uint8_t*)(srcAddr + i);
  }
  return 1;
}

int32_t flash_read_halfwords(uint32_t addr, uint16_t* pData, uint32_t len)
{
  uint32_t srcAddr = addr;
  for (uint32_t i = 0; i < len; i++)
  {
    *(pData + i) = *(__IO uint16_t*)(srcAddr + 2U*i);
  }
  return 1;
}

int32_t flash_write_halfwords(uint32_t addr, uint16_t* pData, uint32_t len)
{
  uint32_t index;
  
  HAL_StatusTypeDef status = HAL_OK;

  HAL_FLASH_Unlock();

  for (index = 0; index < len; index++)
  {
    FLASH_Program_HalfWord((addr + (2U*index)), (uint16_t)*(pData  + index));

    /* Wait for last operation to be completed */
    status = FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

    /* If the program operation is completed, disable the PG Bit */
    CLEAR_BIT(FLASH->CR, FLASH_CR_PG);

//    FLASHStatus  = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashDest, *(uint64_t*)FlashSrc);
    if (status != HAL_OK)
    {
      return 0;
    }
  }
  HAL_FLASH_Lock();
  return 1;
}

int32_t config_load(void)
{
  return flash_read_halfwords((uint32_t)CONFIG_ADDR, (uint16_t*)config_buf, CONFIG_LEN);
}
  
int32_t config_verify(void)
{
  if (!flash_erase_bytes(CONFIG_ADDR, CONFIG_LEN<<1))  /* erase brefore program */
  {
    return 0;
  }
  
  return flash_write_halfwords((uint32_t)CONFIG_ADDR, (uint16_t*)config_buf, CONFIG_LEN);
}
