#include "serialwidget.h"
#include "searchdialog.h"
#include <QGridLayout>
#include "serialprotocoltransmission.h"
#include "serialmultisendunit.h"
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
#include <QValidator>

SerialWidget::SerialWidget(const QString &portName, QWidget *parent)
    : QWidget(parent), portName(portName), serialPortManager(new SerialManager(this)), sentBytes(0), receivedBytes(0)
{
    serialSettings = new QSettings("serialconfig.ini", QSettings::IniFormat, this);
    multiSendSettings = new QSettings("multisendconfig.ini", QSettings::IniFormat, this);
    initUI();
    initConnections();
    initSearchDialog();
    loadSettings();
}

SerialWidget::~SerialWidget()
{
    saveSettings();
}

/* Event */

void SerialWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (searchDialog->getShowFlag())
    {
        searchDialog->show();
    }
}

void SerialWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    if (searchDialog)
    {
        searchDialog->hide();
    }
}

/* Event */

void SerialWidget::openSearchDialog()
{
    if (searchDialog)
    {
        searchDialog->show();            // 显示搜索对话框
        searchDialog->activateWindow();  // 激活窗口
        searchDialog->setShowFlag(true); // 设置显示标志
    }
}

void SerialWidget::initUI()
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
    sendSideLayout = new QVBoxLayout;
    sendSideLayout->addWidget(hexSendCheckBox);
    sendSideLayout->addWidget(sendNewRowCheckbox);
    sendSideLayout->addWidget(sendButton);
    singleSendLayout->addLayout(sendSideLayout);

    // 多条发送区
    QWidget *multiSendWidget = new QWidget;
    QHBoxLayout *multiSendLayout = new QHBoxLayout;
    QGridLayout *multiSendGridLayout = new QGridLayout;
    multiSendSideLayout = new QVBoxLayout;
    multiSendUnit0 = new serialMultiSendUnit(0);
    multiSendUnit1 = new serialMultiSendUnit(1);
    multiSendUnit2 = new serialMultiSendUnit(2);
    multiSendUnit3 = new serialMultiSendUnit(3);
    multiSendUnit4 = new serialMultiSendUnit(4);
    multiSendUnit5 = new serialMultiSendUnit(5);
    multiSendUnit6 = new serialMultiSendUnit(6);
    multiSendUnit7 = new serialMultiSendUnit(7);
    multiSendUnit8 = new serialMultiSendUnit(8);
    multiSendUnit9 = new serialMultiSendUnit(9);
    multiSendGridLayout->addWidget(multiSendUnit0, 0, 0);
    multiSendGridLayout->addWidget(multiSendUnit1, 0, 1);
    multiSendGridLayout->addWidget(multiSendUnit2, 1, 0);
    multiSendGridLayout->addWidget(multiSendUnit3, 1, 1);
    multiSendGridLayout->addWidget(multiSendUnit4, 2, 0);
    multiSendGridLayout->addWidget(multiSendUnit5, 2, 1);
    multiSendGridLayout->addWidget(multiSendUnit6, 3, 0);
    multiSendGridLayout->addWidget(multiSendUnit7, 3, 1);
    multiSendGridLayout->addWidget(multiSendUnit8, 4, 0);
    multiSendGridLayout->addWidget(multiSendUnit9, 4, 1);
    multiSendLayout->addLayout(multiSendGridLayout);
    multiSendWidget->setLayout(multiSendLayout);
    sendTabWidget->addTab(multiSendWidget, "Multi");

    // 多条发送区侧边布局
    multiCycleSendSideLayout = new QVBoxLayout;
    QHBoxLayout *multiCycleSendLayout = new QHBoxLayout;

    multiCycleSendCheckBox = new QCheckBox("Cycle Send", this);
    multiCycleSendLineEdit = new QLineEdit(this);
    multiCycleSendLineEdit->setText("1000");
    multiCycleSendLineEdit->setValidator(new QIntValidator(1, 1000, this));
    multiCycleSendLabel = new QLabel("cycle", this);
    multiCycleSendUnitLabel = new QLabel("ms", this);

    multiCycleSendSideLayout->addWidget(multiCycleSendCheckBox);
    multiCycleSendLayout->addWidget(multiCycleSendLabel);
    multiCycleSendLayout->addWidget(multiCycleSendLineEdit);
    multiCycleSendLayout->addWidget(multiCycleSendUnitLabel);
    multiCycleSendSideLayout->addLayout(multiCycleSendLayout);
    multiSendSideLayout->addLayout(multiCycleSendSideLayout);

    multiSendLayout->addLayout(multiSendSideLayout);

    // ymodem发送区
    QWidget *ymodemSendWidget = new QWidget;
    ymodemWidget = new SerialProtocolTransmission(this);
    QVBoxLayout *ymodemLayout = new QVBoxLayout(ymodemSendWidget);
    ymodemLayout->addWidget(ymodemWidget);
    sendTabWidget->addTab(ymodemSendWidget, "YModem");

    // 将上下布局添加到主布局
    mainLayout->addLayout(upLayout);
    mainLayout->addLayout(downLayout);

    // 添加底部栏
    mainLayout->addWidget(statusLabel);

    // 未连接时禁用相关控件
    enableUi(false);
    ymodemWidget->protocolEnableUI(false);
}

void SerialWidget::enableUi(bool enable)
{
    multiCycleSendCheckBox->setEnabled(enable);
    sendButton->setEnabled(enable);
    multiSendUnit0->getPushButton()->setEnabled(enable);
    multiSendUnit1->getPushButton()->setEnabled(enable);
    multiSendUnit2->getPushButton()->setEnabled(enable);
    multiSendUnit3->getPushButton()->setEnabled(enable);
    multiSendUnit4->getPushButton()->setEnabled(enable);
    multiSendUnit5->getPushButton()->setEnabled(enable);
    multiSendUnit6->getPushButton()->setEnabled(enable);
    multiSendUnit7->getPushButton()->setEnabled(enable);
    multiSendUnit8->getPushButton()->setEnabled(enable);
    multiSendUnit9->getPushButton()->setEnabled(enable);
    if(enable == false)
    {
        multiCycleSendCheckBox->setChecked(false);
    }
}

void SerialWidget::sendAddUi(int index)
{
    if(index == 0)
    {
        sendSideLayout->addWidget(hexSendCheckBox);
        sendSideLayout->addWidget(sendNewRowCheckbox);
        sendSideLayout->addWidget(sendButton);
    }
    else if(index == 1)
    {
        multiSendSideLayout->addWidget(hexSendCheckBox);
        multiSendSideLayout->addWidget(sendNewRowCheckbox);
        multiSendSideLayout->removeItem(multiCycleSendSideLayout);
        multiSendSideLayout->addLayout(multiCycleSendSideLayout);
    }
}

void SerialWidget::initConnections()
{
    connect(sendTabWidget, &QTabWidget::currentChanged, this, &SerialWidget::sendAddUi);
    connect(connectButton, &QPushButton::clicked, this, &SerialWidget::toggleConnection);
    connect(sendButton, &QPushButton::clicked, this, &SerialWidget::sendData);
    connect(searchButton, &QPushButton::clicked, this, &SerialWidget::openSearchDialog);
    connect(clearReceiveButton, &QPushButton::clicked, this, &SerialWidget::clearReceiveArea);
    connect(serialPortManager, &SerialManager::dataReceived, this, &SerialWidget::onDataReceived);
    connect(serialPortManager, &SerialManager::errorOccurred, this, &SerialWidget::onErrorOccurred);
    connect(multiCycleSendCheckBox, &QCheckBox::stateChanged, this, &SerialWidget::multiCycleTimer);

    connect(multiSendUnit0, &serialMultiSendUnit::clickPushButton, this, &SerialWidget::multiSendData);
    connect(multiSendUnit1, &serialMultiSendUnit::clickPushButton, this, &SerialWidget::multiSendData);
    connect(multiSendUnit2, &serialMultiSendUnit::clickPushButton, this, &SerialWidget::multiSendData);
    connect(multiSendUnit3, &serialMultiSendUnit::clickPushButton, this, &SerialWidget::multiSendData);
    connect(multiSendUnit4, &serialMultiSendUnit::clickPushButton, this, &SerialWidget::multiSendData);
    connect(multiSendUnit5, &serialMultiSendUnit::clickPushButton, this, &SerialWidget::multiSendData);
    connect(multiSendUnit6, &serialMultiSendUnit::clickPushButton, this, &SerialWidget::multiSendData);
    connect(multiSendUnit7, &serialMultiSendUnit::clickPushButton, this, &SerialWidget::multiSendData);
    connect(multiSendUnit8, &serialMultiSendUnit::clickPushButton, this, &SerialWidget::multiSendData);
    connect(multiSendUnit9, &serialMultiSendUnit::clickPushButton, this, &SerialWidget::multiSendData);
}

void SerialWidget::initSearchDialog()
{
    // 初始化搜索对话框
    searchDialog = new SearchDialog(portName, receiveTextEdit, this);
    searchDialog->setParent(this, searchDialog->windowFlags());
    searchDialog->hide();             // 初始隐藏
    searchDialog->setShowFlag(false); // 初始隐藏

    // 添加 Ctrl+F 快捷键
    searchShortcut = new QShortcut(QKeySequence::Find, this);
    connect(searchShortcut, &QShortcut::activated, this, &SerialWidget::openSearchDialog);
}

void SerialWidget::toggleConnection()
{
    if (serialPortManager->isConnected())
    {
        multiCycleSendCheckBox->setChecked(false);
        enableUi(false);

        serialPortManager->disconnectSerialPort();
        connectButton->setText("Connect");
        ymodemWidget->protocolEnableUI(false);
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
            enableUi(true);
            ymodemWidget->protocolEnableUI(true);
        }
    }
}

void SerialWidget::multiSendData(QString &dataStr)
{
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

void SerialWidget::sendData()
{
    QString dataStr = sendTextEdit->toPlainText();
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

void SerialWidget::multiCycleTimer(int state)
{
    if (state == Qt::Checked)
    {
        if (multiCycleSendLineEdit->text().isEmpty())
        {
            QMessageBox::warning(this, "Warning", "Please enter cycle time.");
            multiCycleSendCheckBox->setChecked(false);
            return;
        }

        multiCycleSendLineEdit->setReadOnly(true);
        multiCycleSendLabel->setEnabled(false);
        multiCycleSendUnitLabel->setEnabled(false);

        multiCycleSendTimer = new QTimer(this);
        multiCycleSendTimer->setInterval(multiCycleSendLineEdit->text().toInt());
        connect(multiCycleSendTimer, &QTimer::timeout, this, &SerialWidget::multiAutoSendData);
        multiCycleSendTimer->start();
    }
    else
    {
        multiCycleSendLineEdit->setReadOnly(false);
        multiCycleSendLabel->setEnabled(true);
        multiCycleSendUnitLabel->setEnabled(true);

        if (multiCycleSendTimer)
        {
            multiCycleSendTimer->stop();
            multiCycleSendTimer->deleteLater();
        }
    }

}

void SerialWidget::multiAutoSendData()
{
    switch (unitId)
    {//相关警告为故意不加break,可无视
    case 0:
        if(multiSendUnit0->getCheckBoxStatus() == true)
        {
            multiSendUnit0->getPushButton()->click();
            break;
        }
        unitId++;
    case 1:
        if(multiSendUnit1->getCheckBoxStatus() == true)
        {
            multiSendUnit1->getPushButton()->click();
            break;
        }
        unitId++;
    case 2:
        if(multiSendUnit2->getCheckBoxStatus() == true)
        {
            multiSendUnit2->getPushButton()->click();
            break;
        }
        unitId++;
    case 3:
        if(multiSendUnit3->getCheckBoxStatus() == true)
        {
            multiSendUnit3->getPushButton()->click();
            break;
        }
        unitId++;
    case 4:
        if(multiSendUnit4->getCheckBoxStatus() == true)
        {
            multiSendUnit4->getPushButton()->click();
            break;
        }
        unitId++;
    case 5:
        if(multiSendUnit5->getCheckBoxStatus() == true)
        {
            multiSendUnit5->getPushButton()->click();
            break;
        }
        unitId++;
    case 6:
        if(multiSendUnit6->getCheckBoxStatus() == true)
        {
            multiSendUnit6->getPushButton()->click();
            break;
        }
        unitId++;
    case 7:
        if(multiSendUnit7->getCheckBoxStatus() == true)
        {
            multiSendUnit7->getPushButton()->click();
            break;
        }
        unitId++;
    case 8:
        if(multiSendUnit8->getCheckBoxStatus() == true)
        {
            multiSendUnit8->getPushButton()->click();
            break;
        }
        unitId++;
    case 9:
        if(multiSendUnit9->getCheckBoxStatus() == true)
        {
            multiSendUnit9->getPushButton()->click();
            break;
        }
        unitId++;
    default:
        break;
    }
    unitId++;
    if(unitId>9)
    {
        unitId = 0;
    }
}

void SerialWidget::onDataReceived(const QByteArray &data)
{
    receivedBytes += data.size();
    updateStatusLabel();
    logMessage("Recv: " + (hexReceiveCheckBox->isChecked() ? data.toHex(' ').toUpper() : data));
}

void SerialWidget::onErrorOccurred(const QString &error)
{
    QMessageBox::critical(this, "Error", error);
}

void SerialWidget::clearReceiveArea()
{
    receiveTextEdit->clear();
    sentBytes = 0;
    receivedBytes = 0;
    updateStatusLabel();
}

void SerialWidget::updateStatusLabel()
{
    statusLabel->setText(QString("Sent: %1 bytes | Received: %2 bytes").arg(sentBytes).arg(receivedBytes));
}

void SerialWidget::logMessage(const QString &message)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    receiveTextEdit->append("[" + currentDateTime.toString("yyyy-MM-dd hh:mm:ss.zzz") + "] " + message);
}

void SerialWidget::loadSettings()
{
    baudRateComboBox->setCurrentText(serialSettings->value(portName + "/BaudRate", "9600").toString());
    dataBitsComboBox->setCurrentText(serialSettings->value(portName + "/DataBits", "8").toString());
    parityComboBox->setCurrentText(serialSettings->value(portName + "/Parity", "None").toString());
    stopBitsComboBox->setCurrentText(serialSettings->value(portName + "/StopBits", "1").toString());
    flowControlComboBox->setCurrentText(serialSettings->value(portName + "/FlowControl", "None").toString());
    hexReceiveCheckBox->setChecked(serialSettings->value(portName + "/HexReceive", false).toBool());
    hexSendCheckBox->setChecked(serialSettings->value(portName + "/HexSend", false).toBool());
    sendNewRowCheckbox->setChecked(serialSettings->value(portName + "/NewRow", false).toBool());

    multiSendUnit0->getLineEdit()->setText(multiSendSettings->value(portName + "/Unit0", "").toString());
    multiSendUnit0->getCheckBox()->setChecked(multiSendSettings->value(portName + "/Unit0/CheckBox", false).toBool());

    multiSendUnit1->getLineEdit()->setText(multiSendSettings->value(portName + "/Unit1", "").toString());
    multiSendUnit1->getCheckBox()->setChecked(multiSendSettings->value(portName + "/Unit1/CheckBox", false).toBool());

    multiSendUnit2->getLineEdit()->setText(multiSendSettings->value(portName + "/Unit2", "").toString());
    multiSendUnit2->getCheckBox()->setChecked(multiSendSettings->value(portName + "/Unit2/CheckBox", false).toBool());

    multiSendUnit3->getLineEdit()->setText(multiSendSettings->value(portName + "/Unit3", "").toString());
    multiSendUnit3->getCheckBox()->setChecked(multiSendSettings->value(portName + "/Unit3/CheckBox", false).toBool());

    multiSendUnit4->getLineEdit()->setText(multiSendSettings->value(portName + "/Unit4", "").toString());
    multiSendUnit4->getCheckBox()->setChecked(multiSendSettings->value(portName + "/Unit4/CheckBox", false).toBool());

    multiSendUnit5->getLineEdit()->setText(multiSendSettings->value(portName + "/Unit5", "").toString());
    multiSendUnit5->getCheckBox()->setChecked(multiSendSettings->value(portName + "/Unit5/CheckBox", false).toBool());

    multiSendUnit6->getLineEdit()->setText(multiSendSettings->value(portName + "/Unit6", "").toString());
    multiSendUnit6->getCheckBox()->setChecked(multiSendSettings->value(portName + "/Unit6/CheckBox", false).toBool());

    multiSendUnit7->getLineEdit()->setText(multiSendSettings->value(portName + "/Unit7", "").toString());
    multiSendUnit7->getCheckBox()->setChecked(multiSendSettings->value(portName + "/Unit7/CheckBox", false).toBool());

    multiSendUnit8->getLineEdit()->setText(multiSendSettings->value(portName + "/Unit8", "").toString());
    multiSendUnit8->getCheckBox()->setChecked(multiSendSettings->value(portName + "/Unit8/CheckBox", false).toBool());

    multiSendUnit9->getLineEdit()->setText(multiSendSettings->value(portName + "/Unit9", "").toString());
    multiSendUnit9->getCheckBox()->setChecked(multiSendSettings->value(portName + "/Unit9/CheckBox", false).toBool());

}

void SerialWidget::saveSettings()
{
    serialSettings->beginGroup(portName);
    serialSettings->setValue("BaudRate", baudRateComboBox->currentText());
    serialSettings->setValue("DataBits", dataBitsComboBox->currentText());
    serialSettings->setValue("Parity", parityComboBox->currentText());
    serialSettings->setValue("StopBits", stopBitsComboBox->currentText());
    serialSettings->setValue("FlowControl", flowControlComboBox->currentText());
    serialSettings->setValue("HexReceive", hexReceiveCheckBox->isChecked());
    serialSettings->setValue("HexSend", hexSendCheckBox->isChecked());
    serialSettings->setValue("NewRow", sendNewRowCheckbox->isChecked());
    serialSettings->endGroup();
    serialSettings->sync();

    multiSendSettings->beginGroup(portName);

    multiSendSettings->setValue("Unit0", multiSendUnit0->getLineText());
    multiSendSettings->setValue("Unit0/CheckBox", multiSendUnit0->getCheckBoxStatus());

    multiSendSettings->setValue("Unit1", multiSendUnit1->getLineText());
    multiSendSettings->setValue("Unit1/CheckBox", multiSendUnit1->getCheckBoxStatus());

    multiSendSettings->setValue("Unit2", multiSendUnit2->getLineText());
    multiSendSettings->setValue("Unit2/CheckBox", multiSendUnit2->getCheckBoxStatus());

    multiSendSettings->setValue("Unit3", multiSendUnit3->getLineText());
    multiSendSettings->setValue("Unit3/CheckBox", multiSendUnit3->getCheckBoxStatus());

    multiSendSettings->setValue("Unit4", multiSendUnit4->getLineText());
    multiSendSettings->setValue("Unit4/CheckBox", multiSendUnit4->getCheckBoxStatus());

    multiSendSettings->setValue("Unit5", multiSendUnit5->getLineText());
    multiSendSettings->setValue("Unit5/CheckBox", multiSendUnit5->getCheckBoxStatus());

    multiSendSettings->setValue("Unit6", multiSendUnit6->getLineText());
    multiSendSettings->setValue("Unit6/CheckBox", multiSendUnit6->getCheckBoxStatus());

    multiSendSettings->setValue("Unit7", multiSendUnit7->getLineText());
    multiSendSettings->setValue("Unit7/CheckBox", multiSendUnit7->getCheckBoxStatus());

    multiSendSettings->setValue("Unit8", multiSendUnit8->getLineText());
    multiSendSettings->setValue("Unit8/CheckBox", multiSendUnit8->getCheckBoxStatus());

    multiSendSettings->setValue("Unit9", multiSendUnit9->getLineText());
    multiSendSettings->setValue("Unit9/CheckBox", multiSendUnit9->getCheckBoxStatus());

    multiSendSettings->endGroup();
    multiSendSettings->sync();
}
