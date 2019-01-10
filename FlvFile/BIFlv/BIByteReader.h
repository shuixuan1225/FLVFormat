//
//  BIByteReader.h
//  FlvFile
//
//  Created by kirk on 2019/1/6.
//  Copyright © 2019 kirk. All rights reserved.
//

#ifndef BIByteReader_h
#define BIByteReader_h

#include <stdio.h>
#include "BIFlv.h"

#define readByte(inputbuffer,T) ({\
T des = 0; \
uint8_t length = sizeof(T);\
memcpy(&des, inputbuffer->data+inputbuffer->cursor, length);\
if(length ==2){\
des = CFSwapInt16BigToHost(des);\
}else if(length == 4){\
des = CFSwapInt32BigToHost(des);\
}else if(length == 8){\
des = CFSwapInt64BigToHost(des);\
}\
inputbuffer->cursor += length; \
des;\
})

//#define readByte(data,T) readByteWithLength(data,T,sizeof(T))

#define readByteWithLength(inputbuffer,T,readlength) ({\
T des = 0; \
uint8_t length = sizeof(T);\
void* p = (void*)&des;\
memcpy(p+(length-readlength), inputbuffer->data+inputbuffer->cursor, readlength);\
if(length ==2){\
des = CFSwapInt16BigToHost(des);\
}else if(length == 4){\
des = CFSwapInt32BigToHost(des);\
}else if(length == 8){\
des = CFSwapInt64BigToHost(des);\
}\
inputbuffer->cursor += readlength; \
des;\
})

//uint8_t readUint8(bi_data_buffer* data);
//
//uint16_t readUint16(bi_data_buffer* data,uint8_t length);
//
int16_t readInt16(bi_data_buffer* data,uint8_t length);
//
//uint32_t readUint32(bi_data_buffer* data,uint8_t length);
//
//uint64_t readUint64(bi_data_buffer* data);
//
double readDouble(bi_data_buffer* data);

char* readStringValue(bi_data_buffer* data,uint32_t length);

uint8_t* readDataValue(bi_data_buffer* data,uint32_t length);


/**
 读取一个NALUN 添加前缀 0x00 00 00 01

 @param data 输入buffer
 @param length NALUN 长度
 @return 带有前缀的NALUN
 */
uint8_t* readNaluDataValue(bi_data_buffer* data,uint32_t length);

#endif /* BIByteReader_h */
