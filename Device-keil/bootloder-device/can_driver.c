/**
  ******************************************************************************
  * @file    can_stm32.c
  * @author  Zhenglin R&D Driver Software Team
  * @version V1.0.0
  * @date    26/04/2015
  * @brief   This file is can_stm32 file.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
/* Standard includes. */
#include <stdlib.h>
#include <string.h>
#include "can_driver.h"
#include "stm32f3xx_hal.h"
#include "device.h"

#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
// <o> ±¾»úPCLK1µÄÆµÂÊ (Hz) <1-1000000000>
//     <i> Same as PCLK1
#define CAN_CLK               36000000
#define CAN_TX_TIMEOUT_MS     10
#define CAN_RX_TIMEOUT_MS     10

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


CAN_HandleTypeDef hcan;

uint8_t hex_convert(uint8_t nbr)
{
  if (nbr < 10)
  {
    return ('0' + nbr);
  }
  else if (nbr < 16)
  {
    return ('A' + nbr - 10);
  }
  else
    return '\0';
}

void printSCI_str(const char * str)
{
  Message msg;
  uint16_t len = strlen((const char *)str);

  str += len;

  msg.cob_id = ((uint16_t)FRAME_PRINTF << 7) + device_GetNodeid();
  msg.rtr = 0;
  msg.len = 8;
  while (len > 8)
  {
    memcpy(msg.data, (str - len), 8);
    len -= 8;
    canSend(&msg);
  }
  
  msg.len = len;
  memcpy(msg.data, (str - len), len);
  msg.data[len] = 0;
  canSend(&msg);

//  msg.len = 1;
//  msg.data[0] = 0;
//  canSend(&msg);
    
}

void printSCI_nbr(uint32_t nbr, uint8_t lastChar)
{
  uint8_t buf[12] = {0};
  uint8_t i, flag = 0;
  uint8_t j = 0;

  buf[j++] = '0';  
  buf[j++] = 'x';

  for (i = 1; i <= 8; i++)
  {
    uint8_t tmp = 0x0f&(uint8_t)(nbr >> (32 - (i<<2)));
    if ((tmp != 0) || flag == 1)
    {
      buf[j++] = hex_convert(tmp);
      flag = 1;
    }
  }

  buf[j++] = lastChar;
  buf[j++] = '\0';

  printSCI_str((const char *)buf);  
}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(canHandle->Instance==CAN)
  {
  /* USER CODE BEGIN CAN_MspInit 0 */

  /* USER CODE END CAN_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
  
    /**CAN GPIO Configuration    
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN CAN_MspInit 1 */

  /* USER CODE END CAN_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN)
  {
  /* USER CODE BEGIN CAN_MspDeInit 0 */

  /* USER CODE END CAN_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();
  
    /**CAN GPIO Configuration    
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_LP_CAN_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN_RX1_IRQn);
  /* USER CODE BEGIN CAN_MspDeInit 1 */

  /* USER CODE END CAN_MspDeInit 1 */
  }
} 

/* CAN init function */
uint8_t canInit(CAN_PORT CANx)
{
  
  hcan.Init.SJW = CAN_SJW_1TQ;
  hcan.Init.BS1 = CAN_BS1_7TQ;
  hcan.Init.BS2 = CAN_BS2_1TQ;
  
  hcan.pTxMsg = (CanTxMsgTypeDef*)malloc(sizeof(CanTxMsgTypeDef));
  hcan.pRxMsg = (CanRxMsgTypeDef*)malloc(sizeof(CanRxMsgTypeDef));
  hcan.pRx1Msg = (CanRxMsgTypeDef*)malloc(sizeof(CanRxMsgTypeDef));
  hcan.Init.Prescaler  = 4;
  hcan.Instance = CANx;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.TTCM = DISABLE;
  hcan.Init.ABOM = DISABLE;
  hcan.Init.AWUM = DISABLE;
  hcan.Init.NART = DISABLE;
  hcan.Init.RFLM = DISABLE;
  hcan.Init.TXFP = ENABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
  {
    CAN_FilterConfTypeDef  sFilterConfig;
    /*##-2- Configure the CAN Filter ###########################################*/
    sFilterConfig.FilterNumber = 0;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = NodeId<<5;
    sFilterConfig.FilterIdLow = 0;
    sFilterConfig.FilterMaskIdHigh = 0x7f<<5;
    sFilterConfig.FilterMaskIdLow = 0;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
    {
      /* Filter configuration Error */
      _Error_Handler(__FILE__, __LINE__);
    }
    
    sFilterConfig.FilterNumber = 1;
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
    sFilterConfig.FilterIdHigh = 0xff<<5;
    sFilterConfig.FilterIdLow = 0;
    sFilterConfig.FilterMaskIdHigh = 0xff<<5;
    sFilterConfig.FilterMaskIdLow = 0;
    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    sFilterConfig.FilterActivation = ENABLE;
    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
    {
      /* Filter configuration Error */
      _Error_Handler(__FILE__, __LINE__);
    }
    HAL_CAN_Receive_IT(&hcan, CAN_FIFO0);               
       
    /*##-2- Configure the CAN Filter ###########################################*/
//    sFilterConfig.FilterNumber = 1;
//    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
//    sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
//    sFilterConfig.FilterIdHigh = 0x0000;
//    sFilterConfig.FilterIdLow = 0x0000;
//    sFilterConfig.FilterMaskIdHigh = 0x0000;
//    sFilterConfig.FilterMaskIdLow = 0x0000;
//    sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO1;
//    sFilterConfig.FilterActivation = ENABLE;
//    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
//    {
//      /* Filter configuration Error */
//      _Error_Handler(__FILE__, __LINE__);
//    }
//    HAL_CAN_Receive_IT(&hcan,CAN_FIFO1);
  }
    
  HAL_NVIC_EnableIRQ(USB_LP_CAN_RX0_IRQn);

  return 0;
}

/**
  * @brief  canSend
	* @param  CANx:CAN1 or CAN2   m:can message
  * @retval 0£ºSuccess
  */
uint8_t canSend(Message *m)	                
{
  HAL_StatusTypeDef ret;
  uint8_t i;

  hcan.pTxMsg->StdId = (uint32_t)(m->cob_id);
  hcan.pTxMsg->ExtId = 0x00;
  hcan.pTxMsg->RTR = m->rtr;								  
  hcan.pTxMsg->IDE = CAN_ID_STD;                           
  hcan.pTxMsg->DLC = m->len;
  
  for(i=0;i<m->len;i++)                                 
  {
    hcan.pTxMsg->Data[i] = m->data[i];
  }
  ret = HAL_CAN_Transmit(&hcan, CAN_TX_TIMEOUT_MS);
  if(ret == HAL_OK)
  {
      return 1;	
  }
  else 
  {
      return 0;    
  }
}
/**
  * @brief  This function handles CAN1 RX0 request.
  * @param  None
  * @retval None
  */
void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{  
  uint32_t i = 0;
				
	Message RxMSG = Message_Initializer;

  if ((hcan->pRxMsg != NULL) && (hcan->pRxMsg->FIFONumber == CAN_FIFO0))
  {
    RxMSG.cob_id = (uint16_t)(hcan->pRxMsg->StdId);
    RxMSG.rtr = hcan->pRxMsg->RTR;
    RxMSG.len = hcan->pRxMsg->DLC;
    for(i = 0;i < RxMSG.len; i++)
    {
      RxMSG.data[i] = hcan->pRxMsg->Data[i];
    }
  //	SEGGER_RTT_printf(0, "can master revcive data!!!!!!!!!!!!\n");
    canDispatch(&(RxMSG));
    HAL_CAN_Receive_IT(hcan,CAN_FIFO0);
  }
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef* hcan)
{
  if (hcan->ErrorCode == HAL_CAN_ERROR_FOV0)
  {}
}


/******************* (C) COPYRIGHT 2015 Personal Electronics *****END OF FILE****/
