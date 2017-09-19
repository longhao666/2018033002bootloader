/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BT_FLASH_H
#define __BT_FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif
#include "stdint.h"
   
#define LAST_PAGE_ADDR  0x0801F800
   
#define CONFIG_LEN      0x100
#define NODE_ID_OFFSET    0   
#define FW_VER_OFFSET     CONFIG_LEN - 16
#define FW_FLAG_OFFSET    FW_VER_OFFSET + 4
#define FW_ADDR_OFFSET    FW_VER_OFFSET + 8
#define FW_NBR_OFFSET     FW_VER_OFFSET + 12

extern uint8_t config_buf[CONFIG_LEN];
   
int32_t flash_erase_bytes(uint32_t addr, uint32_t bytes);
int32_t flash_read_bytes(uint32_t addr, uint8_t* pData, uint32_t len);
int32_t flash_write_halfwords(uint32_t addr, uint16_t* pData, uint32_t len);
   
int32_t config_load(void);
int32_t config_verify(void);

#ifdef __cplusplus
}
#endif
#endif

