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

class SerialPortWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SerialPortWidget(const QString &portName, QWidget *parent = nullptr);
    ~SerialPortWidget();

signals:
    void closeRequested(int index);

private slots:
    void toggleConnection(); // 合并连接和断开按钮的功能
    void sendData();
    void receiveData();
    void clearReceiveArea(); // 清空接收区

private:
    QSerialPort *serialPort;
    QSettings *settings;
    QTextEdit *receiveTextEdit;
    QTextEdit *sendTextEdit;
    QComboBox *baudRateComboBox;
    QComboBox *dataBitsComboBox;
    QComboBox *parityComboBox;
    QComboBox *stopBitsComboBox;
    QComboBox *flowControlComboBox;
    QPushButton *connectButton; // 合并后的连接/断开按钮
    QPushButton *sendButton; // 发送按钮
    QPushButton *clearReceiveButton; // 清空接收区按钮
    QCheckBox *hexReceiveCheckBox;
    QCheckBox *hexSendCheckBox;
    QLabel *statusLabel; // 底部栏，显示收发字节数

    QString portName;
    int sentBytes; // 已发送字节数
    int receivedBytes; // 已接收字节数

    void connectSerialPort(); // 连接串口
    void disconnectSerialPort(); // 断开串口
    void loadcomSettings(); // 加载串口设置
    void savecomSettings(); // 保存串口设置
    void updateStatusLabel(); // 更新底部栏的字节数显示
};

#endif // SERIALPORTWIDGET_H