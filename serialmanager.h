#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager();

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

#endif // SERIALMANAGER_H
