#include "ymodem.h"
#include "serialmanager.h"
#include "serialwidget.h"
#include "serialprotocoltransmission.h"
#include <QThread>
#include <QDebug>
#include <iostream>
#include <cstdint>
uint8_t recvData[10];
uint8_t sendData[1030];

SerialProtocolTransmission::SerialProtocolTransmission(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnections();
    serialWidget = (SerialWidget*)parentWidget();
    ymodem = new Ymodem();

    //connect(this, &SerialProtocolTransmission::protcocolRecvData, ymodem, &Ymodem::ymodemRecvData);
}

SerialProtocolTransmission::~SerialProtocolTransmission()
{
    protocolThread.quit();
    protocolThread.wait();
}

void SerialProtocolTransmission::initUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 文件选择部分
    QHBoxLayout *fileLayout = new QHBoxLayout;
    openFileButton = new QPushButton(tr("Open File"), this);
    fileLabel = new QLabel(tr("No file selected"), this);
    fileLayout->addWidget(openFileButton);
    fileLayout->addWidget(fileLabel);

    // 进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    // 开始传输按钮
    startButton = new QPushButton(tr("Start YModem Transfer"), this);
    startButton->setEnabled(false); // 初始禁用

    // 添加到主布局
    mainLayout->addLayout(fileLayout);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(startButton);
}

void SerialProtocolTransmission::protocolEnableUI(bool flag)
{
    if(flag)
    {
        openFileButton->setEnabled(true);
    }
    else
    {
        openFileButton->setEnabled(false);
        startButton->setEnabled(false);
        progressBar->setRange(0, 100);
        progressBar->setValue(0);
        fileLabel->setText(tr("No file selected"));
        filePath.clear();
        ymodem->YmodemSendFileInterrupt();
        disconnect(ymodem, &Ymodem::ymodemSendData, this, &SerialProtocolTransmission::protcocolSendData);
        disconnect(this, &SerialProtocolTransmission::startTrasmit, ymodem, &Ymodem::YmodemSendFileStart);
    }
}

void SerialProtocolTransmission::initConnections()
{
    //connect(openFileButton, &QPushButton::clicked, this, &SerialProtocolTransmission::openFile);
    connect(startButton, &QPushButton::clicked, this, &SerialProtocolTransmission::startYModemTransfer);
    //bug
    //connect(serialManager, &SerialManager::dataReceived, this, &SerialProtocolTransmission::protocolHexReceiveData);
 }

void SerialProtocolTransmission::openFile()
{
    filePath = QFileDialog::getOpenFileName(this, tr("Select File"), "", tr("All Files (*)"));
    if (!filePath.isEmpty()) {
        fileLabel->setText(filePath);
        startButton->setEnabled(true); // 启用开始按钮
    } else {
        fileLabel->setText(tr("No file selected"));
        startButton->setEnabled(false); // 禁用开始按钮
    }
    serialWidget->enableUi(false);
}

void SerialProtocolTransmission::protocolHexSendData(const QByteArray &data)
{
    serialWidget->serialPortManager->sendData(data);
    serialWidget->sentBytes += data.size();
    serialWidget->updateStatusLabel();
    serialWidget->logMessage("protocolSent: " + data.toHex(' ').toUpper());
}

void SerialProtocolTransmission::protocolHexReceiveData(const QByteArray &data)
{
    memcpy(recvData, data.data(), data.size());
}

int SerialProtocolTransmission::protcocolSendData(uint8_t *data, int len)
{
    QByteArray sendData(reinterpret_cast<const char*>(data), len);
    //qDebug() << sendData.toHex(' ') << endl;
    protocolHexSendData(sendData);
    return 0;
}

 int SerialProtocolTransmission::protcocolRecvData(uint8_t *data, int len)
 {
    ymodem->ymodemRecvData(data, len);
    return 0;
 }

void SerialProtocolTransmission::startYModemTransfer()
{
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("Please select a file first."));
        return;
    }

    QFile ymodemFile(filePath);
    if (!ymodemFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to open file."));
        return;
    }

    QFileInfo *ymodemFileInfo = new QFileInfo(filePath);
    QByteArray fileData = ymodemFile.readAll();
    QByteArray fileName = ymodemFileInfo->fileName().toUtf8();
    int fileSize = ymodemFileInfo->size();

    connect(serialWidget->serialPortManager, &SerialManager::dataReceived, this, [this](QByteArray data){
        protocolHexReceiveData(data);
        qDebug() << "data:" << data << endl;
        protcocolRecvData(recvData, sizeof(recvData));
    });

    connect(ymodem, &Ymodem::ymodemSendData, this, &SerialProtocolTransmission::protcocolSendData);
    connect(this, &SerialProtocolTransmission::startTrasmit, ymodem, &Ymodem::YmodemSendFileStart);
    connect(ymodem, &Ymodem::ymodemSendEnd, this, &SerialProtocolTransmission::endYModemTransfer);

    progressBar->setMaximum(0);
    progressBar->setMinimum(0);
    qDebug() << fileName << endl << fileSize;
    ymodem->YmodemSendFileReady(fileName, fileSize);
    ymodem->YmodemSendFileData(fileData, fileSize);
    qDebug() << "y" << endl;
    serialWidget->enableUi(false);
    qDebug() << "x" << endl;

    protocolThread.start();//todo:补一个触发线程的槽函数
    ymodem->moveToThread(&protocolThread);


    emit startTrasmit();

    //protocolThread.exit();
    // TODO: 实现 YModem 传输逻辑
    // 这里可以调用 YModem 协议的实现代码
    // 例如：YModem::sendFile(serialPort, filePath, progressBar);

    //serialWidget->enableUi(true);

    //QMessageBox::information(this, "Info", "YModem transfer started.");
}

void SerialProtocolTransmission::endYModemTransfer()
{
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    serialWidget->enableUi(true);
    QMessageBox::information(this, tr("Info"), tr("YModem transfer finished."));
    disconnect(ymodem, &Ymodem::ymodemSendData, this, &SerialProtocolTransmission::protcocolSendData);
    disconnect(this, &SerialProtocolTransmission::startTrasmit, ymodem, &Ymodem::YmodemSendFileStart);
}
