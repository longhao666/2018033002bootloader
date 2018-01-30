/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BT_FLASH_H
#define __BT_FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif
#include "stdint.h"
   
#define CONFIG_ADDR  0x08039000
#define CONFIG_LEN      20
//#define NODE_ID_OFFSET    1   
//#define FW_VER_OFFSET     3
//#define FW_FLAG_OFFSET    194
//#define FW_ADDR_OFFSET    196
//#define FW_NBR_OFFSET     198

extern uint16_t config_buf[CONFIG_LEN];
   
int32_t flash_erase_bytes(uint32_t addr, uint32_t bytes);
int32_t flash_read_bytes(uint32_t addr, uint8_t* pData, uint32_t len);
int32_t flash_read_halfwords(uint32_t addr, uint16_t* pData, uint32_t len);
int32_t flash_write_halfwords(uint32_t addr, uint16_t* pData, uint32_t len);
   
int32_t config_load(void);
int32_t config_verify(void);

#ifdef __cplusplus
}
#endif
#endif

