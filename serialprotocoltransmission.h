#ifndef SERIALPROTOCOLTRANSMISSION_H
#define SERIALPROTOCOLTRANSMISSION_H

#include <ymodem.h>
#include <QWidget>
#include <QPushButton>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSerialPort>

class SerialWidget;
class SerialManager;

class SerialProtocolTransmission : public QWidget
{
    Q_OBJECT

public:
    explicit SerialProtocolTransmission(QWidget *parent = nullptr);
    ~SerialProtocolTransmission();
    SerialWidget *serialWidget;

signals:
    int protcocolRecvData(uint8_t *data, uint16_t len);

private slots:
    void openFile();
    void startYModemTransfer();
    void protocolHexReceiveData(const QByteArray &data);
    int protcocolSendData(uint8_t *data, uint16_t len);
private:
    QString filePath;        // 文件路径
    QProgressBar *progressBar; // 进度条
    QPushButton *openFileButton; // 打开文件按钮
    QPushButton *startButton;    // 开始传输按钮
    QLabel *fileLabel;           // 文件路径显示

    Ymodem *ymodem;

    void protocolHexSendData(const QByteArray &data);
    void initUI();
    void initConnections();
};

#endif // SERIALPROTOCOLTRANSMISSION_H
