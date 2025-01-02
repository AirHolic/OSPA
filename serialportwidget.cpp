#include "serialportwidget.h"
#include "searchdialog.h"
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
#include <QShortcut>
#include <QDockWidget>

SerialPortWidget::SerialPortWidget(const QString &portName, QWidget *parent)
    : QWidget(parent), serialPort(nullptr), portName(portName), sentBytes(0), receivedBytes(0), dockWidget(nullptr)
{
    settings = new QSettings("config.ini", QSettings::IniFormat, this);
    initUI();
    initConnections();
    loadSettings();

    // 添加 Ctrl+F 快捷键
    QShortcut *searchShortcut = new QShortcut(QKeySequence::Find, this);
    connect(searchShortcut, &QShortcut::activated, this, &SerialPortWidget::openSearchDialog);

    // 初始化搜索对话框
    searchDialog = new SearchDialog(receiveTextEdit, this);
    searchDialog->setParent(this,searchDialog->windowFlags());
    //searchDialog->setWindowFlag(Qt::Window, false); // 设置为非独立窗口
    //searchDialog->setParent(this,searchDialog->windowFlags());
    searchDialog->hide(); // 初始隐藏
    searchDialog->setShowFlag(false);// 初始隐藏
}

SerialPortWidget::~SerialPortWidget()
{
    saveSettings();
    if (serialPort && serialPort->isOpen()) {
        serialPort->close();
    }
    delete serialPort;
}

void SerialPortWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (searchDialog->getShowFlag() == true) {
        searchDialog->show(); // 显示搜索对话框
    }
}

void SerialPortWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if (searchDialog) {
        searchDialog->hide(); // 隐藏搜索对话框
    }
}

void SerialPortWidget::openSearchDialog()
{
    if (searchDialog) {
        searchDialog->show(); // 显示搜索对话框
        searchDialog->activateWindow(); // 激活窗口
        searchDialog->setShowFlag(true);// 设置显示标志
    }
}

void SerialPortWidget::setDockWidget(QDockWidget *dockWidget)
{
    this->dockWidget = dockWidget;
}

void SerialPortWidget::initUI()
{
    // 接收区
    receiveTextEdit = new QTextEdit(this);
    receiveTextEdit->setReadOnly(true);

    // 发送区
    sendTextEdit = new QTextEdit(this);

    // 串口配置
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

    // 按钮
    connectButton = new QPushButton("Connect", this);
    sendButton = new QPushButton("Send", this);
    clearReceiveButton = new QPushButton("Clear Receive", this);

    // 复选框
    hexReceiveCheckBox = new QCheckBox("HEX Receive", this);
    hexSendCheckBox = new QCheckBox("HEX Send", this);

    // 状态栏
    statusLabel = new QLabel("Sent: 0 bytes | Received: 0 bytes", this);

    // 布局
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
    mainLayout->addWidget(statusLabel);
}

void SerialPortWidget::initConnections()
{
    connect(connectButton, &QPushButton::clicked, this, &SerialPortWidget::toggleConnection);
    connect(sendButton, &QPushButton::clicked, this, &SerialPortWidget::sendData);
    connect(clearReceiveButton, &QPushButton::clicked, this, &SerialPortWidget::clearReceiveArea);
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
    serialPort->setPortName(portName);
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
                logMessage("Sent: " + dataStr);
            }
        } else {
            QByteArray data = dataStr.toUtf8();
            if (serialPort->write(data) == -1) {
                QMessageBox::critical(this, "Error", "Failed to send data.");
            } else {
                sentBytes += data.size();
                updateStatusLabel();
                logMessage("Sent: " + dataStr);
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
        if (hexReceiveCheckBox->isChecked()) {
            logMessage("Recv: " + data.toHex());
        } else {
            logMessage("Recv: " + data);
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

void SerialPortWidget::logMessage(const QString &message)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    receiveTextEdit->append("[" + currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz") + "] " + message);
}

void SerialPortWidget::loadSettings()
{
    baudRateComboBox->setCurrentText(settings->value(portName + "/BaudRate", "9600").toString());
    dataBitsComboBox->setCurrentText(settings->value(portName + "/DataBits", "8").toString());
    parityComboBox->setCurrentText(settings->value(portName + "/Parity", "None").toString());
    stopBitsComboBox->setCurrentText(settings->value(portName + "/StopBits", "1").toString());
    flowControlComboBox->setCurrentText(settings->value(portName + "/FlowControl", "None").toString());
    hexReceiveCheckBox->setChecked(settings->value(portName + "/HexReceive", false).toBool());
    hexSendCheckBox->setChecked(settings->value(portName + "/HexSend", false).toBool());
}

void SerialPortWidget::saveSettings()
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


