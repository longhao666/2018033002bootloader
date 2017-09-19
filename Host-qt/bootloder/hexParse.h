#ifndef HEX_PARSE_H
#define HEX_PARSE_H


#include <string.h>
#include <QFileDialog>
#include <QDataStream>
#include <QFileInfo>
#include <stdio.h>


class hexParse{
public:
    explicit hexParse(QFile *file);
    ~hexParse();

    QString data;
    uint32_t len;
    uint32_t addr;

    int parseHexFile();
private:
    QFile * file;
    int parseLine(QString * str_in, QString * str_out, uint32_t * linAddr);

};

#endif // HEX_PARSE_H
