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

SerialPortWidget::SerialPortWidget(const QString &portName, QWidget *parent)
    : QWidget(parent), serialPort(nullptr), portName(portName)
{
    // Create UI elements
    receiveTextEdit = new QTextEdit(this);
    receiveTextEdit->setReadOnly(true);

    sendTextEdit = new QTextEdit(this);

    baudRateComboBox = new QComboBox(this);
    baudRateComboBox->addItems({"9600", "19200", "38400", "57600", "115200"});
    baudRateComboBox->setCurrentText("9600");

    dataBitsComboBox = new QComboBox(this);
    dataBitsComboBox->addItems({"5", "6", "7", "8"});
    dataBitsComboBox->setCurrentText("8");

    parityComboBox = new QComboBox(this);
    parityComboBox->addItems({"None", "Even", "Odd", "Mark", "Space"});
    parityComboBox->setCurrentText("None");

    stopBitsComboBox = new QComboBox(this);
    stopBitsComboBox->addItems({"1", "1.5", "2"});
    stopBitsComboBox->setCurrentText("1");

    flowControlComboBox = new QComboBox(this);
    flowControlComboBox->addItems({"None", "Hardware", "Software"});
    flowControlComboBox->setCurrentText("None");

    connectButton = new QPushButton("Connect", this);
    disconnectButton = new QPushButton("Disconnect", this);
    disconnectButton->setEnabled(false);

    hexReceiveCheckBox = new QCheckBox("HEX Receive", this);
    hexSendCheckBox = new QCheckBox("HEX Send", this);

    // 添加发送按钮
    sendButton = new QPushButton("Send", this);

    // Layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QFormLayout *settingsLayout = new QFormLayout;
    settingsLayout->addRow("Baud Rate:", baudRateComboBox);
    settingsLayout->addRow("Data Bits:", dataBitsComboBox);
    settingsLayout->addRow("Parity:", parityComboBox);
    settingsLayout->addRow("Stop Bits:", stopBitsComboBox);
    settingsLayout->addRow("Flow Control:", flowControlComboBox);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(connectButton);
    buttonLayout->addWidget(disconnectButton);

    // 添加发送按钮到布局
    QHBoxLayout *sendLayout = new QHBoxLayout;
    sendLayout->addWidget(sendButton);

    mainLayout->addLayout(settingsLayout);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(receiveTextEdit);
    mainLayout->addWidget(sendTextEdit);
    mainLayout->addLayout(sendLayout); // 将发送按钮布局添加到主布局
    mainLayout->addWidget(hexReceiveCheckBox);
    mainLayout->addWidget(hexSendCheckBox);

    // Connect signals and slots
    connect(connectButton, &QPushButton::clicked, this, &SerialPortWidget::connectSerialPort);
    connect(disconnectButton, &QPushButton::clicked, this, &SerialPortWidget::disconnectSerialPort);
    connect(sendButton, &QPushButton::clicked, this, &SerialPortWidget::sendData); // 连接发送按钮的点击事件
}

SerialPortWidget::~SerialPortWidget()
{
    if (serialPort && serialPort->isOpen()) {
        serialPort->close();
    }
    delete serialPort;
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
        connectButton->setEnabled(false);
        disconnectButton->setEnabled(true);

        // 连接 readyRead 信号
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
    connectButton->setEnabled(true);
    disconnectButton->setEnabled(false);
}

void SerialPortWidget::sendData()
{
    if (serialPort && serialPort->isOpen()) {
        // 获取发送区的内容
        QString dataStr = sendTextEdit->toPlainText();

        // 如果 HEX 发送模式被选中
        if (hexSendCheckBox->isChecked()) {
            // 移除所有空格和换行符
            dataStr = dataStr.replace(" ", "").replace("\n", "");

            // 检查是否为有效的 HEX 字符串
            QRegExp hexRegExp("^[0-9A-Fa-f]+$");
            if (!hexRegExp.exactMatch(dataStr)) {
                QMessageBox::warning(this, "Invalid HEX Data", "Please enter valid HEX characters (0-9, A-F).");
                return;
            }

            // 如果长度为奇数，补零
            if (dataStr.length() % 2 != 0) {
                dataStr.prepend("0");
            }

            // 将 HEX 字符串转换为 QByteArray
            QByteArray data = QByteArray::fromHex(dataStr.toUtf8());

            // 发送数据
            if (serialPort->write(data) == -1) {
                QMessageBox::critical(this, "Error", "Failed to send data.");
            } else {
                // 将发送的数据显示在接收区（自动分割）
                QString formattedData;
                for (int i = 0; i < dataStr.length(); i += 2) {
                    formattedData += dataStr.mid(i, 2) + " ";
                }
                formattedData = formattedData.trimmed(); // 移除最后一个空格
                QDateTime currentDateTime = QDateTime::currentDateTime();//显示时间戳
                receiveTextEdit->append(currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz"));
                receiveTextEdit->append("Sent: " + formattedData);


                // 清空发送区（可选）
                sendTextEdit->clear();
                sendTextEdit->append(formattedData);
            }
        } else {
            // 直接发送文本数据
            QByteArray data = dataStr.toUtf8();
            if (serialPort->write(data) == -1) {
                QMessageBox::critical(this, "Error", "Failed to send data.");
            } else {
                // 将发送的数据显示在接收区
                QDateTime currentDateTime = QDateTime::currentDateTime();//显示时间戳
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
        QDateTime currentDateTime = QDateTime::currentDateTime();//显示时间戳
        receiveTextEdit->append(currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz"));
        if (hexReceiveCheckBox->isChecked()) {
            receiveTextEdit->append("Recv: " + data.toHex());
        } else {
            receiveTextEdit->append("Recv: " + data);
        }
    }
}

void SerialPortWidget::updateSettings()
{
    if (serialPort && serialPort->isOpen()) {
        serialPort->setBaudRate(baudRateComboBox->currentText().toInt());
        serialPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBitsComboBox->currentText().toInt()));
        serialPort->setParity(static_cast<QSerialPort::Parity>(parityComboBox->currentIndex()));
        serialPort->setStopBits(static_cast<QSerialPort::StopBits>(stopBitsComboBox->currentIndex() + 1));
        serialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(flowControlComboBox->currentIndex()));
    }
}

void SerialPortWidget::saveSetting()
{

}
