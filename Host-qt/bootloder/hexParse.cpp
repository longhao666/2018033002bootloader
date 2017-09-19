#include <QDebug>
#include "hexParse.h"
#include <stdio.h>

hexParse :: hexParse(QFile *file)
{
    this->file = file;
    this->addr = 0;
    this->len = 0;

}

hexParse :: ~hexParse()
{
}

int hexParse :: parseLine(QString * str_in, QString  * str_out, uint32_t *linAddr)
{
    unsigned int nbytes = 0, type = 0, i, val, check_sum_read;
    unsigned char check_sum;  

    QByteArray ba = str_in->toLatin1();

    char *s = ba.data();

    if(*s != ':') //第一个为冒号
    {    
        return (1);
    }  
    ++ s;

    //接下来的8个字节为数据大小、地址等
    if(sscanf(s, "%02x%04x%02x", &nbytes, linAddr, &type) != 3)
    {    
        return (2);
    }  
    s += 8; 

    //读到的类型  
    if(type == 0) //为数据段  
    {  
        if(!(nbytes >= 0 && nbytes < 1024))  
        {  
            return (3);
        }  
          
        check_sum = nbytes + *linAddr + (*linAddr>>8) + type;
        //  
        for(i = 0; i < nbytes; i++)  
        {  
            val = 0;  
            if(sscanf(s, "%02x", &val) != 1)  
            {    
                return (4);
            }  
            s += 2;
            check_sum += val;

            str_out->append(val);
        }  
        this->len += nbytes;

        //  
        check_sum_read = 0;

        if(sscanf(s, "%02x", &check_sum_read) != 1)  
        {    
            return (5);
        }
        if((check_sum + check_sum_read) & 0xff)  
        {    
            return (6);
        }  
    }
    else if (type == 4) //start address
    {
        int  startAddr = 0;

        if(nbytes != 2)
        {
            return (3);
        }

        check_sum = nbytes + *linAddr + (*linAddr>>8) + type;

        //
        for(i = 0; i < 2; i++)
        {
            val = 0;
            if(sscanf(s, "%02x", &val) != 1)
            {
                return (4);
            }
            s += 2;
            check_sum += val;

            startAddr <<= 8;
            startAddr |= val;
        }
        //
        check_sum_read = 0;

        if(sscanf(s, "%02x", &check_sum_read) != 1)
        {
            return (5);
        }
        if((check_sum + check_sum_read) & 0xff)
        {
            return (6);
        }
        if (startAddr)
        this->addr = startAddr;
    }
    else if(type == 5)  //Start Linear Address Record
    {
        return (0);
    }
    else if(type == 1)  //end of hex file
    {
        return (0);
    }
    else
    {  
        return (7);
    }  
    return (0);
}

int hexParse::parseHexFile()
{
    int lineIndex = 1;
    uint32_t lineAddr;

    if(!this->file->open(QIODevice::ReadOnly))
    {
        return (-1);  //file cannot be opened
    }
    QTextStream in(this->file);

    while(!in.atEnd())
    {
        QString lineText = in.readLine(); //读取一行

        if (this->parseLine(&lineText, &this->data, &lineAddr) != 0){
            qDebug("format violation at line %d\n", lineIndex);
            return lineIndex;
        }
        if (lineIndex == 2){
            this->addr = (this->addr << 16) + lineAddr;
        }
        if (lineAddr == 0xFFF0){
            in.readLine();
        }

        lineIndex++;
    }
    this->file->close();

    return 0;
}

