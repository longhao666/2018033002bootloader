/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BT_DEVICE_H
#define __BT_DEVICE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "can_driver.h"

typedef  void (*app_reset_fun)(void);       //

#define MAX_BUFF_SIZE  1120

#define CMDMAP_ADDR             0x08038000 

// from bootloader configure   
#define BL_CONFIG_BASE          CONFIG_ADDR // 0x08039000
#define BootTrigger             *(__IO uint16_t*)(BL_CONFIG_BASE)  //0
#define Firmware_UpdateFlag     *(__IO uint16_t*)(BL_CONFIG_BASE+2)//1
#define Firmware_FlashAdress    *(__IO uint32_t*)(BL_CONFIG_BASE+4)//2
#define Firmware_FlashByteNbr   *(__IO uint32_t*)(BL_CONFIG_BASE+8)//4

// from origin cmd map   
#define NodeId                  *(__IO uint16_t*)(CMDMAP_ADDR+2)
#define Firmware_Version        *(__IO uint16_t*)(CMDMAP_ADDR+6)
   
#define INFO_FW_VERSION   ((uint8_t)0x01 << 0)
#define INFO_FW_FLAG      ((uint8_t)0x01 << 1)
#define INFO_FW_ADDRESS   ((uint8_t)0x01 << 2)
#define INFO_FW_BYTE_NBR  ((uint8_t)0x01 << 3)
   
#define DEFAULT_FW_VERSION      0
#define DEFAULT_FW_FLASH_LEN    0

uint16_t device_GetNodeid(void);
void device_SetNodeid(uint16_t id);
void device_Init(void);
void device_SendInfo(uint8_t option);
void device_SetInfo(Message *m);
int8_t device_Run(void);
uint8_t device_GetTrigger(void);
void device_SetTrigger(void);
void canDispatch(Message *m);



#ifdef __cplusplus
}
#endif
#endif


