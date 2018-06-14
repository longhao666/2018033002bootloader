#include "mainwindow.h"
#include <QApplication>
#include <QTextcodec>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QTextCodec::setCodecForTr(QTextCodec::codecForName("GB2312"));//tr()函数支持中文字符
//    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("GB2312"));//""直接支持中文字符

//    QTranslator trans;
//    trans.load(":/qt_zh_CN");
//    a.installTranslator(&trans);
     QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));

    MainWindow w;
    w.show();

    return a.exec();
}
