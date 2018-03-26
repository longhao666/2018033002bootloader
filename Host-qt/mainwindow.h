#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <qtimer.h>
#include <QTableWidgetItem>
#include <QSettings>
#include "configure.h"
#include <QThread>
#include "can_driver.h"

namespace Ui {
class MainWindow;
}

class canRead : public QThread
{
    Q_OBJECT
public:
    canRead() {
        running = 0;
    }
    ~canRead() {}
    void stop_read();
    void start_read();
private:
    int running;
public slots:

protected:
    void run();
};



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer * ShowMsgTimer, * SearchNodeTimer;
    canRead canReadThread;

private slots:
    void ShowMsg();
    void SearchNode();

    void on_pBReadInfo_clicked();
    void on_pBUpload_clicked();
    void on_pBDownload_clicked();
    void on_pBOpenHex_clicked();
    void on_pBSaveHex_clicked();

    void on_pBClearPrintMsg_clicked();

    void on_pBSearchNode_clicked();

    void on_pBLoadCfg_clicked();

    void on_pBSaveCfg_clicked();

    void on_pBReadCfg_clicked();

    void on_pBWriteCfg_clicked();

    void on_tWConfig_cellChanged(int row, int column);

    void on_pBSwitch2App_clicked();

    void on_pBSetNodeid_clicked();

    void on_pBSetBoot_clicked();

private:

    bool startSearch;

    uint32_t configAddr;
    uint32_t configLen;

    char * configBuf;   //read from flash

    char * hexBuf;      //upload from flash
    uint32_t hexAddr;
    uint32_t hexSize;

    char * saveHexBuf;  //save to file
    uint32_t saveHexAddr;
    uint32_t saveHexSize;

    int hexSave(QFile *file);
};

#endif // MAINWINDOW_H
