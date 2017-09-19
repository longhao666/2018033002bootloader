/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BT_DEVICE_H
#define __BT_DEVICE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "can_driver.h"


typedef  void (*app_reset_fun)(void);       //

#define MAX_BUFF_SIZE  1120
   
#define DEFAULT_FW_VERSION      0x12345678
#define DEFAULT_FW_FLASH_ADDR   0x08005000
#define DEFAULT_FW_FLASH_LEN    100

   
#define INFO_FW_VERSION   ((uint8_t)0x01 << 0)
#define INFO_FW_FLAG      ((uint8_t)0x01 << 1)
#define INFO_FW_ADDRESS   ((uint8_t)0x01 << 2)
#define INFO_FW_BYTE_NBR  ((uint8_t)0x01 << 3)

uint16_t device_GetNodeid(void);
void device_SetNodeid(uint16_t id);
void device_Init(void);
void device_SendInfo(uint8_t option);
void device_SetInfo(Message *m);
int8_t device_Run(void);
uint8_t device_GetTrigger(void);
void device_SetTrigger(uint8_t t);
void canDispatch(Message *m);



#ifdef __cplusplus
}
#endif
#endif


