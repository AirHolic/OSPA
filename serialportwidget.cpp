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

    QFormLayout *settingsLayout = new QFormLayout;
    settingsLayout->addRow("Baud Rate:", baudRateComboBox);
    settingsLayout->addRow("Data Bits:", dataBitsComboBox);
    settingsLayout->addRow("Parity:", parityComboBox);
    settingsLayout->addRow("Stop Bits:", stopBitsComboBox);
    settingsLayout->addRow("Flow Control:", flowControlComboBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(connectButton); // 合并后的按钮

    // 添加发送按钮和清空接收区按钮到布局
    QHBoxLayout *sendClearLayout = new QHBoxLayout;
    sendClearLayout->addWidget(sendButton);
    sendClearLayout->addWidget(clearReceiveButton);

    mainLayout->addLayout(settingsLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(receiveTextEdit);
    mainLayout->addWidget(sendTextEdit);
    mainLayout->addLayout(sendClearLayout);
    mainLayout->addWidget(hexReceiveCheckBox);
    mainLayout->addWidget(hexSendCheckBox);
    mainLayout->addWidget(statusLabel); // 添加底部栏

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
                receiveTextEdit->append(currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz"));
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
                receiveTextEdit->append(currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz"));
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
        receiveTextEdit->append(currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz"));
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
