#include <stdlib.h>
#include <mqtt.h>


int mqttFrameToMQtt(unsigned char *mqttFrame, int len, MQtt *mqtt)
{
    int ret = MQTTERR_INVALID_ARGUMENTS;
    int offset = 0;
    int shift = 0;

    if(mqttFrame == NULL || mqtt == NULL || len<= 0)
    {
        goto OUT;
    }

    memset(mqtt, 0x00, sizeof(MQtt));
    mqtt->fixheader.byte1 = mqttFrame[offset];
    offset++;


    do{

        if(offset >= len)
        {
            ret = MQTTERR_INVALID_FRAME;
            goto OUT;
        }

        mqtt->fixheader.remainingLeng |= (mqttFrame[offset]&0x7F)<<shift;
        shift += 7;

        if(shift > 24)
        {
            ret = MQTTERR_INVALID_REMAININGLENGTH;
            goto OUT;
        }
    }while(mqttFrame[offset++]&0x80);

    switch(mqtt->fixheader.cpt)
    {
        case CPT_PUBLISH:
            if(mqtt->fixheader.flags & FLAG_QOS)
            {
                mqtt->varheader.Topic.len = (mqttFrame[offset++]<< 8) | mqttFrame[offset++];
                mqtt->varheader.Topic.name = mqttFrame+offset;
                mqtt->varheader.id.id_msb = mqttFrame[offset++];
                mqtt->varheader.id.id_lsb = mqttFrame[offset++];
                mqtt->payload.len = mqtt->fixheader.remainingLeng - 2;
            }
            else
            {
                mqtt->payload.len = mqtt->fixheader.remainingLeng;
            }
        break;
        case CPT_PUBACK:
        case CPT_PUBREC:
        case CPT_PUBREL:
        case CPT_PUBCOMP:
        case CPT_SUBSCRIBE:
        case CPT_SUBACK:
        case CPT_UNSUBSCRIBE:
        case CPT_UNSUBACK:
            mqtt->varheader.id.id_msb = mqttFrame[offset++];
            mqtt->varheader.id.id_lsb = mqttFrame[offset++];
            mqtt->payload.len = mqtt->fixheader.remainingLeng - 2;
        break;
        case CPT_CONNECT:
            mqtt->varheader.Protocol.len = (mqttFrame[offset++]<< 8) | mqttFrame[offset++];
            mqtt->varheader.Protocol.name = mqttFrame+offset;
            offset += mqtt->varheader.Protocol.len;
            mqtt->varheader.Protocol.level = mqttFrame[offset++];
            mqtt->varheader.connFlags = mqttFrame[offset++];
        break;
        case CPT_CONNACK:
        case CPT_PINGREQ:
        case CPT_PINGRESP:
        case CPT_DISCONNECT:
        default:
             mqtt->payload.len = mqtt->fixheader.remainingLeng;
        break;
    }

    if(offset >= len)
    {
        ret = MQTTERR_INVALID_FRAME;
        goto OUT;
    }

    if((mqtt->payload.len + offset) > len)
    {
        ret = MQTTERR_INVALID_FRAME;
        goto OUT;
    }

    mqtt->payload.data = mqttFrame+offset;

    OUT:
    return ret;
}


int HandleMQttMessage(unsigned char *mqttFrame, int len, MQTTHandler handle)
{
    int ret = -MQTTERR_INVALID_ARGUMENTS;
    MQtt mqtt;

    if(mqttFrame == NULL || len<= 0 || handle == NULL)
    {
        goto OUT;
    }


    ret = mqttFrameToMQtt(mqttFrame, len, &mqtt);

    if(ret != 0)
    {
        goto OUT;
    }

    ret = handle(&mqtt);
    //  switch(mqtt.fixheader.cpt)
    // {
    //     case CPT_PUBLISH:
    //         // ret = handler->publish_callback_handler(&mqtt);
    //     break;
    //     case CPT_PUBACK:

    //     break;
    //     case CPT_PUBREC:

    //     break;
    //     case CPT_PUBREL:

    //     break;
    //     case CPT_PUBCOMP:

    //     break;
    //     case CPT_SUBSCRIBE:

    //     break;
    //     case CPT_SUBACK:

    //     break;
    //     case CPT_UNSUBSCRIBE:

    //     break;
    //     case CPT_UNSUBACK:

    //     break;
    //     case CPT_CONNECT:

    //     break;
    //     case CPT_CONNACK:

    //     break;
    //     case CPT_PINGREQ:

    //     break;
    //     case CPT_PINGRESP:

    //     break;
    //     case CPT_DISCONNECT:

    //     break;
    //     default:
    //         ret = MQTTERR_UNKNOW_CPT;
    //     break;
    // }

    OUT:
    return ret;
}


#define DEFAULT_TIMEOUT   3

int MQtt_ReadFrame(void *frame, IOReader reader)
{
    int ret = MQTTERR_INVALID_ARGUMENTS;
    int offset = 0;
    unsigned char *pdata = (unsigned char *)frame;
    int remainingLen = 0;
    unsigned char shift = 0;

    if(frame == NULL)
    {
        goto OUT;
    }

    //第一步读取头部的控制字节
    ret = reader(pdata+offset, 1, DEFAULT_TIMEOUT);

    if(ret != 1)
    {
        ret = MQTTERR_READ_FAILED;
        goto OUT;
    }

    offset++;
    //第二步读取长度
    do{
        ret = reader(pdata+offset, 1, DEFAULT_TIMEOUT);

        if(ret != 1)
        {
            ret = MQTTERR_READ_FAILED;
            goto OUT;
        }

        remainingLen += (pdata[offset] & 0x7F) << shift;
        shift += 7;

        if(shift > 21)
        {
            ret = MQTTERR_INVALID_REMAININGLENGTH;
            goto OUT;
        }
    }while(pdata[offset++]&0x80);

    ret = reader(pdata+offset, remainingLen, DEFAULT_TIMEOUT*remainingLen);

    if(ret != remainingLen)
    {
        ret = MQTTERR_READ_FAILED;
        goto OUT;
    }

    offset += remainingLen;
    ret = offset;
    OUT:
    return ret;
}
