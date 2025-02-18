#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QSerialPort>
#include <QByteArray>
#include <QTimer>

class SerialManager : public QObject
{
    Q_OBJECT

public:
    explicit SerialManager(const QString &identifier, QObject *parent = nullptr);
    ~SerialManager();

    bool connectSerialPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits, int flowControl);
    void disconnectSerialPort();
    bool isConnected() const;

    void sendData(const QByteArray &data);
    QByteArray receiveData();
    QString getIdentifier() const { return portIdentifier; }

signals:
    void dataReceived(const QByteArray &data);
    void errorOccurred(const QString &error);

private slots:
    void onReadyRead(const QByteArray &data);

private:
    QSerialPort *serialPort;
    QTimer *timer;
    QString portIdentifier;
};

#endif // SERIALMANAGER_H
