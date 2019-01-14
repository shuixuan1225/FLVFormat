//
//  BIFlvDecoder.h
//  FlvFile
//
//  Created by kirk on 2019/1/3.
//  Copyright © 2019 kirk. All rights reserved.
//

#ifndef BIFlvDecoder_h
#define BIFlvDecoder_h

#include <stdio.h>
#include "BIFlv.h"

#endif /* BIFlvDecoder_h */


bi_flv_header* readHeaderWithAlloc(bi_data_buffer* data);
void freeFlvHeader(bi_flv_header* header);

bi_flv_tag* readFlvTag(bi_data_buffer* data);
void freeFlvTag(bi_flv_tag* tag);

bi_flv_script_tag* readScriptTag(bi_data_buffer* data,uint32_t data_size);
void freeFlvScriptTag(bi_flv_script_tag* tag);

bi_flv_video_tag* readVideoFlvTag(bi_data_buffer* data,uint32_t data_size);
void freeFlvVideoTag(bi_flv_video_tag* tag);

bi_flv_audio_tag* readAudioFlvTag(bi_data_buffer* data,uint32_t data_size);
void freeFlvAudioTag(bi_flv_audio_tag* tag);


bi_rtmp_flv_tag* readRTMPFlvTag(bi_data_buffer* data);
void freeRTMPFlvTag(bi_rtmp_flv_tag* tag);
