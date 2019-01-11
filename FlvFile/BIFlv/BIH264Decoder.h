//
//  BIH264Decoder.h
//  FlvFile
//
//  Created by kirk on 2019/1/9.
//  Copyright © 2019 kirk. All rights reserved.
//

#ifndef BIH264Decoder_h
#define BIH264Decoder_h

#include <stdio.h>
#import <VideoToolbox/VideoToolbox.h>
#include "BIFlv.h"


typedef struct bi_video_input_package {
    uint32_t    timestamp;//24 当前帧时戳，单位是毫秒。相对于FLV文件的第一个TAG时戳。第一个tag的时戳总是0。——不是时戳增量 dts
    uint8_t     timestamp_extends;//8  如果时戳大于0xFFFFFF，将会使用这个字节。这个字节是时戳的高8位，上面的三个字节是低24位。
    int32_t     cts;//SI24 cts偏移
    bi_flv_video_tag_type     type;//UB4 帧类型
    uint8_t* packetBuffer;
    uint32_t packetSize;
}bi_video_input_package;

typedef struct bi_video_output_package {
    uint32_t    timestamp;//24 当前帧时戳，单位是毫秒。相对于FLV文件的第一个TAG时戳。第一个tag的时戳总是0。——不是时戳增量 dts
    uint8_t     timestamp_extends;//8  如果时戳大于0xFFFFFF，将会使用这个字节。这个字节是时戳的高8位，上面的三个字节是低24位。
    int32_t     cts;//SI24 cts偏移
    bi_flv_video_tag_type     type;//UB4 帧类型
    CVPixelBufferRef bufferRef;
}bi_video_output_package;

bi_video_output_package* decoderInputTag(bi_video_input_package* inputPackage);

void EndVideoToolBox(void);

#endif /* BIH264Decoder_h */
