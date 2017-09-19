#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "host.h"
#include "qfileinfo.h"
#include "hexParse.h"
#include <QDebug>
#include <stdio.h>
#include <QQueue>
#include <QSettings>
#include <QThread>
#include <QHeaderView>

extern QQueue<QString> canPrintMsgQueue;

extern uint8_t newid;
extern uint8_t newidflag;

void canRead :: run(){
    while(running){
        ReadThread(NULL);
    }
    exec();
}

void canRead :: stop_read(){
    running = 0;
}

void canRead :: start_read(){
    running = 1;
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    canInit(0);
    canReadThread.start();
    canReadThread.start_read();

    startSearch = false;

    configAddr = DEF_CONFIG_ADDR;
    configLen =  DEF_CONFIG_LEN;
    configBuf = (char*)malloc(configLen);

    ShowMsgTimer = new QTimer(this);
    SearchNodeTimer = new QTimer(this);

    connect(ShowMsgTimer, SIGNAL(timeout()), this, SLOT(ShowMsg()));
    connect(SearchNodeTimer, SIGNAL(timeout()), this, SLOT(SearchNode()));
    ShowMsgTimer->start(100);
    SearchNodeTimer->start(100);

    ui->pBSaveHex->setEnabled(false);
    ui->pBDownload->setEnabled(false);


    ui->tWConfig->setColumnCount(5);

    QStringList  headers;
    headers<<"Index"<<"Name"<<"Value"<<"Type"<<"Comment";
    ui->tWConfig->setHorizontalHeaderLabels(headers);

    ui->tWConfig->setColumnWidth(0, 40);
    ui->tWConfig->setColumnWidth(1, 150);
    ui->tWConfig->setColumnWidth(2, 100);
    ui->tWConfig->setColumnWidth(3, 40);
    ui->tWConfig->setColumnWidth(4, 150);
    //ui->tWConfig->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QCoreApplication::setOrganizationName("Aubo Robotics");
    QCoreApplication::setOrganizationDomain("aubo-robotics.cn");
    QCoreApplication::setApplicationName("Star Runner");
    //QHeaderView *headerView = ui->tWConfig->verticalHeader();
    //headerView->setHidden(true);
}

MainWindow::~MainWindow()
{
    canReadThread.stop_read();
    canReadThread.quit();
    if (!canReadThread.wait(3000)){
        canReadThread.terminate();
        canReadThread.wait();
    }
    delete ui;
}

void MainWindow::ShowMsg()
{
    if (!canPrintMsgQueue.isEmpty()){
        ui->tEPrintMsg->append(canPrintMsgQueue.dequeue());
    }

    if (host_IsDownloading()){
        ui->progressBar->setValue(downloaded_len*100/hexSize);
    }

    else if (host_IsUploading()){
        ui->progressBar->setValue(uploaded_len*100/saveHexSize);
    }
    else{
        ui->progressBar->setValue(0);
    }
}

void MainWindow :: SearchNode()
{
    if (startSearch == true){
        host_TrigBootloader();
    }
    if(newidflag == 1){
        newidflag = 0;
        if (-1 == ui->cBNodeid->findText(QString::number(newid))){
            ui->cBNodeid->addItem(QString::number(newid));
        }
    }
}


void MainWindow::on_pBReadInfo_clicked()
{
    bool ok;
    uint16_t id = ui->cBNodeid->currentText().toInt(&ok);

    if (ok == false){
        ui->tEPrintMsg->append("No valid id");
        return;
    }

    uint8_t option = (uint8_t)0x01 << (ui->cBInfoSelect->currentIndex());
    host_GetInfo(id, option);

    if (host_GetAckStatus(ACK_READ_BIT, 1000) != 1){
        ui->tEPrintMsg->append("Read info failed");
        return;
    }
    else{
        ui->tEPrintMsg->append("Read info succeeded");
    }

    host_ClearAckStatus(ACK_READ_BIT);
}

void MainWindow::on_pBUpload_clicked()
{
    bool IsSearching = startSearch;
    startSearch = false;
    ui->pBSearchNode->setText("Stopped");

    while(!isCanOK()){
        QCoreApplication::processEvents();
    }

    bool ok;
    uint16_t id = ui->cBNodeid->currentText().toInt(&ok);

    if (ok == false){
        ui->tEPrintMsg->append("No valid id");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }

    host_ClearAckStatus(ACK_READ_BIT);
    host_GetInfo(id, INFO_FW_FLAG);
    if (host_GetAckStatus(ACK_READ_BIT,100) != 1) {
        ui->tEPrintMsg->append("Uploading >> Read flag failed");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }
    else{
        ui->tEPrintMsg->append("Uploading >> Read flag succeeded");
    }
    host_ClearAckStatus(ACK_READ_BIT);

    if (Firmware_UpdateFlag != 1){
        ui->tEPrintMsg->append("Uploading >> No valid firmware (flag != 1)");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }

    host_ClearAckStatus(ACK_READ_BIT);
    host_GetInfo(id, INFO_FW_VERSION);
    if (host_GetAckStatus(ACK_READ_BIT, 100) != 1){
        ui->tEPrintMsg->append("Uploading >> Read firmware version failed");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }
    else{
        ui->tEPrintMsg->append("Uploading >> Read firmware version succeeded");
    }
    host_ClearAckStatus(ACK_READ_BIT);

    host_ClearAckStatus(ACK_READ_BIT);
    host_GetInfo(id, INFO_FW_ADDRESS);
    if (host_GetAckStatus(ACK_READ_BIT, 100) != 1){
        ui->tEPrintMsg->append("Uploading >> Read firmware address failed");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }
    else{
        ui->tEPrintMsg->append("Uploading >> Read firmware address succeeded");
    }
    host_ClearAckStatus(ACK_READ_BIT);

    host_ClearAckStatus(ACK_READ_BIT);
    host_GetInfo(id, INFO_FW_BYTE_NBR);
    if (host_GetAckStatus(ACK_READ_BIT, 100) != 1){
        ui->tEPrintMsg->append("Uploading >> Read firmware bytes number failed");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }
    else{
        ui->tEPrintMsg->append("Uploading >> Read firmware bytes number succeeded");
    }
    host_ClearAckStatus(ACK_READ_BIT);

    this->saveHexBuf = (char*)malloc(Firmware_FlashByteNbr);

    this->saveHexAddr = Firmware_FlashAdress;

    this->saveHexSize = Firmware_FlashByteNbr;

    host_UploadRequst(id, (uint8_t *)this->saveHexBuf, saveHexAddr, saveHexSize);

    while (host_IsIdle()){
        QCoreApplication::processEvents();
    }
    while (host_IsUploading()){
        QCoreApplication::processEvents();
    }

//    if (host_GetAckStatus(ACK_UPLOAD_BIT, 5000) != 1){
//        ui->pBSaveHex->setEnabled(false);
//        ui->tEPrintMsg->append("Upload firmware failed");
//        return;
//    }
//    else{
        ui->tEPrintMsg->append("Upload firmware succeeded");
        ui->pBSaveHex->setEnabled(true);
//    }
//    host_ClearAckStatus(ACK_UPLOAD_BIT);
    if (IsSearching){
        startSearch = true;
        ui->pBSearchNode->setText("Searching");
    }
    return;
}

void MainWindow::on_pBDownload_clicked()
{
    bool IsSearching = startSearch;
    startSearch = false;
    ui->pBSearchNode->setText("Stopped");

    while(!isCanOK()){
        QCoreApplication::processEvents();
    }

    bool ok;
    uint16_t id = ui->cBNodeid->currentText().toInt(&ok);

    if (ok == false){
        ui->tEPrintMsg->append("No valid id");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }

    uint32_t addr = this->hexAddr;
    uint8_t *buf = (uint8_t*)this->hexBuf;
    uint32_t len = this->hexSize;

    host_ClearAckStatus(ACK_WRITE_BIT);
    host_SetInfo(id, INFO_FW_FLAG, 0);
    if (host_GetAckStatus(ACK_WRITE_BIT, 250) != 1){
        ui->tEPrintMsg->append("Write info flag failed");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }
    else{
        ui->tEPrintMsg->append("Write info flag succeeded");
    }
    host_ClearAckStatus(ACK_WRITE_BIT);

    host_ClearAckStatus(ACK_WRITE_BIT);
    host_SetInfo(id, INFO_FW_ADDRESS, addr);
    if (host_GetAckStatus(ACK_WRITE_BIT, 250) != 1){
        ui->tEPrintMsg->append("Write info addr failed");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }
    else{
        ui->tEPrintMsg->append("Write info addr succeeded");
    }
    host_ClearAckStatus(ACK_WRITE_BIT);

    host_ClearAckStatus(ACK_WRITE_BIT);
    host_SetInfo(id, INFO_FW_BYTE_NBR, len);
    if (host_GetAckStatus(ACK_WRITE_BIT, 250) != 1){
        ui->tEPrintMsg->append("Write info len failed");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }
    else{
        ui->tEPrintMsg->append("Write info len succeeded");
    }
    host_ClearAckStatus(ACK_WRITE_BIT);

    host_ClearAckStatus(ACK_ERASE_BIT);
    host_EraseRequest(id, addr, len);
    if (host_GetAckStatus(ACK_ERASE_BIT, 750) != 1){
        ui->tEPrintMsg->append("Erase flash failed");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }
    else{
        ui->tEPrintMsg->append("Erase flash succeeded");
    }
    host_ClearAckStatus(ACK_ERASE_BIT);

    host_DownloadRequest(id, buf, addr, len);

    while (host_IsIdle()){
        QCoreApplication::processEvents();
    }
    while (host_IsDownloading()){
        QCoreApplication::processEvents();
    }

//    if (host_GetAckStatus(ACK_DOWNLOAD_BIT, 5000) != 1){
//        ui->pBSaveHex->setEnabled(false);
//        ui->tEPrintMsg->append("Download firmware failed");
//        return;
//    }
//    else{
//        ui->tEPrintMsg->append("Download firmware succeeded");
//        ui->pBSaveHex->setEnabled(true);
//    }
//    host_ClearAckStatus(ACK_DOWNLOAD_BIT);

    host_ClearAckStatus(ACK_WRITE_BIT);
    host_SetInfo(id, INFO_FW_FLAG, 1);
    if (host_GetAckStatus(ACK_WRITE_BIT, 250) != 1){
        ui->tEPrintMsg->append("Write flag 1 failed");
        if (IsSearching){
            startSearch = true;
            ui->pBSearchNode->setText("Searching");
        }
        return;
    }
    else{
        ui->tEPrintMsg->append("Write flag 1 succeeded");
    }
    host_ClearAckStatus(ACK_WRITE_BIT);
    if (IsSearching){
        startSearch = true;
        ui->pBSearchNode->setText("Searching");
    }
    return;
}

void MainWindow::on_pBOpenHex_clicked()
{
    QString hexPath = QFileDialog::getOpenFileName (this, tr("Open File"),
                                                 "", tr("文件 (*.hex)"));
    ui->lEHexPath->setText(hexPath);

    QFile f(hexPath);

    hexParse h(&f);
    if (h.parseHexFile() != 0){
        ui->tEPrintMsg->append("Hex file parse failed");
        return;
    }

    ui->tEPrintMsg->append("Hex file parse succeeded");

    QByteArray ba = (h.data).toLatin1();

    this->hexAddr = h.addr;
    this->hexSize = h.len;
    this->hexBuf = (char*)malloc(this->hexSize);
    memcpy((void*)this->hexBuf, (void*)ba.data(), this->hexSize);

    ui->pBDownload->setEnabled(true);
}

void MainWindow::on_pBSaveHex_clicked()
{
    QString saveHexPath = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                 "untitled.hex", tr("文件 (*.hex)"));
    ui->lESaveHexPath->setText(saveHexPath);

    QFile f(saveHexPath);

    hexSave(&f);
}

int MainWindow::hexSave(QFile *file)
{
    uint32_t i;
    uint8_t check_sum = 0;

    if (!file->open(QIODevice::ReadWrite)){
        return (-1);
    }
    file->resize(0); /* clear context */

    QTextStream out(file);

    out.reset();

    out.setFieldAlignment(QTextStream::AlignRight);
    out.setPadChar('0');
    out.setIntegerBase(16);
    out.setNumberFlags(QTextStream::UppercaseDigits);
    out.setFieldWidth(2);

    out << ":02000004";
    out << (uint8_t)(saveHexAddr >> 24);
    out << (uint8_t)(saveHexAddr >> 16);
    check_sum = 0x02 + 0x04 + (uint8_t)(saveHexAddr >> 24) + (uint8_t)(saveHexAddr >> 16);
    check_sum = ~check_sum + 1;
    out << (uint8_t)check_sum;

    out.setFieldWidth(1);
    out << "\n";
    out.setFieldWidth(2);

    uint16_t addrOffset = (uint16_t)saveHexAddr;
    int16_t addrMSB = (uint16_t)(saveHexAddr>>16);
    uint32_t leftLen = saveHexSize;

    while (leftLen != 0){
        uint8_t line_len = 0x10, line_type = 0x00;
        if (leftLen < 0x10){
            line_len = leftLen;
        }
        out.setFieldWidth(1);
        out << ":";
        out.setFieldWidth(2);

        out << line_len << (uint8_t)(addrOffset>>8) << (uint8_t)addrOffset << line_type;
        check_sum = line_len + (addrOffset>>8) + addrOffset + line_type;
        for (i = 0; i < line_len; i++){
            uint8_t val = saveHexBuf[saveHexSize - leftLen--];
            out << val;
            check_sum += val;
        }
        check_sum = ~check_sum + 1;
        out << (uint8_t)check_sum;

        out.setFieldWidth(1);
        out << "\n";
        out.setFieldWidth(2);

        addrOffset += line_len;

        if (addrOffset == 0x0000){
            addrMSB += 1;
            out << ":02000004";
            out << (uint8_t)(addrMSB >> 8);
            out << (uint8_t)(addrMSB >> 0);
            check_sum = 0x02 + 0x04 + (uint8_t)(addrMSB >> 8) + (uint8_t)(addrMSB >> 0);
            check_sum = ~check_sum + 1;
            out << (uint8_t)check_sum;

            out.setFieldWidth(1);
            out << "\n";
            out.setFieldWidth(2);
        }
    }

    uint8_t line_len = 4, line_type = 5;
    uint32_t linearAddr = saveHexAddr + 0x189;

    out.setFieldWidth(1);
    out << ":";
    out.setFieldWidth(2);

    out << line_len << 0 << 0 << line_type;
    check_sum = line_len + line_type;
    for (i = 0; i < line_len; i++){
        uint8_t val = linearAddr >> (24 - 8*i);
        out << val;
        check_sum += val;
    }
    check_sum = ~check_sum + 1;
    out << (uint8_t)check_sum;

    out.setFieldWidth(1);
    out << "\n";

    out << ":00000001FF"<<"\n";
    file->close();
    return 1;
}

void MainWindow::on_pBClearPrintMsg_clicked()
{
    ui->tEPrintMsg->clear();
}

void MainWindow::on_pBSearchNode_clicked()
{
    if (startSearch == true){
        startSearch = false;
        ui->pBSearchNode->setText("Stopped");
    }
    else{
        startSearch = true;
        ui->pBSearchNode->setText("Searching");
    }
}

void MainWindow::on_pBLoadCfg_clicked()
{
    bool ok;
    QString cfgPath = QFileDialog::getOpenFileName (this, tr("Open File"),
                                                 "", tr("文件 (*.ini)"));
    if (cfgPath.isEmpty()){
        return;
    }
    QSettings * configure = new QSettings(cfgPath, QSettings::IniFormat);

    int temp1 = configure->value("Configure/addr").toString().toInt(&ok, 16);
    if (ok){
        configAddr = temp1;
    }
    int temp2 =  configure->value("Configure/len").toString().toInt(&ok, 16);
    if (ok){
        if (temp2 != (int)configLen){
            configLen = temp2;
            free(configBuf);
            configBuf = (char*)malloc(configLen);
        }
    }

    if (configure != NULL){
        int rowCnt = configure->value("Info/rows").toString().toInt(&ok);
        ui->tWConfig->setRowCount(rowCnt);
        for (int rowIndex = 0; rowIndex < rowCnt; rowIndex++){
            ui->tWConfig->setRowHeight(rowIndex, 24);//设置行的高度
            QString sector("row");
            sector.append(QString::number(rowIndex));

            configure->beginGroup(sector);

            QString index("index");
            QTableWidgetItem *item0 = new QTableWidgetItem(configure->value(index,"-").toString());
            ui->tWConfig->setItem(rowIndex, 0, item0);

            QString name("name");
            QTableWidgetItem *item1 = new QTableWidgetItem(configure->value(name,"-").toString());
            ui->tWConfig->setItem(rowIndex, 1, item1);

            QString type("type");

            QTableWidgetItem *item3 = new QTableWidgetItem(configure->value(type,"0").toString());
            ui->tWConfig->setItem(rowIndex, 3, item3);

            QString comment("comment");
            QTableWidgetItem *item4 = new QTableWidgetItem(configure->value(comment,"-").toString());
            ui->tWConfig->setItem(rowIndex, 4, item4);

            QString value("value");
            QTableWidgetItem *item2 = new QTableWidgetItem(configure->value(value,"-").toString());
            ui->tWConfig->setItem(rowIndex, 2, item2);

            configure->endGroup();
        }//end of for
    }//end of if
}

void MainWindow::on_pBSaveCfg_clicked()
{
    QString saveCfgPath = QFileDialog::getSaveFileName(this, tr("Save File"),
                                                 "untitled.ini", tr("文件 (*.ini)"));
    QSettings *configure = new QSettings(saveCfgPath, QSettings::IniFormat);
    configure->clear();

    int rowCnt = ui->tWConfig->rowCount();
    configure->setValue("Info/rows",QString::number(rowCnt));
    configure->setValue("Configure/addr",QString::number(configAddr,16));
    configure->setValue("Configure/len",QString::number(configLen,16));

    for (int rowIndex = 0; rowIndex < rowCnt; rowIndex++){
        QString sector("row");
        sector.append(QString::number(rowIndex));

        configure->beginGroup(sector);

        configure->setValue("index", ui->tWConfig->item(rowIndex,0)->text());
        configure->setValue("name", ui->tWConfig->item(rowIndex,1)->text());
        configure->setValue("value", ui->tWConfig->item(rowIndex,2)->text());
        configure->setValue("type", ui->tWConfig->item(rowIndex,3)->text());
        configure->setValue("comment", ui->tWConfig->item(rowIndex,4)->text());

        configure->endGroup();
    }

    configure->sync();
}

void MainWindow::on_pBReadCfg_clicked()
{
    bool ok;
    uint16_t id = ui->cBNodeid->currentText().toInt(&ok);

    if (ok == false){
        ui->tEPrintMsg->append("No valid id");
        return;
    }

    host_UploadRequst(id, (uint8_t *)configBuf, configAddr, configLen);
    if (host_GetAckStatus(ACK_UPLOAD_BIT, 5000) != 1){
        ui->tEPrintMsg->append("Reading configure failed");
        return;
    }
    else{
        ui->tEPrintMsg->append("Reading configure succeeded");
    }
    host_ClearAckStatus(ACK_UPLOAD_BIT);

    //update table
    int rowCnt = ui->tWConfig->rowCount();
    int len = 0;

    for (int i = 0; i < rowCnt; i++){
        int type = ui->tWConfig->item(i, 3)->text().toInt(&ok,16);
        if (ok == false){
            continue;
        }
        int index = ui->tWConfig->item(i, 0)->text().toInt(&ok,16); // index or type is not valid
        if (ok == false){
            continue;
        }

        uint32_t value = 0;

        memcpy((void*)&value, (void*)&configBuf[index*2], type); // index is a 16bit array index

        QTableWidgetItem *item = new QTableWidgetItem(QString::number(value));

        ui->tWConfig->setItem(i, 2, item);
        len += type;
    }

    if (len < (int)(this->configLen)){
        ui->tEPrintMsg->append("Stored configure don't match loaded ini file");
    }
    return;
}

void MainWindow::on_pBWriteCfg_clicked()
{
    bool ok;
    uint16_t id = ui->cBNodeid->currentText().toInt(&ok);
    if (ok == false){
        ui->tEPrintMsg->append("No valid id");
        return;
    }

    if (!configBuf){
        ui->tEPrintMsg->append("Please load config firstly");
        return;
    }

    host_EraseRequest(id, configAddr, configLen);
    if (host_GetAckStatus(ACK_ERASE_BIT, 500) != 1){
        ui->tEPrintMsg->append("Writing configure >> Erase flash failed");
        return;
    }
    else{
        ui->tEPrintMsg->append("Writing configure >> Erase flash succeeded");
    }
    host_ClearAckStatus(ACK_ERASE_BIT);

    host_DownloadRequest(id, (uint8_t*)configBuf, configAddr, configLen);
    if (host_GetAckStatus(ACK_DOWNLOAD_BIT, 1000) != 1){
        ui->tEPrintMsg->append("Writing configure >> failed");
        return;
    }
    else{
        ui->tEPrintMsg->append("Writing configure >> succeeded");
    }
    host_ClearAckStatus(ACK_DOWNLOAD_BIT);
}

void MainWindow::on_tWConfig_cellChanged(int row, int column)
{
    bool ok;
    if (column == 2){
        int index = ui->tWConfig->item(row, 0)->text().toInt(&ok, 16);
        if (!ok){
            return;
        }
        int type = ui->tWConfig->item(row,3)->text().toInt(&ok);
        if (!ok){
            return;
        }
        int value = ui->tWConfig->item(row, 2)->text().toInt(&ok);
        if (!ok){
            return;
        }
        memcpy((void*)(configBuf+2*index), (void*)&value, type);
    }
}

void MainWindow::on_pBSwitch2App_clicked()
{
    bool ok;
    uint16_t id = ui->cBNodeid->currentText().toInt(&ok);

    if (ok == false){
        ui->tEPrintMsg->append("No valid id");
        return;
    }
    host_Switch2App(id);
}
