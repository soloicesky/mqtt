#ifndef _mqtt_h_
#define _mqtt_h_

typedef struct _mq_fix_header_{
   union {
       struct {
       unsigned char flags:4;
       unsigned char cpt:4;
       };
       unsigned char byte1;
   };

   int remainingLeng;
}MqFixHeader;

enum {
    CPT_BEGINRESERVED = 0,
    CPT_CONNECT,
    CPT_CONNACK,
    CPT_PUBLISH,
    CPT_PUBACK,
    CPT_PUBREC,
    CPT_PUBREL,
    CPT_PUBCOMP,
    CPT_SUBSCRIBE,
    CPT_SUBACK,
    CPT_UNSUBSCRIBE,
    CPT_UNSUBACK,
    CPT_PINGREQ,
    CPT_PINGRESP,
    CPT_DISCONNECT,
    CPT_ENDRESERVED = 15,
};

#define FLAG_DUP    0x80
#define FLAG_QOS    0x60
#define FLAG_RETAIN 0x01


typedef struct _mq_var_header_{
    struct{
        unsigned char id_msb;
        unsigned char id_lsb;
    }id;

    struct{
        int len;
        void *name;
        unsigned char level;
    }Protocol;

    union {
       struct {
       unsigned char Reserved:1;
       unsigned char CleanSession:1;
       unsigned char willFlag:1;
       unsigned char willQoS:2;
       unsigned char willRetain:1;
       unsigned char password:1;
       unsigned char username:1;
       };
       unsigned char connFlags;
    };

    struct {
        int len;
        void *name;
    }Topic; //topic name


}MqVarHeader;

typedef struct _mq_payload_{
     void *data;
     int len;
}MqPayload;


typedef struct _mqtt_{
    MqFixHeader fixheader;
    MqVarHeader varheader;
    MqPayload payload;
}MQtt;


typedef int (*MQTTHandler)(MQtt *mqtt);

enum{
    MQTTERR_INVALID_ARGUMENTS = -1,   //非法参数
    MQTTERR_INVALID_REMAININGLENGTH = -2018, //非法剩余长度
    MQTTERR_INVALID_FRAME = -2019, //非法帧数数
    MQTTERR_UNKNOW_CPT = -2020, //未识别控制码
    MQTTERR_READ_FAILED = -2021, //数据读取失败
};

/**
 * @name:IOReader
 * @Description:读数据的方法
 * @Param buf:存储数据的缓冲
 * @Param expLen:期望读取的长度
 * @Param timeoutMs:超时时间，单位为ms
 * @retval >0 读取到数据的长度，其它读取失败
 **/
typedef int (*IOReader)(void *buf, int expLen, int timeoutMs);

/**
 * @name:IOWriter
 * @Description:写数据的方法
 * @Param buf:需要写入的数据
 * @Param expLen:写入数据的长度
 * @Param timeoutMs:超时时间，单位为ms
 * @retval >0 吸入数据的长度，其它写入失败
 **/

typedef int (*IOWriter)(void *buf, int len, int timeoutMs);

/**
 * @name:Disconect
 * @Description:断开连接
 * @Param void
 * @retval none
 **/

typedef void (*Close)(void);


typedef int (*Run)(void);

typedef struct _mqtt_io_{
    IOReader reader;
    IOWriter writer;
    Close close;
}MQttIO;


typedef struct _mqtt_server_{
    MQttIO io;
    Run run;
}MQttServer;


#endif