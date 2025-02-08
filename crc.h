#ifndef CRC_H
#define CRC_H

#include <QObject>
#include <QStringList>
#include <cstdint>

class Crc
{
public:
    Crc();
    uint calculateCRC(const QByteArray &data, int formatIndex);

    QStringList crcList;
    enum class crcListIndex
    {
        CRC8 = 0,
        CRC16_CCITT,
        CRC16_XMODEM,
        CRC32,
        CRC32_MPEG2
    };

private:
    uint reverseBits(uint data, int nBits);
    uint crc8_Calculate(const QByteArray &data);
    uint crc16_CCITT_Calculate(const QByteArray &data);
    uint crc16_XMODEM_Calculate(const QByteArray &data);
    uint crc32_Calculate(const QByteArray &data);
    uint crc32_MPEG2_Calculate(const QByteArray &data);

};

#endif // CRC_H
