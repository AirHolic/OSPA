#ifndef SERIALPORTWIDGET_H
#define SERIALPORTWIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSettings>

class QTextEdit;
class QComboBox;
class QPushButton;
class QCheckBox;

class SerialPortWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SerialPortWidget(const QString &portName, QWidget *parent = nullptr);
    ~SerialPortWidget();

signals:
    void closeRequested(int index);

private slots:
    void connectSerialPort();
    void disconnectSerialPort();
    void sendData();
    void receiveData();


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
    QPushButton *connectButton;
    QPushButton *disconnectButton;
    QPushButton *sendButton; // 发送按钮
    QCheckBox *hexReceiveCheckBox;
    QCheckBox *hexSendCheckBox;

    QString portName;

    void loadcomSettings();
    void savecomSettings();
};

#endif // SERIALPORTWIDGET_H
