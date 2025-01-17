#include "ymodem.h"
#include <QDebug>

/* Ymodem 文件信息 */
QByteArray *ymodem_file_name = NULL;
int ymodem_file_size = 0;
QByteArray *ymodem_file_data = NULL;
int ymodem_file_data_len = 0;
int ymodem_file_data_frame_num = 0;
int ymodem_file_last_data_frame_head = 0;

uint8_t ymodem_packet_data[1024 + 5];



Ymodem::Ymodem()
{

}

void Ymodem::ymodemRecvData(uint8_t *recv_data, uint16_t len)
{
    memcpy(ymodem_recv_data, recv_data, len);
    YmodemAckHandle();
}

void Ymodem::YmodemSendFileReady(QByteArray &file_name, int file_size)
{    
    //memcpy(ymodem_file_name, file_name, file_name.size());
    ymodem_file_name = &file_name;
    ymodem_file_size = file_size;
    qDebug() << *ymodem_file_name << ymodem_file_size << endl;
}

void Ymodem::YmodemSendFileData(QByteArray &data, int len)
{
    ymodem_file_data = &data;
    ymodem_file_data_len = len;
    qDebug() << ymodem_file_data_len << endl;
    //YmodemSendFile();
}

void Ymodem::YmodemSendFileStart()
{
    int i = 0;
    qDebug() << "i" << endl;
    while (YmodemSendStateMachine() == 1)
    {
        qDebug() << "t" << endl;
        YmodemAckHandle();
    }
}

uint16_t Ymodem::YmodemCRC16Table(uint8_t *data, int len)
{
    uint16_t crc = 0;
    while(len--) 
    {
        crc = (crc << 8) ^ crc16_xmodem_table[(crc >> 8 ^ *data++) & 0xff];
    }
    
    return(crc);
}

uint16_t Ymodem::YmodemCRC16(uint8_t *data, int len)
{
    uint16_t crc = 0;
    while(len--)
    {
        crc ^= (unsigned short)(*data++) << 8;
        for (int i = 0; i < 8; ++i)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    
    return crc;
}

uint8_t Ymodem::YmodemSendDataPack(uint8_t *cache)
{
    cache[0] = ymodem_data_frame_t->ymodem_head;
    cache[1] = ymodem_data_frame_t->ymodem_num;
    cache[2] = ~(ymodem_data_frame_t->ymodem_num);
    memcpy(cache+3, ymodem_data_frame_t->ymodem_data_cache, ymodem_data_frame_t->ymodem_data_cache_len);
    cache[3+ymodem_data_frame_t->ymodem_data_cache_len] = (ymodem_data_frame_t->ymodem_crc >> 8) & 0xff;
    cache[4+ymodem_data_frame_t->ymodem_data_cache_len] = ymodem_data_frame_t->ymodem_crc & 0xff;
    return 0;
}

uint8_t Ymodem::YmodemSendData()
{
    uint8_t cache[1024];
    YmodemSendDataPack(cache);
    emit ymodemSendData(cache, ymodem_data_frame_t->ymodem_data_cache_len);
    return 0;
}

/**
 * @todo 无文件需要发送时，需补充特定空帧
 */
uint8_t Ymodem::YmodemSendStartFrame()
{
    ymodem_data_frame_t->ymodem_head = YMODEM_SOH;
    ymodem_data_frame_t->ymodem_num = 0;
    ymodem_data_frame_t->ymodem_data_cache_len = 128;
    if(ymodem_file_size > 128)
    {
        ymodem_file_data_frame_num = ymodem_file_size / 1024 + 1;
        if(ymodem_file_size % 1024 == 0)
        {
            ymodem_file_data_frame_num--;
        }
        if((ymodem_file_size % 1024) > 128)
        {
            ymodem_file_last_data_frame_head = YMODEM_STX;
        }
        else if((ymodem_file_size % 1024) < 128)
        {
            ymodem_file_last_data_frame_head = YMODEM_SOH;
        }
    }
    else if(ymodem_file_size < 128)
    {
        ymodem_file_data_frame_num = 1;
        ymodem_file_last_data_frame_head = YMODEM_SOH;
    }
    QByteArray *cache;
    cache = new QByteArray(*ymodem_file_name);
    cache->append('\x00').append(ymodem_file_size).append('\x00');
    if (cache->size() < 128)
    {
        memset(ymodem_data_frame_t->ymodem_data_cache,0x00,1024);
        memcpy(ymodem_data_frame_t->ymodem_data_cache, cache->constData(), cache->size());
        ymodem_data_frame_t->ymodem_crc = YmodemCRC16(ymodem_data_frame_t->ymodem_data_cache, ymodem_data_frame_t->ymodem_data_cache_len);
    }
    else{
        delete cache;
        return 1;
    }

    delete cache;
    return 0;

}

/**
 * @todo 未完成,需补充最后一帧剩余空间0a，1024和128的情况，剩余需传输数据大小判断
 */

uint8_t Ymodem::YmodemSendNewFrame()
{
    ymodem_data_frame_t->ymodem_num++;
    int start_flag = 0;
    int end_flag = 0;
    if(ymodem_data_frame_t->ymodem_num < ymodem_file_data_frame_num)
    {
        ymodem_data_frame_t->ymodem_head = YMODEM_STX;
        ymodem_data_frame_t->ymodem_data_cache_len = 1024;
        start_flag = (ymodem_data_frame_t->ymodem_num - 1) * 1024;
        end_flag = start_flag + 1024;
    }
    else if (ymodem_data_frame_t->ymodem_num >= ymodem_file_data_frame_num)
    {
        ymodem_data_frame_t->ymodem_head = ymodem_file_last_data_frame_head;
        ymodem_data_frame_t->ymodem_data_cache_len = ymodem_file_size % 1024 > 128 ? 1024 : 128;
        start_flag = (ymodem_data_frame_t->ymodem_num - 1) * 1024;
        end_flag = start_flag + ymodem_file_size % 1024;
    }
    

    if (ymodem_file_data_len > 0)
    {
        if(ymodem_data_frame_t->ymodem_num < ymodem_file_data_frame_num)
            memset(ymodem_data_frame_t->ymodem_data_cache,0x00,1024);
        else if (ymodem_data_frame_t->ymodem_num >= ymodem_file_data_frame_num) {
            memset(ymodem_data_frame_t->ymodem_data_cache, 0x0A, 1024);
        }
        memcpy(ymodem_data_frame_t->ymodem_data_cache, ymodem_file_data->mid(start_flag, end_flag).constData(), ymodem_data_frame_t->ymodem_data_cache_len);
        ymodem_data_frame_t->ymodem_crc = YmodemCRC16(ymodem_data_frame_t->ymodem_data_cache, ymodem_data_frame_t->ymodem_data_cache_len);
    }
    else
    {
        return 1;
    }
    return 0;
}

uint8_t Ymodem::YmodemSendEndFrame()
{
    uint8_t data[2];
    data[1] = YMODEM_EOT;
    emit ymodemSendData(data, 1);
    return 0;
}

uint8_t Ymodem::YmodemSendFinishFrame()
{
    ymodem_data_frame_t->ymodem_head = YMODEM_SOH;
    ymodem_data_frame_t->ymodem_num = 0;
    ymodem_data_frame_t->ymodem_data_cache_len = 128;
    memset(ymodem_data_frame_t->ymodem_data_cache,0x00,1024);
    ymodem_data_frame_t->ymodem_crc = YmodemCRC16(ymodem_data_frame_t->ymodem_data_cache, ymodem_data_frame_t->ymodem_data_cache_len);
    return 0;
}

uint8_t Ymodem::YmodemAckHandle()
{
    switch (ymodem_recv_data[0])
    {
    case YMODEM_ACK:
        ymodem_ack_state = YmodemAckState::YMODEM_STATE_ACK;
        break;
    case YMODEM_NAK:
        ymodem_ack_state = YmodemAckState::YMODEM_STATE_NAK;
        break;
    case YMODEM_C:
        ymodem_ack_state = YmodemAckState::YMODEM_STATE_C;
        break;
    case YMODEM_Abort1:
        ymodem_ack_state = YmodemAckState::YMODEM_STATE_ABORT1;
        break;
    case YMODEM_Abort2:
        ymodem_ack_state = YmodemAckState::YMODEM_STATE_ABORT2;
        break;
    default:
        ymodem_ack_state = YmodemAckState::YMODEM_STATE_ACK_IDLE;
        break;
    }
    return 0;
}

uint8_t Ymodem::YmodemSendStateMachine()
{
    switch (ymodem_send_state)
    {
        case YmodemSendState::YMODEM_STATE_IDLE:
            break;
        case YmodemSendState::YMODEM_STATE_READY_SEND://准备发送，等待接收方发送C
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_C)
            {
                YmodemSendStartFrame();
                YmodemSendData();
                ymodem_send_state = YmodemSendState::YMODEM_STATE_SEND_START;
                break;
            }
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_ACK_CANCEL)
            {
                ymodem_send_state = YmodemSendState::YMODEM_STATE_SEND_CANCEL;
                break;
            }
            break;
        case YmodemSendState::YMODEM_STATE_SEND_START://等待接收方C，发送文件第一条数据
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_NAK)
            {
                YmodemSendData();
                break;
            }
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_ACK)
                break;
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_C)
            {
                YmodemSendNewFrame();
                YmodemSendData();
                ymodem_send_state = YmodemSendState::YMODEM_STATE_SEND_DATA;
                break;
            }
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_ACK_CANCEL)
            {
                ymodem_send_state = YmodemSendState::YMODEM_STATE_SEND_CANCEL;
                break;
            }
            break;
        case YmodemSendState::YMODEM_STATE_SEND_DATA:
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_NAK)
            {
                YmodemSendData();
                break;
            }
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_ACK)
            {
                if(YmodemSendNewFrame() != 0)
                {
                    ymodem_send_state = YmodemSendState::YMODEM_STATE_SEND_END;
                    break;
                }
                else
                {        
                    YmodemSendData();
                    break;
                }
            }
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_ACK_CANCEL)
            {
                ymodem_send_state = YmodemSendState::YMODEM_STATE_SEND_CANCEL;
                break;
            }
            break;
        case YmodemSendState::YMODEM_STATE_SEND_END:
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_ACK)
            {
                YmodemSendEndFrame();
                ymodem_send_state = YmodemSendState::YMODEM_STATE_SEND_FINISH;
                break;
            }
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_NAK)
            {
                YmodemSendEndFrame();
                break;
            }
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_ACK_CANCEL)
            {
                ymodem_send_state = YmodemSendState::YMODEM_STATE_SEND_CANCEL;
                break;
            }
            break;
        case YmodemSendState::YMODEM_STATE_SEND_ERROR:
            break;
        case YmodemSendState::YMODEM_STATE_SEND_CANCEL:
            break;
        case YmodemSendState::YMODEM_STATE_SEND_WAIT_ACK:
            while (YmodemAckHandle() == 3)
            {
                /* code */
            }
            break;
        case YmodemSendState::YMODEM_STATE_SEND_FINISH:
            if(ymodem_ack_state == YmodemAckState::YMODEM_STATE_C)
            {
                YmodemSendFinishFrame();
                YmodemSendData();
            }
            break;
        default:
            break;
    }
    ymodem_ack_state = YmodemAckState::YMODEM_STATE_ACK_IDLE;
    if(YmodemSendState::YMODEM_STATE_SEND_FINISH == ymodem_send_state)
    {
        ymodem_send_state = YmodemSendState::YMODEM_STATE_IDLE;
        return 0;
    }
    return 1;
}
