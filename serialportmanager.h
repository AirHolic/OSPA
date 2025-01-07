#ifndef SERIALPORTMANAGER_H
#define SERIALPORTMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>

class SerialPortManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialPortManager(QObject *parent = nullptr);
    ~SerialPortManager();

    bool connectSerialPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits, int flowControl);
    void disconnectSerialPort();
    bool isConnected() const;

    void sendData(const QByteArray &data);
    QByteArray receiveData();

signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);

private:
    QSerialPort *serialPort;
};

#endif // SERIALPORTMANAGER_H
