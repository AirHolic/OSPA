#include "serialportmanager.h"

SerialPortManager::SerialPortManager(QObject *parent) : QObject(parent), serialPort(new QSerialPort(this))
{
    connect(serialPort, &QSerialPort::readyRead, this, [this]() {
        emit dataReceived(receiveData());
    });
}

SerialPortManager::~SerialPortManager()
{
    disconnectSerialPort();
    delete serialPort;
}

bool SerialPortManager::connectSerialPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits, int flowControl)
{
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(static_cast<QSerialPort::DataBits>(dataBits));
    serialPort->setParity(static_cast<QSerialPort::Parity>(parity));
    serialPort->setStopBits(static_cast<QSerialPort::StopBits>(stopBits));
    serialPort->setFlowControl(static_cast<QSerialPort::FlowControl>(flowControl));

    if (serialPort->open(QIODevice::ReadWrite)) {
        return true;
    } else {
        emit errorOccurred("Failed to open serial port.");
        return false;
    }
}

void SerialPortManager::disconnectSerialPort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
}

bool SerialPortManager::isConnected() const
{
    return serialPort->isOpen();
}

void SerialPortManager::sendData(const QByteArray &data)
{
    if (serialPort->isOpen()) {
        if (serialPort->write(data) == -1) {
            emit errorOccurred("Failed to send data.");
        }
    } else {
        emit errorOccurred("Serial port is not connected.");
    }
}

QByteArray SerialPortManager::receiveData()
{
    QByteArray data;
    while (serialPort->bytesAvailable() > 0) {
        data.append(serialPort->readAll());
    }
    return data;
}
