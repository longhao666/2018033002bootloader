#include <stdint.h>
#include <string.h>
#include "stm32f3xx_hal.h"
#include "device.h"
#include "flash.h"

uint32_t download_addr = 0;
uint32_t download_len = 0;
uint32_t downloaded_len = 0;

uint32_t upload_addr = 0;
uint32_t upload_len = 0;
uint32_t uploaded_len = 0;

uint8_t  download_buf[MAX_BUFF_SIZE];
uint8_t  upload_buf[MAX_BUFF_SIZE];

//uint8_t bootloader_trigged = 0;

uint8_t ack_1 = 1;
uint8_t ack_0 = 0;

void proceedCMD(Message *m);
void uploading(uint8_t cs);
void tx_ack(uint8_t cmdword, uint8_t *buf, uint8_t len);

uint16_t device_GetNodeid(void)
{
  return NodeId;
}

void device_SetNodeid(uint16_t id)
{
  flash_write_halfwords(CMDMAP_ADDR+2, &id, 1);
}
  
void device_Init(void)
{
  config_load();
  if (device_GetNodeid() == 0xFFFF)
  {
    device_SetNodeid(1);
  }
  
}

void device_SendInfo(uint8_t option)
{
  uint8_t buf[7];
  uint16_t hAux;
  uint32_t wAux;

  buf[0] = option;

  if ((option & INFO_FW_VERSION) == INFO_FW_VERSION)
  {
    hAux = Firmware_Version;
    memcpy(&buf[1], &hAux, 2); 
    tx_ack(CMD_READ_INFO, buf, 3);
  }

  if ((option &INFO_FW_FLAG) == INFO_FW_FLAG)
  {
    hAux = Firmware_UpdateFlag;
    memcpy(&buf[1], &hAux, 2);
    tx_ack(CMD_READ_INFO, buf, 3);
  }

  if ((option &INFO_FW_ADDRESS) == INFO_FW_ADDRESS)
  {
    wAux = Firmware_FlashAdress;
    memcpy(&buf[1], &wAux, 4); 
    tx_ack(CMD_READ_INFO, buf, 5);
  }

  if ((option &INFO_FW_BYTE_NBR) == INFO_FW_BYTE_NBR)
  {
    wAux = Firmware_FlashByteNbr;
    memcpy(&buf[1], &wAux, 4); 
    tx_ack(CMD_READ_INFO, buf, 5);
  }
}

void device_SetInfo(Message *m)
{
  uint8_t option = m->data[1];
  uint8_t buf[7];

  buf[0] = option;
  buf[1] = 1;

  switch (option)
  {
    case INFO_FW_FLAG:
      memcpy(&config_buf[1], &(m->data[2]), 2);
      break;
    case INFO_FW_ADDRESS:
      memcpy(&config_buf[2], &(m->data[2]), 4);
      break;
    case INFO_FW_BYTE_NBR:
      memcpy(&config_buf[4], &(m->data[2]), 4);
      break;
    default:
      buf[1] = 0;
      break;
  }  

  tx_ack(CMD_WRITE_INFO, buf, 2);
  
  config_verify();
}

void NVIC_DeInit(void)
{
  uint8_t tmp;

  /* Disable all interrupts */
  NVIC->ICER[0] = 0xFFFFFFFF;
  NVIC->ICER[1] = 0x00000001;
  /* Clear all pending interrupts */
  NVIC->ICPR[0] = 0xFFFFFFFF;
  NVIC->ICPR[1] = 0x00000001;

  /* Clear all interrupt priority */
  for (tmp = 0; tmp < 32; tmp++) {
      NVIC->IP[tmp] = 0x00;
  }
}

void NVIC_SCBDeInit(void)
{
  uint8_t tmp;

  SCB->ICSR = 0x0A000000;
  SCB->VTOR = 0x00000000;
  SCB->AIRCR = 0x05FA0000;
  SCB->SCR = 0x00000000;
  SCB->CCR = 0x00000000;

  for (tmp = 0; tmp < 32; tmp++) {
      SCB->SHP[tmp] = 0x00;
  }

  SCB->SHCSR = 0x00000000;
  SCB->CFSR = 0xFFFFFFFF;
  SCB->HFSR = 0xFFFFFFFF;
  SCB->DFSR = 0xFFFFFFFF;
}
int8_t device_Run(void)
{
  if (Firmware_UpdateFlag == 1)
  {
//    MSG_PRINT(0x02, " Prepared to go into application! ", 0xff);
//    __HAL_RCC_CAN1_CLK_DISABLE();
    __disable_irq();
    __set_PRIMASK(1);
    if (((*(__IO uint32_t*)Firmware_FlashAdress) & 0x2FFE0000 ) == 0x20000000)
    {
      __ASM("CPSID  I");
      app_reset_fun app_reset = (app_reset_fun)*(__IO uint32_t*)(Firmware_FlashAdress + 4);
//      __set_BASEPRI(0);                                                                                            
//      __set_FAULTMASK(0);
//      __set_PSP(*((__IO uint32_t*)Firmware_FlashAdress)); 
      __set_CONTROL(0);        
      __set_MSP(*(__IO uint32_t*)Firmware_FlashAdress);         
      SysTick->CTRL = 0;
      SysTick->LOAD = 0;
      SysTick->VAL = 0;
//      HAL_NVIC_DisableIRQ(SysTick_IRQn);
      HAL_NVIC_DisableIRQ(USB_LP_CAN_RX0_IRQn);
      NVIC_DeInit();
      NVIC_SCBDeInit();
      SCB->VTOR = Firmware_FlashAdress;
      app_reset(); 
    }      
  }
  else
  {
//    MSG_PRINT(0x21, " No valid firmware! ", 0xff);
    return 0;
  }
  return 1;

}

//uint8_t device_GetTrigger(void)
//{
//  return bootloader_trigged;
//}

void device_SetTrigger(void)
{
  Message msg;
  msg.cob_id = 0x80;
  msg.rtr = 0;
  msg.len = 1;
  msg.data[0] = device_GetNodeid();
  canSend(&msg);
}

/*!                                                                                                
**                                                                                                 
**                                                                                                 
** @param m                                                                                        
**/  
void canDispatch(Message *m)
{
  uint16_t cob_id = UNS16_LE(m->cob_id);
  Message msg = Message_Initializer;
  const char trigger[8] = {0x22, 0x5A, 0x58, 0x46, 0x57, 0x50, 0x4D, 0x4A};

  if ((m->cob_id == 0xFF) && (0 == strncmp((const char*)m->data, (const char*)trigger, 8)))
  {
    device_SetTrigger();
    return;
  }

  switch(cob_id >> 7)
  {
    case FRAME_CMD:    /* can be a SYNC or a EMCY message */
      proceedCMD(m);
      break;
    case FRAME_DOWNLOADh:
      if (m->data[0]>>6)
      {
        return;
      }
      else if (m->data[0] == 0x21) /*  0x21 */
      {
        download_addr = (uint32_t)m->data[1] + ((uint32_t)m->data[2]<<8) + ((uint32_t)m->data[3]<<16) + ((uint32_t)m->data[4]<<24);
        download_len = (uint32_t)m->data[5] + ((uint32_t)m->data[6]<<8) + ((uint32_t)m->data[7]<<16);
        downloaded_len = 0;

        msg.cob_id = ((uint16_t)FRAME_DOWNLOADd << 7) + device_GetNodeid();
        msg.len = 8;
        msg.data[0] = 0x60;
        memcpy(&msg.data[1], &m->data[1], 7);
      }
      else if ((m->data[0] & 0x0F) == 0) /* 0x10 0x00 */
      {
        memcpy(&download_buf[downloaded_len % MAX_BUFF_SIZE], &m->data[1], 7);
        downloaded_len += 7;

        if (downloaded_len % MAX_BUFF_SIZE == 0)
        {
          flash_write_halfwords(download_addr, (uint16_t*)download_buf, MAX_BUFF_SIZE>>1);
          download_addr += MAX_BUFF_SIZE;
        }

        msg.cob_id = ((uint16_t)FRAME_DOWNLOADd << 7) + device_GetNodeid();
        msg.len = 8;
        msg.data[0] = 0x20 + m->data[0];
      }
      else   /* download complete */
      {
        uint8_t x = 9 - (((m->data[0] & 0x0F) + 1) >> 1);
        memcpy(&download_buf[downloaded_len % MAX_BUFF_SIZE], &m->data[1], x);
        downloaded_len += x;

        flash_write_halfwords(download_addr, (uint16_t*)download_buf, ((downloaded_len % MAX_BUFF_SIZE)+1)>>1);

        msg.cob_id = ((uint16_t)FRAME_DOWNLOADd << 7) + device_GetNodeid();
        msg.len = 8;
        msg.data[0] = 0x20 + (m->data[0] & 0xF0);
        MSG_PRINT(0x02, " Download completed! ", 0xff);
      }
      canSend(&msg);
      break;

    case FRAME_UPLOADh:  
      if (m->data[0]>>7)
      {
        return;
      }
      else if (m->data[0] == 0x40) /*  0x40 */
      {
        msg.cob_id = ((uint16_t)FRAME_UPLOADd << 7) + device_GetNodeid();
        msg.len = 8;

        upload_addr = (uint32_t)m->data[1] + ((uint32_t)m->data[2]<<8) + ((uint32_t)m->data[3]<<16) + ((uint32_t)m->data[4]<<24);
        upload_len = (uint32_t)m->data[5] + ((uint32_t)m->data[6]<<8) + ((uint32_t)m->data[7]<<16);
        uploaded_len = 0;

        msg.data[0] = 0x41;
        memcpy(&msg.data[1], &m->data[1], 7);        
        
        canSend(&msg);
      }
      else if ((m->data[0] == 0x60) || (m->data[0] == 0x70)) /* 0x60 */
      {
        uploading(m->data[0] - 0x60);
      }
      break;
    default:
      break;
  }
}

void proceedCMD(Message *m)
{
  uint32_t erase_addr;
  uint16_t bytes;
  uint8_t cmdword = m->data[0];

  switch (cmdword)
  {
    case CMD_READ_INFO:
      device_SendInfo(m->data[1]);
      break;
    case CMD_WRITE_INFO:
      device_SetInfo(m);
      break;
    case CMD_ERASE:
      erase_addr = (uint32_t)m->data[1] + ((uint32_t)m->data[2]<<8) + ((uint32_t)m->data[3]<<16) + ((uint32_t)m->data[4]<<24);
      bytes = (uint32_t)m->data[5] + ((uint32_t)m->data[6]<<8) + ((uint32_t)m->data[7]<<16);
      flash_erase_bytes(erase_addr, bytes);
      tx_ack(CMD_ERASE, &ack_1, 1);
      break;
    case CMD_RUN:
      HAL_NVIC_SystemReset();
//      device_Run();  /* will not continue if successfully running */
      tx_ack(CMD_RUN, &ack_0, 1); /* error occurred */
      break;
    default:
      break;
  }
}

/* DEVICE upload program to HOST */
void uploading(uint8_t cs)
{
  Message msg = Message_Initializer;

  msg.cob_id = ((uint16_t)FRAME_UPLOADd << 7) + device_GetNodeid();
  msg.rtr = 0;
  msg.data[0] = cs;
  msg.len = 8;

  if ((uploaded_len % MAX_BUFF_SIZE) == 0)
  {
    if ((upload_len - uploaded_len) > MAX_BUFF_SIZE)
    {
      flash_read_bytes(upload_addr, upload_buf, MAX_BUFF_SIZE);
      upload_addr +=  MAX_BUFF_SIZE;    
    }
    else
    {
      flash_read_bytes(upload_addr, upload_buf, upload_len - uploaded_len);
      upload_addr +=  upload_len - uploaded_len;    
    }
  }

  if (upload_len == uploaded_len)
  {
    MSG_PRINT(0x02, " Upload completed! ", 0xff);
    return ;
  }
  else if ((upload_len - uploaded_len) <= 7)
  {
    msg.data[0]  += ((9 - (upload_len - uploaded_len))<<1) - 1;

    for (uint32_t i = 0; i < (upload_len - uploaded_len); i++)
      msg.data[i + 1] = upload_buf[(uploaded_len % MAX_BUFF_SIZE) + i];
    uploaded_len = upload_len;
  }
  else
  {
    for (uint32_t i = 0; i < 7; i++)
      msg.data[i + 1] = upload_buf[(uploaded_len % MAX_BUFF_SIZE) + i];
    uploaded_len += 7;
  }
  canSend(&msg);
}

void tx_ack(uint8_t cmdword, uint8_t *buf, uint8_t len)
{
  Message msg;
  msg.cob_id = ((uint16_t)FRAME_ACK << 7) + device_GetNodeid();
  msg.rtr = 0;
  msg.len = len + 1;
  msg.data[0] = cmdword;

  for (uint8_t i = 0; i < len; i++)
    msg.data[1 + i] = buf[i];

  canSend(&msg);
}




