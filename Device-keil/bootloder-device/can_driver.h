/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __can_driver_h__
#define __can_driver_h__

#include <stdint.h>

#ifdef CANOPEN_BIG_ENDIAN

/* Warning: the argument must not update pointers, e.g. *p++ */

#define UNS16_LE(v)  ((((UNS16)(v) & 0xff00) >> 8) | \
          (((UNS16)(v) & 0x00ff) << 8))

#define UNS32_LE(v)  ((((UNS32)(v) & 0xff000000) >> 24) | \
          (((UNS32)(v) & 0x00ff0000) >> 8)  | \
          (((UNS32)(v) & 0x0000ff00) << 8)  | \
          (((UNS32)(v) & 0x000000ff) << 24))

#else

#define UNS16_LE(v)  (v)

#define UNS32_LE(v)  (v)

#endif

#define MSG_PRINT(num, str, val)         \
        printSCI_nbr(num, ' ');          \
        printSCI_str(str);               \
        printSCI_nbr(val, '\n');

/* Command words */
#define  CMD_READ_INFO       0x01           
#define  CMD_WRITE_INFO      0x02           
#define  CMD_ERASE           0x03
#define  CMD_RUN             0x04

/* Frame tags */
#define  FRAME_CMD           0x00
#define  FRAME_ACK           0x01
#define  FRAME_UPLOADh       0x02
#define  FRAME_UPLOADd       0x03
#define  FRAME_DOWNLOADh     0x04
#define  FRAME_DOWNLOADd     0x05
#define  FRAME_PRINTF        0x06

/** 
 * @brief The CAN message structure 
 * @ingroup can
 */
typedef struct 
{
  uint16_t cob_id;  /**< message's ID */
  uint8_t  rtr;   /**< remote transmission request. (0 if not rtr message, 1 if rtr message) */
  uint8_t  len;   /**< message's length (0 to 8) */
  uint8_t  data[8]; /**< message's datas */
} Message;

#define Message_Initializer {(uint16_t)0,0,0,{0,0,0,0,0,0,0,0}}
#define Message_Trigger {0xff, 0, 8, {0x22, 0x5A, 0x58, 0x46, 0x57, 0x50, 0x4D, 0x4A}}

typedef uint8_t (*canSend_t)(Message *);

typedef void* CAN_HANDLE;

typedef void* CAN_PORT;

uint8_t canInit(CAN_PORT CANx);
uint8_t canSend(Message *m);	                
void printSCI_str(const char * str);
void printSCI_nbr(uint32_t nbr, uint8_t lastChar);



#endif /* __can_h__ */
