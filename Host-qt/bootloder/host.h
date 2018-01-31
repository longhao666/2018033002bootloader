#include <stdint.h>
#include "can_driver.h"

///HOST status
#define STATUS_IDLE         0
#define STATUS_DOWNLOADING  1
#define STATUS_UPLOADING    2
#define STATUS_ERROR        3

///firmware configure
#define DEF_CONFIG_ADDR     0x0803800
#define DEF_CONFIG_LEN      160

#define ACK_READ_BIT  ((uint16_t)0x01<<0)
#define ACK_WRITE_BIT ((uint16_t)0x01<<1)
#define ACK_ERASE_BIT ((uint16_t)0x01<<2)
#define ACK_RUN_BIT   ((uint16_t)0x01<<3)
#define ACK_UPLOAD_BIT   ((uint16_t)0x01<<4)
#define ACK_DOWNLOAD_BIT   ((uint16_t)0x01<<5)

#define INFO_FW_VERSION   ((uint8_t)0x01 << 0)
#define INFO_FW_FLAG      ((uint8_t)0x01 << 1)
#define INFO_FW_ADDRESS   ((uint8_t)0x01 << 2)
#define INFO_FW_BYTE_NBR  ((uint8_t)0x01 << 3)

extern uint16_t Firmware_Version;
extern uint16_t Firmware_UpdateFlag;
extern uint32_t Firmware_FlashAdress;
extern uint32_t Firmware_FlashByteNbr;

extern uint32_t downloaded_len;
extern uint32_t uploaded_len;

void canDispatch(Message *m);

int32_t host_IsDownloading();
int32_t host_IsUploading();
int32_t host_IsErrorOccur();
int32_t host_IsIdle();

void host_TrigBootloader(void);
void host_GetInfo(uint16_t nodeid, uint8_t option);
void host_SetInfo(uint16_t nodeid, uint8_t option, uint32_t buf);
void host_DownloadRequest(uint16_t nodeid, uint8_t *pbuf, uint32_t addr, uint32_t len);
void host_UploadRequst(uint16_t nodeid, uint8_t *pbuf, uint32_t addr, uint32_t len);
void host_EraseRequest(uint16_t nodeid, uint32_t addr, uint32_t len);
void host_Switch2App(uint16_t nodeid);
uint16_t host_GetAckStatus(uint16_t bit, int timeout);
void host_ClearAckStatus(uint16_t bit);
