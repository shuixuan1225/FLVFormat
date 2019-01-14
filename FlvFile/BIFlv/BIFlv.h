//
//  BIFlv.h
//  FlvFile
//
//  Created by kirk on 2019/1/3.
//  Copyright © 2019 kirk. All rights reserved.
//

#ifndef BIFlv_h
#define BIFlv_h

#include <stdio.h>

#define BI_READ_LENGTH 1024

struct bi_script_data_value;

typedef struct bi_data_buffer{
    void* data;
    uint32_t    length;
    uint32_t    cursor;//读取位置
}bi_data_buffer;

#pragma mark header

typedef struct bi_flv_header {
    uint32_t    signature;//24 签名
    uint8_t     version;//8 flv版本
    uint8_t     audio_video;//8  //5 保留字段0 1 是否有音频 1保留字段0 1是否有视频
    uint32_t    header_size;//32 文件头大小
}bi_flv_header;

#pragma mark audio

typedef struct bi_flv_audio_tag {
    uint8_t     format;//UB4 音频格式
    uint8_t     sampling_rate;//UB2 采样率
    uint8_t     rate_size;//UB1 采样大小
    uint8_t     channel;//UB1 声道
    void*       data; //声音数据
}bi_flv_audio_tag;


#pragma mark video

typedef struct bi_flv_video_SPS_or_PPS_data{
    int16_t     length;
    uint8_t*    data;
}bi_flv_video_SPS_or_PPS_data;

typedef struct bi_flv_video_SPS_PPS_list_node{
    bi_flv_video_SPS_or_PPS_data*   sps_or_pps_data;
    struct bi_flv_video_SPS_PPS_list_node*  pre;
    struct bi_flv_video_SPS_PPS_list_node*  next;
}bi_flv_video_SPS_PPS_list_node;

typedef struct bi_avc_decoder_configuration_record{
    uint8_t     configuration_version;//UB8
    uint8_t     avc_profile_indication;//UB8
    uint8_t     profile_compatibility;//UB8
    uint8_t     avc_level_indication;//UB8
    uint8_t     length_size_minus_one;//UB2 前6位reserved
    uint8_t     num_of_sequence_parameter_sets;//UB5 前3位reserved
    bi_flv_video_SPS_PPS_list_node* sps_list;
    uint8_t     num_of_picture_parameter_sets;//UB8
    bi_flv_video_SPS_PPS_list_node* pps_list;
}bi_avc_decoder_configuration_record;

typedef struct bi_flv_video_NALU_data{
    int32_t     length;//
    uint8_t*    data;
}bi_flv_video_NALU_data;

typedef struct bi_flv_video_NALU_list_node{
    bi_flv_video_NALU_data*   nalu_data;
    struct bi_flv_video_NALU_list_node*  pre;
    struct bi_flv_video_NALU_list_node*  next;
}bi_flv_video_NALU_list_node;

//AVCVIDEOPACKET

typedef enum bi_flv_video_avc_type{
    video_avc_sequenceheader = 0,//解码器配置，sps，pps
    video_avc_nalu = 1,
    video_avc_endofsequence = 2
}bi_flv_video_avc_type;

typedef struct bi_flv_video_avc_package_data{
    bi_flv_video_avc_type     avc_type;   //UB4 帧类型 0 = AVC sequence header
                            //1 = AVC NALU
                            //2 = AVC end of sequence (lower level NALU sequence ender is not required or supported)
    int32_t     cts;//SI24 cts偏移
    union {
        bi_avc_decoder_configuration_record*    configuration_record;
        bi_flv_video_NALU_list_node*            video_data;
    };
}bi_flv_video_avc_package_data;

typedef enum bi_flv_video_tag_type{
    video_tag_keyframe = 1,
    video_tag_interframe = 2,
    video_tag_disposableinterframe = 3,
    video_tag_generatedkeyframe = 4,//(reserved for server use only)
    video_tag_videoinfo_commandframe = 5
}bi_flv_video_tag_type;

typedef enum bi_flv_video_encoderid{
    video_encoderid_JPEG = 1,
    video_encoderid_screenvideo = 3,
    video_encoderid_On2VP6 = 4,
    video_encoderid_On2VP6_with_alpha_channel  = 5,//(reserved for server use only)
    video_encoderid_screenvideo_version2 = 6,
    video_encoderid_AVC = 7
}bi_flv_video_encoder_id;

typedef struct bi_flv_video_tag {
    bi_flv_video_tag_type     type;//UB4 帧类型
    bi_flv_video_encoder_id     encode_id;//UB4 编码ID
    union {
        //type = 7
        bi_flv_video_avc_package_data*       avc_package; //视频数据
    };
}bi_flv_video_tag;


#pragma mark script

typedef struct bi_script_string_value {
    uint16_t    string_length;
    char*       string_data;
}bi_script_string_value;

typedef struct bi_script_long_string_value {
    uint32_t    string_length;
    char*       string_data;
}bi_script_long_string_value;

typedef struct bi_script_data_object_property_value {
    bi_script_string_value*             property_name;
    struct bi_script_data_value*        property_data;
}bi_script_data_object_property_value;

typedef struct bi_script_object_properties_list_node{
    bi_script_data_object_property_value*   property_value;
    struct bi_script_object_properties_list_node*  pre;
    struct bi_script_object_properties_list_node*  next;
}bi_script_object_properties_list_node;

typedef struct bi_script_strict_object_list_node{
    struct bi_script_data_value*        strict_data;
    struct bi_script_strict_object_list_node*  pre;
    struct bi_script_strict_object_list_node*  next;
}bi_script_strict_object_list_node;

typedef struct bi_script_object_value {
    bi_script_object_properties_list_node*  object_propertie_list;
    uint32_t                                object_end_marker;//24: UI8 [3] Shall be 0, 0, 9
}bi_script_object_value;

typedef struct bi_script_data_ecma_array_value{
    uint32_t                                ecma_array_length;
    bi_script_object_properties_list_node*  variables_list;
    uint32_t                                object_end_marker;//24: UI8 [3] Shall be 0, 0, 9
}bi_script_data_ecma_array_value;

typedef struct bi_script_data_strict_array_value{
    uint32_t                                strict_array_length;
    bi_script_strict_object_list_node*  strict_list;
//    uint32_t                                object_end_marker;//24: UI8 [3] Shall be 0, 0, 9
}bi_script_data_strict_array_value;


typedef struct bi_script_data_date_value{
    double                                  date_time;//Number of milliseconds since Jan 1, 1970 UTC.
    int16_t                                 local_datetime_offset;//Local time offset in minutes from UTC. For time zones located west of Greenwich, UK, this value is a negative number. Time zones east of Greenwich, UK, are positive.
}bi_script_data_date_value;


/*
 IF Type == 0 DOUBLE
 IF Type == 1 UI8
 IF Type == 2 SCRIPTDATASTRING
 IF Type == 3 SCRIPTDATAOBJECT
 IF Type == 7 UI16
 IF Type == 8 SCRIPTDATAECMAARRAY
 IF Type == 10 SCRIPTDATASTRICTARRAY
 IF Type == 11 SCRIPTDATADATE
 IF Type == 12 SCRIPTDATALONGSTRING
 */

typedef enum bi_flv_script_data_type{
    script_type_double = 0,
    script_type_ui8 = 1,
    script_type_string = 2,
    script_type_object = 3,
    script_type_ui16 = 7,
    script_type_ecmaarray = 8,
    script_type_strictarray = 10,
    script_type_date = 11,
    script_type_longstring = 12
}bi_flv_script_data_type;

typedef struct bi_script_data_value {
    bi_flv_script_data_type     type;//uint8 帧类型 一般总是0x02，表示字符串
    union {
        //Type == 0
        double                              double_value;
        //Type == 1
        uint8_t                             ui8_value;
        //Type == 2
        bi_script_string_value*             string_value;
        //Type == 3
        bi_script_object_value*             object_value;
        //Type == 7
        uint16_t                            ui16_value;
        //Type == 8
        bi_script_data_ecma_array_value*    array_value;
        //Type == 10
        bi_script_data_strict_array_value*  strict_array_value;
        //Type == 11
        bi_script_data_date_value*          script_data_value;
        //Type == 12
        bi_script_long_string_value*        long_string_value;
    };
}bi_script_data_value;

typedef struct bi_flv_script_tag {
    bi_script_data_value*     name;// 第一个amf包 String 一般总为“onMetaData”
    bi_script_data_value*     value;//第二个amf包  ECMA array 视频信息数组
}bi_flv_script_tag;



typedef enum bi_flv_tag_type{
    flv_tag_type_audio = 8,
    flv_tag_type_video = 9,
    flv_tag_type_script = 18,
}bi_flv_tag_type;

typedef struct bi_flv_tag {
    uint8_t     reserved;//UB [2] Reserved for FMS, should be 0  不处理
    uint8_t     filter;//UB [1] Indicates if packets are filtered. 0 = No pre-processing required.\
    1 = Pre-processing (such as decryption) of the packet is required before it can be rendered.\
    Shall be 0 in unencrypted files, and 1 for encrypted tags. See Annex F. FLV Encryption for the use of filters.不处理
    bi_flv_tag_type     type;//UB [5] 8: audio 9: video 18: script data——这里是一些描述信息。 all others: reserved其他所有值未使用。
    
    uint32_t    data_size;//24 数据区的大小，不包括包头。包头总大小是11个字节。
    uint32_t    timestamp;//24 当前帧时戳，单位是毫秒。相对于FLV文件的第一个TAG时戳。第一个tag的时戳总是0。——不是时戳增量
    uint8_t     timestamp_extends;//8  如果时戳大于0xFFFFFF，将会使用这个字节。这个字节是时戳的高8位，上面的三个字节是低24位。
    uint32_t    stream_id;//24 流ID
    
    union {
        bi_flv_audio_tag*   audio;
        bi_flv_video_tag*   video;
        bi_flv_script_tag*  script;
    };
    
}bi_flv_tag;



typedef struct bi_rtmp_flv_tag {
    bi_flv_tag_type     type;
    uint32_t    data_size;//24 数据区的大小，不包括包头。包头总大小是11个字节。
     uint32_t    timestamp;
    void*       data;
    
}bi_rtmp_flv_tag;

#endif /* BIFlv_h */
