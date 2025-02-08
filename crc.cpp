#include "crc.h"
#include <QDebug>

Crc::Crc()
{
    //crcList = new QStringList;
    crcList  << "CRC8"
             << "CRC16/CCITT" << "CRC16/XMODEM"
             << "CRC32" << "CRC32/MPEG2";
}

uint Crc::calculateCRC(const QByteArray &data, int formatIndex)
{
    switch (formatIndex)
    {
        case static_cast<int>(crcListIndex::CRC8):
            return crc8_Calculate(data);
        case static_cast<int>(crcListIndex::CRC16_CCITT):
            return crc16_CCITT_Calculate(data);
        case static_cast<int>(crcListIndex::CRC16_XMODEM):
            return crc16_XMODEM_Calculate(data);
        case static_cast<int>(crcListIndex::CRC32):
            return crc32_Calculate(data);
        case static_cast<int>(crcListIndex::CRC32_MPEG2):
            return crc32_MPEG2_Calculate(data);
        default:
            return 0;
    }
    //return crc;
}

uint Crc::crc8_Calculate(const QByteArray &data)
{
    uint crc = 0x00; // CRC初始值
    uint polynomial = 0x07; // CRC多项式，对应x^8 + x^2 + x + 1

    for (const auto &byte : data) {
        crc ^= static_cast<uint>(byte);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial; // 若最高位为1，则进行异或操作
            } else {
                crc <<= 1; // 否则左移一位
            }
        }
    }
    crc = crc & 0xFF; // 返回计算出的CRC值
    return crc;
}

uint Crc::crc16_CCITT_Calculate(const QByteArray &data)
{
    uint crc = 0x0000; // 使用crc16_XMODEM的初始值
    uint polynomial = 0x1021; // CRC多项式

    for (const auto &byte : data) {
        // 对输入字节进行反转（REFIN）——反转8位
        unsigned char b = static_cast<unsigned char>(byte);
        uint rb = reverseBits(b, 8);
        crc ^= rb << 8;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ polynomial;
            else
                crc <<= 1;
        }
    }

    // 对输出数据进行反转（REFOUT），共16位
    uint out = reverseBits(crc, 16);
    return out & 0xFFFF;
}

uint Crc::crc16_XMODEM_Calculate(const QByteArray &data)
{
    uint crc = 0x0000; // CRC初始值
    uint polynomial = 0x1021; // CRC多项式

    for (const auto &byte : data) {
        crc ^= static_cast<uint>(byte) << 8; // 异或操作
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ polynomial; // 若最高位为1，则进行异或操作
            } else {
                crc <<= 1; // 否则左移一位
            }
        }
    }

    return crc & 0xFFFF; // 返回计算出的CRC值（两个字节）
}

uint Crc::crc32_Calculate(const QByteArray &data)
{
    uint crc = 0xFFFFFFFF; // CRC初始值
    uint polynomial = 0x04C11DB7; // CRC多项式

    for (const auto &byte : data) {
        // 对输入字节进行反转（REFIN）——反转8位
        unsigned char b = static_cast<unsigned char>(byte);
        uint rb = reverseBits(b, 8);
        crc ^= rb << 24;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x80000000)
                crc = (crc << 1) ^ polynomial;
            else
                crc <<= 1;
        }
    }

    // 对输出数据进行反转（REFOUT），共32位，再进行 XOROUT
    uint out = reverseBits(crc, 32);
    return out ^ 0xFFFFFFFF;
}

uint Crc::crc32_MPEG2_Calculate(const QByteArray &data)
{
    uint crc = 0xFFFFFFFF; // CRC初始值为全1
    uint polynomial = 0x04C11DB7; // CRC32/MPEG-2多项式

    for (const auto &byte : data) {
        crc ^= static_cast<uint>(byte) << 24; // 异或操作
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ polynomial; // 若最高位为1，则进行异或操作
            } else {
                crc <<= 1; // 否则左移一位
            }
        }
    }
    crc = crc & 0xFFFFFFFF;

    return crc; // 返回计算出的CRC值（四个字节）
}

// 新增辅助函数：通用的位反转函数，nBits指定反转的位数
uint Crc::reverseBits(uint data, int nBits)
{
    uint reversed = 0;
    for (int i = 0; i < nBits; i++) {
        if (data & (1U << i))
            reversed |= (1U << (nBits - 1 - i));
    }
    return reversed;
}
