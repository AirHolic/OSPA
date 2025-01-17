#include "ymodem.h"
#include "serialmanager.h"
#include "serialwidget.h"
#include "serialprotocoltransmission.h"
#include <QDebug>
uint8_t recvData[10];
uint8_t sendData[1030];

SerialProtocolTransmission::SerialProtocolTransmission(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnections();
    serialWidget = (SerialWidget*)parentWidget();
    ymodem = new Ymodem();

    connect(this, &SerialProtocolTransmission::protcocolRecvData, ymodem, &Ymodem::ymodemRecvData);
    connect(serialWidget->serialPortManager, &SerialManager::dataReceived, this, [this](QByteArray data){
        protocolHexReceiveData(data);
        emit protcocolRecvData(recvData,sizeof (recvData));
    });

    connect(ymodem, &Ymodem::ymodemSendData, this, &SerialProtocolTransmission::protcocolSendData);

    
}

SerialProtocolTransmission::~SerialProtocolTransmission()
{

}

void SerialProtocolTransmission::initUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 文件选择部分
    QHBoxLayout *fileLayout = new QHBoxLayout;
    openFileButton = new QPushButton("Open File", this);
    fileLabel = new QLabel("No file selected", this);
    fileLayout->addWidget(openFileButton);
    fileLayout->addWidget(fileLabel);

    // 进度条
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);

    // 开始传输按钮
    startButton = new QPushButton("Start YModem Transfer", this);
    startButton->setEnabled(false); // 初始禁用

    // 添加到主布局
    mainLayout->addLayout(fileLayout);
    mainLayout->addWidget(progressBar);
    mainLayout->addWidget(startButton);
}

void SerialProtocolTransmission::initConnections()
{
    connect(openFileButton, &QPushButton::clicked, this, &SerialProtocolTransmission::openFile);
    connect(startButton, &QPushButton::clicked, this, &SerialProtocolTransmission::startYModemTransfer);
    //bug
    //connect(serialManager, &SerialManager::dataReceived, this, &SerialProtocolTransmission::protocolHexReceiveData);
}

void SerialProtocolTransmission::openFile()
{
    filePath = QFileDialog::getOpenFileName(this, "Select File", "", "All Files (*)");
    if (!filePath.isEmpty()) {
        fileLabel->setText(filePath);
        startButton->setEnabled(true); // 启用开始按钮
    } else {
        fileLabel->setText("No file selected");
        startButton->setEnabled(false); // 禁用开始按钮
    }
    serialWidget->enableUi(false);
}

void SerialProtocolTransmission::protocolHexSendData(const QByteArray &data)
{
    serialWidget->serialPortManager->sendData(data);
    serialWidget->sentBytes += data.size();
    serialWidget->updateStatusLabel();
    serialWidget->logMessage("Sent: " + data.toHex(' ').toUpper());
}

void SerialProtocolTransmission::protocolHexReceiveData(const QByteArray &data)
{
    memcpy(recvData, data.constData(), data.size());
}

int SerialProtocolTransmission::protcocolSendData(uint8_t *data, uint16_t len)
{
    QByteArray sendData(reinterpret_cast<const char*>(data), len);
    protocolHexSendData(sendData);
    return 0;
}

// int SerialProtocolTransmission::protcocolRecvData(uint8_t *data, uint16_t len)
// {
//     // data = recvData;
//     // len = sizeof (recvData);
//     // return 0;
// }

void SerialProtocolTransmission::startYModemTransfer()
{
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a file first.");
        return;
    }

    QFile ymodemFile(filePath);
    if (!ymodemFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Warning", "Failed to open file.");
        return;
    }

    QFileInfo *ymodemFileInfo = new QFileInfo(filePath);
    QByteArray fileData = ymodemFile.readAll();
    QByteArray fileName = ymodemFileInfo->fileName().toUtf8();
    int fileSize = ymodemFileInfo->size();

    progressBar->setMaximum(0);
    progressBar->setMinimum(0);
    qDebug() << fileName << endl << fileSize;
    ymodem->YmodemSendFileReady(fileName, fileSize);
    ymodem->YmodemSendFileData(fileData, fileSize);
    qDebug() << "y" << endl;
    serialWidget->enableUi(false);
    qDebug() << "x" << endl;

    ymodem->YmodemSendFileStart();

    // TODO: 实现 YModem 传输逻辑
    // 这里可以调用 YModem 协议的实现代码
    // 例如：YModem::sendFile(serialPort, filePath, progressBar);

    serialWidget->enableUi(true);

    QMessageBox::information(this, "Info", "YModem transfer started.");
}
