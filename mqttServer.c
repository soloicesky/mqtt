#include <stdlib.h>
#include "mqtt.h"

int MQtt_InitService(MQttServer *mqttServer ,MQttIO *io)
{
    int ret = MQTTERR_INVALID_ARGUMENTS;

    if(mqttServer == NULL|| io == NULL)
    {
        goto OUT;
    }

    mqttServer = io;
    OUT:
    return ret;
}



int Mqtt_RunService(MQttServer *sev, MQTTHandler handle)
{
    int ret = MQTTERR_INVALID_ARGUMENTS;
    unsigned char frame[4096];

    if(sev == NULL)
    {
        goto OUT;
    }

    do{

        ret = MQtt_ReadFrame(frame, sev->io.reader);

        if(ret <= 0)
        {
            goto OUT;
        }

        ret = HandleMQttMessage(frame, ret, handle);
        

    }while(1);

    OUT:
    return ret;
}