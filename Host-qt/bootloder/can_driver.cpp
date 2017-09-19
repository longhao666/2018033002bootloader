#include "PCANBasic.h"
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include "can_driver.h"
#include <QDebug>
#include "host.h"
#include <QQueue>

#define CAN_CHANNEL PCAN_USBBUS1

HANDLE hEvent = NULL;                      // PCAN收到消息的事件
QString canPrintMsg;
QQueue<QString> canPrintMsgQueue;

uint8_t newid;
uint8_t newidflag = 0;

void MSG_PRINT(char * str, uint8_t len)
{
    int i = 0;
    char str2[9];

    for (i = 0; i < len; i++){
        str2[i] = str[i];
    }
    str2[i] = 0;

    if (str[len-1] == '\n')
    {
        str2[len-1] = 0;
        canPrintMsg.append(str2);
        canPrintMsgQueue.enqueue(canPrintMsg);
        canPrintMsg.clear();
    }
    else
    {
        canPrintMsg.append(str2);
    }
}


void ReceivedNewid(uint8_t id)
{
    newid = id;
    newidflag = 1;

    while(newidflag == 1);
}

int isCanOK(void)
{
    return ((PCAN_ERROR_OK == CAN_GetStatus(CAN_CHANNEL))?1:0);
}

uint8_t canRead(void)
{
    Message m = Message_Initializer;
    TPCANMsg rxmsg;
    memset(&rxmsg, 0, sizeof(TPCANMsg));

    TPCANTimestamp timestamp;
    TPCANStatus result;

    /* Read the RX message */
    result = CAN_Read(CAN_CHANNEL, &rxmsg, &timestamp);
    if (result == PCAN_ERROR_OK) {
        m.cob_id = rxmsg.ID;
        m.rtr = rxmsg.MSGTYPE;
        m.len = rxmsg.LEN;
        memcpy(&m.data, &rxmsg.DATA, 8);
//        qDebug("ID 0x%3x LEN %d DATA", m.cob_id, m.len);
//        for (int i = 0; i < m.len; i ++){
//            qDebug(" 0x%2x", m.data[i]);
//        }
//        qDebug("\n");
        canDispatch(&m);
        return 1;
    }
    return 0;
}

unsigned int __stdcall ReadThread(void* arg) {
    while (true) {
        DWORD dReturn = WaitForSingleObject(hEvent, INFINITE);
        // 事件触发
        if ( WAIT_OBJECT_0 == dReturn) {
            uint8_t isOK = 1;
            while (isOK) { // 要一直读到队列空
                isOK = canRead();
            }
        }
    }
    return 0;
}


uint8_t canInit(CAN_PORT CANx)
{
    TPCANStatus result;

    result = CAN_Initialize(CAN_CHANNEL,PCAN_BAUD_1M,0,0,0);

    if(result == PCAN_ERROR_OK)
    {
        qDebug() << "canbus init ok";
    }
    else
    {
        qDebug() << "canbus init fail";
        return 0;
    }

    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    result = CAN_SetValue(CAN_CHANNEL, PCAN_RECEIVE_EVENT, &hEvent, sizeof(hEvent));
    // An error occurred, get a text describing the error
    if (result != PCAN_ERROR_OK) {
        char strMsg[256];
        CAN_GetErrorText(result, 0, strMsg);
        qDebug() << strMsg;
        //errorTxt = strMsg;
        return 0;
    }
    else{
        qDebug() << "SetValue OK";
    }

    // 开启新线程 //    WaitForSingleObject(handle, 5); // 等待5ms
//    HANDLE handle = (HANDLE)_beginthreadex(NULL, 0, ReadThread, NULL, 0, NULL);
//    if ( handle == 0 ) {
//        qDebug() << "Failed to create thread !";
//        return 0;
//    } else {
//        qDebug() << "thread_read init success";
//    }
    return 1;
}

uint8_t canSend(Message *m)
{
    TPCANMsg msg;
    DWORD result;

    msg.MSGTYPE = m->rtr;
    msg.ID = m->cob_id;
    msg.LEN = m->len;

    memcpy(&msg.DATA, m->data, 8);

    result = CAN_Write(CAN_CHANNEL, &msg);

    if (result == PCAN_ERROR_OK){
        return 1;
    }
    return 0;
}
