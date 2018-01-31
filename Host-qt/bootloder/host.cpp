#include <string.h>
#include "host.h"
#include <qthread.h>
#include "watchdog.h"
#include  <QDebug>
#include <QTime>
#include <QCoreApplication>

uint16_t Firmware_Version;

uint16_t Firmware_UpdateFlag;
uint32_t Firmware_FlashAdress;
uint32_t Firmware_FlashByteNbr;

uint32_t download_addr = 0;
uint32_t download_len = 0;
uint32_t downloaded_len = 0;

uint32_t upload_addr = 0;
uint32_t upload_len = 0;
uint32_t uploaded_len = 0;

uint8_t * download_buf;
uint8_t * upload_buf;

uint16_t ack_status = 0x00;
uint16_t bootloader_status = STATUS_IDLE;

void rx_ack(Message *m);
void downloading(uint8_t cs, uint16_t nodeid);

void downloading_timeout(void);
void uploading_timeout(void);

watchDog *download_dog;
watchDog *upload_dog;

void host_TrigBootloader(void)
{
  Message msg = Message_Trigger;

  canSend(&msg);
}

void host_NodeInit()
{
  Firmware_Version = 0;
  Firmware_UpdateFlag = 0;
  Firmware_FlashAdress = 0;
  Firmware_FlashByteNbr = 0;

  download_addr = 0;
  download_len = 0;
  downloaded_len = 0;
  upload_addr= 0;
  upload_len = 0;
  uploaded_len = 0;

  download_buf = NULL;
  upload_buf = NULL;


  ack_status = 0;
}

int32_t host_IsDownloading()
{
  return (bootloader_status == STATUS_DOWNLOADING) ? 1:0;
}

int32_t host_IsUploading()
{
  return (bootloader_status == STATUS_UPLOADING) ? 1:0;
}

int32_t host_IsErrorOccur()
{
  return (bootloader_status == STATUS_ERROR) ? 1:0;
}

int32_t host_IsIdle()
{
  return (bootloader_status == STATUS_IDLE) ? 1:0;
}

uint16_t host_GetAckStatus(uint16_t bit, int timeout)
{
  QTime time;
  time.restart();
  while ((ack_status&bit) != bit)
  {
      QCoreApplication::processEvents();
      if (time.elapsed() == timeout)
        return 0;
  }
  return 1;
}

void host_ClearAckStatus(uint16_t bit)
{
  ack_status &= ~bit;
}

void host_GetInfo(uint16_t nodeid, uint8_t option)
{
  Message msg;
  msg.cob_id = ((uint16_t)FRAME_CMD << 7) + nodeid;
  msg.rtr = 0;
  msg.len = 2;
  msg.data[0] = CMD_READ_INFO;
  msg.data[1] = option; /* read all info */

  canSend(&msg);
}

void host_SetInfo(uint16_t nodeid, uint8_t option, uint32_t buf)
{
  Message msg;
  msg.cob_id = ((uint16_t)FRAME_CMD << 7) + nodeid;
  msg.rtr = 0;
  msg.len = 6;

  msg.data[0] = CMD_WRITE_INFO;
  msg.data[1] = option;

  memcpy(&msg.data[2], &buf, 4); 
  canSend(&msg);
}

void host_DownloadRequest(uint16_t nodeid, uint8_t *pbuf, uint32_t addr, uint32_t len)
{
  Message msg;
  
  download_buf = pbuf;
  downloaded_len = 0;
  download_len = len;

  msg.cob_id = ((uint16_t)FRAME_DOWNLOADh << 7) + nodeid;
  msg.rtr = 0;
  msg.len = 8;

  msg.data[0] =  0x21;
  msg.data[1] = (uint8_t)(addr >> 0);
  msg.data[2] = (uint8_t)(addr >> 8);
  msg.data[3] = (uint8_t)(addr >> 16);
  msg.data[4] = (uint8_t)(addr >> 24);

  msg.data[5] = (uint8_t)(len >> 0);
  msg.data[6] = (uint8_t)(len >> 8);
  msg.data[7] = (uint8_t)(len >> 16);
  
  canSend(&msg);
}

void host_UploadRequst(uint16_t nodeid, uint8_t *pbuf, uint32_t addr, uint32_t len)
{
  Message msg;

  upload_buf = pbuf;
  uploaded_len = 0;
  upload_len = len;

  msg.cob_id = ((uint16_t)FRAME_UPLOADh << 7) + nodeid;
  msg.rtr = 0;
  msg.len = 8;

  msg.data[0] =  0x40;
  msg.data[1] = (uint8_t)(addr >> 0);
  msg.data[2] = (uint8_t)(addr >> 8);
  msg.data[3] = (uint8_t)(addr >> 16);
  msg.data[4] = (uint8_t)(addr >> 24);

  msg.data[5] = (uint8_t)(len >> 0);
  msg.data[6] = (uint8_t)(len >> 8);
  msg.data[7] = (uint8_t)(len >> 16);
  
  canSend(&msg);
}

void host_EraseRequest(uint16_t nodeid, uint32_t addr, uint32_t len)
{
  Message msg;

  msg.cob_id = ((uint16_t)FRAME_CMD << 7) + nodeid;
  msg.rtr = 0;
  msg.len = 8;

  msg.data[0] =  CMD_ERASE;
  msg.data[1] = (uint8_t)(addr >> 0);
  msg.data[2] = (uint8_t)(addr >> 8);
  msg.data[3] = (uint8_t)(addr >> 16);
  msg.data[4] = (uint8_t)(addr >> 24);

  msg.data[5] = (uint8_t)(len >> 0);
  msg.data[6] = (uint8_t)(len >> 8);
  msg.data[7] = (uint8_t)(len >> 16);
  
  canSend(&msg);
}

void host_Switch2App(uint16_t nodeid)
{
  Message msg;

  msg.cob_id = ((uint16_t)FRAME_CMD << 7) + nodeid;
  msg.rtr = 0;
  msg.len = 1;

  msg.data[0] =  CMD_RUN;

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
  uint16_t nodeid = cob_id&0x7f;

  if (cob_id == 0x80)
  {
    ReceivedNewid(m->data[0]);
    return;
  }

  switch(cob_id >> 7)
  {
    case FRAME_ACK:
      rx_ack(m);
      break;

    case FRAME_DOWNLOADd:
      if (bootloader_status == STATUS_IDLE){
          download_dog = new watchDog(100,(dog_barking_fun)downloading_timeout);
          bootloader_status = STATUS_DOWNLOADING;
      }
      if ((m->data[0] == 0x60) || (m->data[0] == 0x30))
      {
        downloading(0x00, nodeid);
      }
      else if (m->data[0] == 0x20)
      {
        downloading(0x10, nodeid);
      }
      break;
    case FRAME_UPLOADd:
      if (bootloader_status == STATUS_IDLE){
          upload_dog = new watchDog(100,(dog_barking_fun)uploading_timeout);
          bootloader_status = STATUS_UPLOADING;
      }
      else{
          upload_dog->feed_dog();
      }

      if (m->data[0] == 0x41)
      {
        msg.cob_id = ((uint16_t)FRAME_UPLOADh << 7) + nodeid;
        msg.len = 8;
        msg.data[0] = 0x60;

        canSend(&msg);
      }
      else if ((m->data[0] == 0x00) || (m->data[0] == 0x10))
      {
        memcpy(&upload_buf[uploaded_len], &m->data[1], 7);
        uploaded_len += 7;

        msg.cob_id = ((uint16_t)FRAME_UPLOADh << 7) + nodeid;
        msg.len = 8;
        msg.data[0] = 0x70 - m->data[0];

        canSend(&msg);
      }
      else
      {
        uint8_t x = 9 - (((m->data[0] & 0x0F) + 1) >> 1);
        memcpy(&upload_buf[uploaded_len], &m->data[1], x);
        uploaded_len += x;
        /* upload complete */
        ack_status |= ACK_UPLOAD_BIT;
        delete upload_dog; //kill dog
        bootloader_status = STATUS_IDLE;
      }
      break;
    case FRAME_PRINTF:
      MSG_PRINT((char*)m->data,m->len);
      break;

    default:
      break;
  }
}


/* HOST download program to DEVICE */
void downloading(uint8_t cs, uint16_t nodeid)
{
  Message msg = Message_Initializer;

  download_dog->feed_dog();

  msg.cob_id = ((uint16_t)FRAME_DOWNLOADh << 7) + nodeid;
  msg.rtr = 0;
  msg.len = 8;
  msg.data[0] = cs;

  if (download_len == downloaded_len)
  {
    /* download complete */
    ack_status |= ACK_DOWNLOAD_BIT;
    delete download_dog; //kill dog
    bootloader_status = STATUS_IDLE;
    return;
    
  }
  if ((download_len - downloaded_len) <= 7)
  {
    msg.data[0] += ((9 - (download_len - downloaded_len))<<1) - 1;

    for (uint32_t i = 0; i < download_len - downloaded_len; i++)
      msg.data[i + 1] = *(uint8_t*)(download_buf + downloaded_len + i);
    downloaded_len = download_len;
  }
  else
  {
    for (uint32_t i = 0; i < 7; i++)
      msg.data[i + 1] = *(uint8_t*)(download_buf + downloaded_len + i);
    downloaded_len += 7;
  }

  canSend(&msg);
}

void rx_ack(Message *m)
{
  uint8_t cmdword = m->data[0];

  switch (cmdword)
  {
    case CMD_READ_INFO:
      ack_status |= ACK_READ_BIT;
      switch (m->data[1]) /* option */
      {
        case INFO_FW_VERSION:
          memcpy(&Firmware_Version, &m->data[2], 2);
          break;
        case INFO_FW_FLAG:
          memcpy(&Firmware_UpdateFlag, &m->data[2], 2);
          break;
        case INFO_FW_ADDRESS:
          memcpy(&Firmware_FlashAdress, &m->data[2], 4);
          break;
        case INFO_FW_BYTE_NBR:
          memcpy(&Firmware_FlashByteNbr, &m->data[2], 4);
          break;
      }
      break;
    case CMD_WRITE_INFO:
      if (m->data[2] == 1)
      {
        ack_status |= ACK_WRITE_BIT;
      }
      else
      {
        ack_status &= ~ACK_WRITE_BIT;
      }
      break;
    case CMD_ERASE:
      if (m->data[1] == 1)
      {
        ack_status |= ACK_ERASE_BIT;
      }
      else
      {
        ack_status &= ~ACK_ERASE_BIT;
      }
      break;
    case CMD_RUN:
      if (m->data[1] == 0)
      {
        ack_status &= ACK_RUN_BIT;
      }
      break;
    default:
      break;
  }
}

void downloading_timeout(void)
{
    bootloader_status = STATUS_ERROR;
    qDebug() << "downloading timeout";
}

void uploading_timeout(void)
{
    bootloader_status = STATUS_ERROR;
    qDebug() << "uploading timeout";
}

