#include "serialportwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>
#include <QMessageBox>
#include <QDateTime>
#include <QLabel>

SerialPortWidget::SerialPortWidget(const QString &portName, QWidget *parent)
    : QWidget(parent), serialPort(nullptr), portName(portName), sentBytes(0), receivedBytes(0)
{
    // 添加配置文件
    settings = new QSettings("config.ini", QSettings::IniFormat);

    // Create UI elements
    receiveTextEdit = new QTextEdit(this);
    receiveTextEdit->setReadOnly(true);

    sendTextEdit = new QTextEdit(this);

    baudRateComboBox = new QComboBox(this);
    baudRateComboBox->addItems({"9600", "19200", "38400", "57600", "115200"});

    dataBitsComboBox = new QComboBox(this);
    dataBitsComboBox->addItems({"5", "6", "7", "8"});

    parityComboBox = new QComboBox(this);
    parityComboBox->addItems({"None", "Even", "Odd", "Mark", "Space"});

    stopBitsComboBox = new QComboBox(this);
    stopBitsComboBox->addItems({"1", "1.5", "2"});

    flowControlComboBox = new QComboBox(this);
    flowControlComboBox->addItems({"None", "Hardware", "Software"});

    connectButton = new QPushButton("Connect", this); // 合并连接和断开按钮

    hexReceiveCheckBox = new QCheckBox("HEX Receive", this);
    hexSendCheckBox = new QCheckBox("HEX Send", this);

    // 添加发送按钮
    sendButton = new QPushButton("Send", this);

    // 添加清空接收区按钮
    clearReceiveButton = new QPushButton("Clear Receive", this);

    // 添加底部栏，显示收发字符数
    statusLabel = new QLabel("Sent: 0 bytes | Received: 0 bytes", this);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 接收区和相关控件
    QHBoxLayout *upLayout = new QHBoxLayout;
    upLayout->addWidget(receiveTextEdit);

    // 接收区侧边布局：串口配置、十六进制接收勾选框和连接按钮
    QFormLayout *settingsLayout = new QFormLayout;
    settingsLayout->addRow("Baud Rate:", baudRateComboBox);
    settingsLayout->addRow("Data Bits:", dataBitsComboBox);
    settingsLayout->addRow("Parity:", parityComboBox);
    settingsLayout->addRow("Stop Bits:", stopBitsComboBox);
    settingsLayout->addRow("Flow Control:", flowControlComboBox);

    QVBoxLayout *receiveSideLayout = new QVBoxLayout;
    receiveSideLayout->addLayout(settingsLayout);
    receiveSideLayout->addWidget(hexReceiveCheckBox);
    receiveSideLayout->addWidget(connectButton);
    receiveSideLayout->addWidget(clearReceiveButton);

    upLayout->addLayout(receiveSideLayout);

    // 发送区和相关控件
    QHBoxLayout *downLayout = new QHBoxLayout;
    downLayout->addWidget(sendTextEdit);

    // 发送区侧边布局：十六进制发送勾选框和发送按钮
    QVBoxLayout *sendSideLayout = new QVBoxLayout;
    sendSideLayout->addWidget(hexSendCheckBox);
    sendSideLayout->addWidget(sendButton);

    downLayout->addLayout(sendSideLayout);

    // 将左右布局添加到主布局
    mainLayout->addLayout(upLayout);
    mainLayout->addLayout(downLayout);

    // 添加底部栏
    QVBoxLayout *bottomLayout = new QVBoxLayout;
    bottomLayout->addWidget(statusLabel);
    mainLayout->addLayout(bottomLayout);

    loadcomSettings();

    // Connect signals and slots
    connect(connectButton, &QPushButton::clicked, this, &SerialPortWidget::toggleConnection);
    connect(sendButton, &QPushButton::clicked, this, &SerialPortWidget::sendData);
    connect(clearReceiveButton, &QPushButton::clicked, this, &SerialPortWidget::clearReceiveArea);
}

SerialPortWidget::~SerialPortWidget()
{
    savecomSettings();
    if (serialPort && serialPort->isOpen()) {
        serialPort->close();
    }
    delete serialPort;
}

void SerialPortWidget::toggleConnection()
{
    if (serialPort && serialPort->isOpen()) {
        disconnectSerialPort();
    } else {
        connectSerialPort();
    }
}

void SerialPortWidget::connectSerialPort()
{
    serialPort = new QSerialPort(this);

    // 设置串口名称
    serialPort->setPortName(portName);

    // 设置串口参数
    serialPort->setBaudRate(baudRateComboBox->currentText().toInt());
    serialPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBitsComboBox->currentText().toInt()));
    serialPort->setParity(static_cast<QSerialPort::Parity>(parityComboBox->currentIndex()));
    serialPort->setStopBits(static_cast<QSerialPort::StopBits>(stopBitsComboBox->currentIndex() + 1));
    serialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(flowControlComboBox->currentIndex()));

    if (serialPort->open(QIODevice::ReadWrite)) {
        connectButton->setText("Disconnect");
        connect(serialPort, &QSerialPort::readyRead, this, &SerialPortWidget::receiveData);
    } else {
        QMessageBox::critical(this, "Error", "Failed to open serial port.");
    }
}

void SerialPortWidget::disconnectSerialPort()
{
    if (serialPort && serialPort->isOpen()) {
        serialPort->close();
    }
    connectButton->setText("Connect");
}

void SerialPortWidget::sendData()
{
    if (serialPort && serialPort->isOpen()) {
        QString dataStr = sendTextEdit->toPlainText();

        if (hexSendCheckBox->isChecked()) {
            dataStr = dataStr.replace(" ", "").replace("\n", "");
            QRegExp hexRegExp("^[0-9A-Fa-f]+$");
            if (!hexRegExp.exactMatch(dataStr)) {
                QMessageBox::warning(this, "Invalid HEX Data", "Please enter valid HEX characters (0-9, A-F).");
                return;
            }
            if (dataStr.length() % 2 != 0) {
                dataStr.prepend("0");
            }
            QByteArray data = QByteArray::fromHex(dataStr.toUtf8());
            if (serialPort->write(data) == -1) {
                QMessageBox::critical(this, "Error", "Failed to send data.");
            } else {
                sentBytes += data.size();
                updateStatusLabel();
                QDateTime currentDateTime = QDateTime::currentDateTime();
                receiveTextEdit->append("["+currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz")+"]");
                receiveTextEdit->append("Sent: " + dataStr);
            }
        } else {
            QByteArray data = dataStr.toUtf8();
            if (serialPort->write(data) == -1) {
                QMessageBox::critical(this, "Error", "Failed to send data.");
            } else {
                sentBytes += data.size();
                updateStatusLabel();
                QDateTime currentDateTime = QDateTime::currentDateTime();
                receiveTextEdit->append("["+currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz")+"]");
                receiveTextEdit->append("Sent: " + dataStr);
            }
        }
    } else {
        QMessageBox::warning(this, "Warning", "Serial port is not connected.");
    }
}

void SerialPortWidget::receiveData()
{
    if (serialPort && serialPort->isOpen()) {
        QByteArray data = serialPort->readAll();
        receivedBytes += data.size();
        updateStatusLabel();
        QDateTime currentDateTime = QDateTime::currentDateTime();
        receiveTextEdit->append("["+currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz")+"]");
        if (hexReceiveCheckBox->isChecked()) {
            receiveTextEdit->append("Recv: " + data.toHex());
        } else {
            receiveTextEdit->append("Recv: " + data);
        }
    }
}

void SerialPortWidget::clearReceiveArea()
{
    receiveTextEdit->clear();
    sentBytes = 0;
    receivedBytes = 0;
    updateStatusLabel();
}

void SerialPortWidget::updateStatusLabel()
{
    statusLabel->setText(QString("Sent: %1 bytes | Received: %2 bytes").arg(sentBytes).arg(receivedBytes));
}

void SerialPortWidget::loadcomSettings()
{
    baudRateComboBox->setCurrentText(settings->value(portName+"/BaudRate", "9600").toString());
    dataBitsComboBox->setCurrentText(settings->value(portName+"/DataBits", "8").toString());
    parityComboBox->setCurrentText(settings->value(portName+"/Parity", "None").toString());
    stopBitsComboBox->setCurrentText(settings->value(portName+"/StopBits", "1").toString());
    flowControlComboBox->setCurrentText(settings->value(portName+"/FlowControl", "None").toString());
    hexReceiveCheckBox->setChecked(settings->value(portName+"/HexReceive", false).toBool());
    hexSendCheckBox->setChecked(settings->value(portName+"/HexSend", false).toBool());
}

void SerialPortWidget::savecomSettings()
{
    settings->beginGroup(portName);
    settings->setValue("BaudRate", baudRateComboBox->currentText());
    settings->setValue("DataBits", dataBitsComboBox->currentText());
    settings->setValue("Parity", parityComboBox->currentText());
    settings->setValue("StopBits", stopBitsComboBox->currentText());
    settings->setValue("FlowControl", flowControlComboBox->currentText());
    settings->setValue("HexReceive", hexReceiveCheckBox->isChecked());
    settings->setValue("HexSend", hexSendCheckBox->isChecked());
    settings->endGroup();
    settings->sync();
}