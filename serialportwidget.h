#ifndef SERIALPORTWIDGET_H
#define SERIALPORTWIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSettings>

class QTextEdit;
class QComboBox;
class QPushButton;
class QCheckBox;
class QLabel;
class QShortcut;

class SerialPortWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SerialPortWidget(const QString &portName, QWidget *parent = nullptr);
    ~SerialPortWidget();

signals:
    void closeRequested(int index); // 请求关闭信号

private slots:
    void toggleConnection(); // 切换连接状态
    void sendData();         // 发送数据
    void receiveData();      // 接收数据
    void clearReceiveArea(); // 清空接收区
    void openSearchDialog(); // 打开搜索对话框

private:
    void initUI();           // 初始化 UI
    void initConnections();  // 初始化信号槽连接
    void loadSettings();     // 加载配置
    void saveSettings();     // 保存配置
    void updateStatusLabel(); // 更新状态栏
    void logMessage(const QString &message); // 日志记录
    void connectSerialPort(); // 连接串口
    void disconnectSerialPort(); // 断开串口

    QSerialPort *serialPort; // 串口对象
    QSettings *settings;     // 配置文件对象
    QString portName;        // 串口名称

    // UI 元素
    QTextEdit *receiveTextEdit;     // 接收区
    QTextEdit *sendTextEdit;        // 发送区
    QComboBox *baudRateComboBox;    // 波特率
    QComboBox *dataBitsComboBox;    // 数据位
    QComboBox *parityComboBox;      // 校验位
    QComboBox *stopBitsComboBox;    // 停止位
    QComboBox *flowControlComboBox; // 流控制
    QPushButton *connectButton;     // 连接/断开按钮
    QPushButton *sendButton;        // 发送按钮
    QPushButton *clearReceiveButton; // 清空接收区按钮
    QCheckBox *hexReceiveCheckBox;  // HEX 接收复选框
    QCheckBox *hexSendCheckBox;     // HEX 发送复选框
    QLabel *statusLabel;            // 状态栏

    // 统计信息
    int sentBytes;     // 已发送字节数
    int receivedBytes; // 已接收字节数

    // 快捷键
    QShortcut *searchShortcut; // Ctrl+F 快捷键
};

#endif // SERIALPORTWIDGET_H
