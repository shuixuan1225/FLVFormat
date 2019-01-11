//
//  BIByteReader.c
//  FlvFile
//
//  Created by kirk on 2019/1/6.
//  Copyright © 2019 kirk. All rights reserved.
//

#include "BIByteReader.h"
#include <string.h>
#include "BIMalloc.h"
#import <CoreFoundation/CFByteOrder.h>


uint8_t readUint8(bi_data_buffer* data){
    uint8_t des = 0;
    memcpy(&des,data->data+data->cursor,1);
    data->cursor += 1;
    return des;
}

uint16_t readUint16(bi_data_buffer* data,uint8_t length){
    uint16_t buf = 0;
    void* p = (void*)&buf;
    memcpy(p+(sizeof(buf)-length), data->data+data->cursor, length);
    buf = CFSwapInt16BigToHost(buf);
    data->cursor += length;
    return buf;
}


int16_t readInt16(bi_data_buffer* data,uint8_t length){
    int16_t buf = 0;
    void* p = (void*)&buf;
    memcpy(p+(sizeof(buf)-length), data->data+data->cursor, length);
    buf = CFSwapInt16BigToHost(buf);
    data->cursor += length;
    return buf;
}

uint32_t readUint32(bi_data_buffer* data,uint8_t length){
    uint32_t buf = 0;
    void* p = (void*)&buf;
    memcpy(p+(sizeof(buf)-length), data->data+data->cursor, length);
    buf = CFSwapInt32BigToHost(buf);
    data->cursor += length;
    return buf;
}

uint64_t readUint64(bi_data_buffer* data){
    uint64_t buf = 0;
    void* p = (void*)&buf;
    memcpy(p, data->data+data->cursor, sizeof(uint64_t));
    buf = CFSwapInt64BigToHost(buf);
    data->cursor += sizeof(uint64_t);
    return buf;
}

//大小端转换
#define CONVERT_32(i, store, size) \
do{\
store = 0;\
for(int j = 0; j < size; j++){ \
store |=  (((i & (0xff << j * 8)) >> j * 8) & 0xff) << (((int)size - 1 - j) * 8); \
} \
}while(0)

//大小端不影响移位操作
#define CONVERT_64(i, store) \
do{ \
/*后面*/ \
uint32_t i1 = (uint32_t)i; \
uint32_t store1 = 0; \
CONVERT_32(i1, store1, 4); \
uint32_t *ipre = (uint32_t *)&store + 1; \
memcpy(ipre, &store1, 4); \
/*前面*/\
uint32_t i2 = i >> 32; \
uint32_t store2 = 0; \
CONVERT_32(i2, store2, 4); \
uint32_t *itail = (uint32_t *)&store; \
memcpy(itail, &store2, 4); \
}while(0)


double readDouble(bi_data_buffer* data){
    uint64_t buf = 0;
    void* p = (void*)&buf;
    memcpy(p, data->data+data->cursor, sizeof(double));
    double store = 0;
    CONVERT_64(buf,store);
    data->cursor += sizeof(uint64_t);
    return store;
}

uint8_t* readNaluDataValue(bi_data_buffer* data,uint32_t length){
    if (length<=0) {
        return NULL;
    }
    uint32_t t = CFSwapInt32HostToBig(length);//0x00000001;
    uint8_t* string = (uint8_t*)bi_malloc(sizeof(uint8_t)*length +4);
    memcpy(string,&t, 4);
    memcpy(string+4, data->data+data->cursor, length);
    data->cursor += length;
    return string;
}

uint8_t* readDataValue(bi_data_buffer* data,uint32_t length){
    if (length<=0) {
        return NULL;
    }
    uint8_t* string = (uint8_t*)bi_malloc(sizeof(uint8_t)*length);
    memcpy(string, data->data+data->cursor, length);
    data->cursor += length;
    return string;
}

char* readStringValue(bi_data_buffer* data,uint32_t length){
    if (length<=0) {
        return NULL;
    }
    char* string = (char*)bi_malloc(sizeof(char)*length);
    memcpy(string, data->data+data->cursor, length);
    data->cursor += length;
    return string;
}
