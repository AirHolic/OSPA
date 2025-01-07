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
#include <QEvent>
#include <QDebug>

SerialPortWidget::SerialPortWidget(const QString &portName, QWidget *parent)
    : QWidget(parent), portName(portName), serialPortManager(new SerialPortManager(this)), sentBytes(0), receivedBytes(0)
{
    settings = new QSettings("config.ini", QSettings::IniFormat, this);
    initUI();
    initConnections();
    initSearchDialog();
    loadSettings();
}

SerialPortWidget::~SerialPortWidget()
{
    saveSettings();
}

/* Event */

void SerialPortWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (searchDialog->getShowFlag())
    {
        searchDialog->show();
    }
}

void SerialPortWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if (searchDialog)
    {
        searchDialog->hide();
    }
}

/* Event */

void SerialPortWidget::openSearchDialog()
{
    if (searchDialog)
    {
        searchDialog->show();            // 显示搜索对话框
        searchDialog->activateWindow();  // 激活窗口
        searchDialog->setShowFlag(true); // 设置显示标志
    }
}

void SerialPortWidget::initUI()
{
    // 接收区
    receiveTextEdit = new QTextEdit(this);
    receiveTextEdit->setReadOnly(true);

    // 单条发送区
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
    searchButton = new QPushButton("Search", this);
    clearReceiveButton = new QPushButton("Clear Receive", this);

    // 复选框
    hexReceiveCheckBox = new QCheckBox("HEX Receive", this);
    hexSendCheckBox = new QCheckBox("HEX Send", this);
    sendNewRowCheckbox = new QCheckBox("Send New Row", this);

    // 状态栏
    statusLabel = new QLabel("Sent: 0 bytes | Received: 0 bytes", this);
    statusLabel->setAlignment(Qt::AlignRight);

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
    receiveSideLayout->addWidget(searchButton);
    receiveSideLayout->addWidget(clearReceiveButton);

    upLayout->addLayout(receiveSideLayout);

    // 发送区和相关控件
    QHBoxLayout *downLayout = new QHBoxLayout;
    sendTabWidget = new QTabWidget(this);
    downLayout->addWidget(sendTabWidget);

    // 单条发送区
    QWidget *singleSendWidget = new QWidget;
    QHBoxLayout *singleSendLayout = new QHBoxLayout;
    singleSendWidget->setLayout(singleSendLayout);
    sendTabWidget->addTab(singleSendWidget, "Single");
    singleSendLayout->addWidget(sendTextEdit);

    // 发送区侧边布局：十六进制发送勾选框和发送按钮
    QVBoxLayout *sendSideLayout = new QVBoxLayout;
    sendSideLayout->addWidget(hexSendCheckBox);
    sendSideLayout->addWidget(sendNewRowCheckbox);
    sendSideLayout->addWidget(sendButton);
    singleSendLayout->addLayout(sendSideLayout);

    // // 多条发送区
    // QWidget *multiSendWidget = new QWidget;
    // QHBoxLayout *multiSendLayout = new QHBoxLayout;

    // 将上下布局添加到主布局
    mainLayout->addLayout(upLayout);
    mainLayout->addLayout(downLayout);

    // 添加底部栏
    mainLayout->addWidget(statusLabel);
}

void SerialPortWidget::initConnections()
{
    connect(connectButton, &QPushButton::clicked, this, &SerialPortWidget::toggleConnection);
    connect(sendButton, &QPushButton::clicked, this, &SerialPortWidget::sendData);
    connect(searchButton, &QPushButton::clicked, this, &SerialPortWidget::openSearchDialog);
    connect(clearReceiveButton, &QPushButton::clicked, this, &SerialPortWidget::clearReceiveArea);
    connect(serialPortManager, &SerialPortManager::dataReceived, this, &SerialPortWidget::onDataReceived);
    connect(serialPortManager, &SerialPortManager::errorOccurred, this, &SerialPortWidget::onErrorOccurred);
}

void SerialPortWidget::initSearchDialog()
{
    // 初始化搜索对话框
    searchDialog = new SearchDialog(portName, receiveTextEdit, this);
    searchDialog->setParent(this, searchDialog->windowFlags());
    searchDialog->hide();             // 初始隐藏
    searchDialog->setShowFlag(false); // 初始隐藏

    // 添加 Ctrl+F 快捷键
    searchShortcut = new QShortcut(QKeySequence::Find, this);
    connect(searchShortcut, &QShortcut::activated, this, &SerialPortWidget::openSearchDialog);
}

void SerialPortWidget::toggleConnection()
{
    if (serialPortManager->isConnected())
    {
        serialPortManager->disconnectSerialPort();
        connectButton->setText("Connect");
    }
    else
    {
        if (serialPortManager->connectSerialPort(portName.mid(0, portName.indexOf(" ")),
                                                 baudRateComboBox->currentText().toInt(),
                                                 dataBitsComboBox->currentText().toInt(),
                                                 parityComboBox->currentIndex(),
                                                 stopBitsComboBox->currentIndex() + 1,
                                                 flowControlComboBox->currentIndex()))
        {
            connectButton->setText("Disconnect");
        }
    }
}

void SerialPortWidget::sendData()
{
    QString dataStr = sendTextEdit->toPlainText();
    if (dataStr.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Please enter data to send.");
        return;
    }

    QByteArray data;
    if (hexSendCheckBox->isChecked())
    {
        dataStr = dataStr.replace(" ", "").replace("\n", "");
        QRegExp hexRegExp("^[0-9A-Fa-f]+$");
        if (!hexRegExp.exactMatch(dataStr))
        {
            QMessageBox::warning(this, "Invalid HEX Data", "Please enter valid HEX characters (0-9, A-F).");
            return;
        }
        if (dataStr.length() % 2 != 0)
        {
            dataStr.prepend("0");
        }
        data = QByteArray::fromHex(dataStr.toUtf8());
    }
    else
    {
        data = dataStr.toUtf8();
    }

    if (sendNewRowCheckbox->isChecked())
    {
        data.append(0x0D).append(0x0A);
    }

    serialPortManager->sendData(data);
    sentBytes += data.size();
    updateStatusLabel();
    logMessage("Sent: " + (hexSendCheckBox->isChecked() ? data.toHex(' ').toUpper() : data));
}

void SerialPortWidget::onDataReceived(const QByteArray &data)
{
    receivedBytes += data.size();
    updateStatusLabel();
    logMessage("Recv: " + (hexReceiveCheckBox->isChecked() ? data.toHex(' ').toUpper() : data));
}

void SerialPortWidget::onErrorOccurred(const QString &error)
{
    QMessageBox::critical(this, "Error", error);
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
    sendNewRowCheckbox->setChecked(settings->value(portName + "/NewRow", false).toBool());
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
    settings->setValue("NewRow", sendNewRowCheckbox->isChecked());
    settings->endGroup();
    settings->sync();
}
