#include "serialmanager.h"

QByteArray serialManagerRecvData;

SerialManager::SerialManager(const QString &identifier, QObject *parent)
    : QObject(parent), portIdentifier(identifier)
{
    serialPort = new QSerialPort(this);
    connect(serialPort, &QSerialPort::readyRead, this, [this]() {
        QByteArray data = serialPort->readAll();
        emit dataReceived(data);
    });
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        timer->stop();
        emit dataReceived(serialManagerRecvData);
        serialManagerRecvData.clear();
    });
}

SerialManager::~SerialManager()
{
    disconnectSerialPort();
    delete serialPort;
}

bool SerialManager::connectSerialPort(const QString &portName, int baudRate, int dataBits, int parity, int stopBits, int flowControl)
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
        emit errorOccurred(tr("Failed to open serial port."));
        return false;
    }
}

void SerialManager::disconnectSerialPort()
{
    if (serialPort->isOpen()) {
        serialPort->close();
    }
}

bool SerialManager::isConnected() const
{
    return serialPort->isOpen();
}

void SerialManager::sendData(const QByteArray &data)
{
    if (serialPort->isOpen()) {
        if (serialPort->write(data) == -1) {
            emit errorOccurred(tr("Failed to send data."));
        }
    } else {
        emit errorOccurred(tr("Serial port is not connected."));
    }
}

QByteArray SerialManager::receiveData()
{
    QByteArray data;
    while (serialPort->bytesAvailable() > 0) {
        data.append(serialPort->readAll());
    }
    return data;
}

void SerialManager::onReadyRead(const QByteArray &data)
{
    timer->start(5);
    serialManagerRecvData.append(data);
}
