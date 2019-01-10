//
//  BIFlvDecoder.c
//  FlvFile
//
//  Created by kirk on 2019/1/3.
//  Copyright © 2019 kirk. All rights reserved.
//

#include "BIFlvDecoder.h"
#include <string.h>
#include "BIMalloc.h"
#import <CoreFoundation/CFByteOrder.h>
#include "BIByteReader.h"

bi_script_data_value* readScriptDataValue(bi_data_buffer* data,uint32_t origin_cursor,uint32_t data_size);

#pragma mark header

bi_flv_header* readHeaderWithAlloc(bi_data_buffer* data){
    bi_flv_header* header = (bi_flv_header*)bi_malloc(sizeof(bi_flv_header));
    header->signature = readByteWithLength(data,uint32_t,3);
    header->version = readByte(data,uint8_t);
    header->audio_video = readByte(data,uint8_t);
    header->header_size = readByteWithLength(data,uint32_t,4);
    return header;
}
#pragma mark header free
void freeFlvHeader(bi_flv_header* header){
    bi_free(header);
}

#pragma mark script

bi_script_string_value* readScriptStringDataValue(bi_data_buffer* data){
    bi_script_string_value* stringData = (bi_script_string_value*)bi_malloc(sizeof(bi_script_string_value));
    stringData->string_length = readByte(data,uint16_t);
    stringData->string_data = readStringValue(data,stringData->string_length);
    return stringData;
}

bi_script_data_object_property_value* readPropertyObjectValue(bi_data_buffer* data,uint32_t origin_cursor,uint32_t data_size){
    bi_script_data_object_property_value* propertyValue = (bi_script_data_object_property_value*)bi_malloc(sizeof(bi_script_data_object_property_value));
    propertyValue->property_name = readScriptStringDataValue(data);
//    if (0 == strcmp(propertyValue->property_name->string_data, "filepositions")) {
//        propertyValue->property_data = readScriptKeyframesArrayValue(data);
//    }else{
        propertyValue->property_data = readScriptDataValue(data,origin_cursor,data_size);
//    }
    return propertyValue;
}

bi_script_object_properties_list_node* readObjectDataList(bi_data_buffer* data,uint32_t origin_cursor,uint32_t data_size){
    bi_script_object_properties_list_node* header = NULL;
    bi_script_object_properties_list_node* cursor = NULL;

    while (data->cursor<(data_size + origin_cursor -8)) {
        bi_script_object_properties_list_node* next = (bi_script_object_properties_list_node*)bi_malloc(sizeof(bi_script_object_properties_list_node));
        next->pre = NULL;
        next->next = NULL;
        next->property_value = readPropertyObjectValue(data,origin_cursor,data_size);
        if (header == NULL) {
            header = next;
            cursor = next;
        }else{
            cursor->next = next;
            next->pre = cursor;
            cursor = next;
        }
    }
    return header;
}


bi_script_object_value* readScriptObjectDataValue(bi_data_buffer* data,uint32_t origin_cursor,uint32_t data_size){
    bi_script_object_value* object_value = (bi_script_object_value*)bi_malloc(sizeof(bi_script_object_value));
    object_value->object_propertie_list = readObjectDataList(data,origin_cursor,data_size);
    object_value->object_end_marker = readByteWithLength(data,uint32_t,3);
    return object_value;
}

bi_script_object_properties_list_node* readDataArray(bi_data_buffer* data,uint32_t num,uint32_t origin_cursor,uint32_t data_size){
    bi_script_object_properties_list_node* header = NULL;
    bi_script_object_properties_list_node* cursor = NULL;
    for (int i = 0; i<num; i++) {
        bi_script_object_properties_list_node* next = (bi_script_object_properties_list_node*)bi_malloc(sizeof(bi_script_object_properties_list_node));
        next->pre = NULL;
        next->next = NULL;
        next->property_value = readPropertyObjectValue(data,origin_cursor,data_size);
        if (header == NULL) {
            header = next;
            cursor = next;
        }else{
            cursor->next = next;
            next->pre = cursor;
            cursor = next;
        }
    }
    return header;
}

bi_script_data_ecma_array_value* readScriptArrayDataValue(bi_data_buffer* data,uint32_t origin_cursor,uint32_t data_size){
    bi_script_data_ecma_array_value* arrayData = (bi_script_data_ecma_array_value*)bi_malloc(sizeof(bi_script_data_ecma_array_value));
    arrayData->ecma_array_length = readByte(data,uint32_t);
    arrayData->variables_list = readDataArray(data,arrayData->ecma_array_length,origin_cursor,data_size);
    arrayData->object_end_marker = readByteWithLength(data,uint32_t,3);
    return arrayData;
}

bi_script_strict_object_list_node* readStrictDataArray(bi_data_buffer* data,uint32_t num,uint32_t origin_cursor,uint32_t data_size){
    bi_script_strict_object_list_node* header = NULL;
    bi_script_strict_object_list_node* cursor = NULL;
    for (int i = 0; i<num; i++) {
        bi_script_strict_object_list_node* next = (bi_script_strict_object_list_node*)bi_malloc(sizeof(bi_script_strict_object_list_node));
        next->pre = NULL;
        next->next = NULL;
        next->strict_data = readScriptDataValue(data,origin_cursor,data_size);
        if (header == NULL) {
            header = next;
            cursor = next;
        }else{
            cursor->next = next;
            next->pre = cursor;
            cursor = next;
        }
    }
    return header;
}

bi_script_data_strict_array_value* readScriptStrictArrayDataValue(bi_data_buffer* data,uint32_t origin_cursor,uint32_t data_size){
    bi_script_data_strict_array_value* arrayData = (bi_script_data_strict_array_value*)bi_malloc(sizeof(bi_script_data_strict_array_value));
    arrayData->strict_array_length = readByte(data,uint32_t);
//    arrayData->array_buffer = readDataValue(data, arrayData->strict_array_length*(sizeof(uint64_t)));
    arrayData->strict_list = readStrictDataArray(data,arrayData->strict_array_length,origin_cursor,data_size);
//    arrayData->object_end_marker = readByteWithLength(data,uint32_t,3);
    return arrayData;
}
bi_script_data_date_value* readScriptDateDataValue(bi_data_buffer* data){
    bi_script_data_date_value* arrayData = (bi_script_data_date_value*)bi_malloc(sizeof(bi_script_data_date_value));
    arrayData->date_time = readByte(data,double);
    arrayData->local_datetime_offset = readInt16(data,16);
    return arrayData;
}
bi_script_long_string_value* readScriptLongStringDataValue(bi_data_buffer* data){
    bi_script_long_string_value* stringData = (bi_script_long_string_value*)bi_malloc(sizeof(bi_script_long_string_value));
    stringData->string_length = readByte(data,uint32_t);
    stringData->string_data = readStringValue(data,stringData->string_length);
    return stringData;
}


bi_script_data_value* readScriptDataValue(bi_data_buffer* data,uint32_t origin_cursor,uint32_t data_size){
    bi_script_data_value* dataValue = (bi_script_data_value*)bi_malloc(sizeof(bi_script_data_value));
    dataValue->type = readByte(data,uint8_t);
    if (dataValue->type == script_type_double) {
//        dataValue->double_value = readByte(data,double);
        dataValue->double_value = readDouble(data);
    }else if (dataValue->type == script_type_ui8){
        dataValue->ui8_value = readByte(data,uint8_t);
    }else if (dataValue->type == script_type_string){
        dataValue->string_value = readScriptStringDataValue(data);
    }else if (dataValue->type == script_type_object){
        dataValue->object_value = readScriptObjectDataValue(data,origin_cursor,data_size);
    }else if (dataValue->type == script_type_ui16){
        dataValue->ui16_value = readByte(data,uint16_t);
    }else if (dataValue->type == script_type_ecmaarray){
        dataValue->array_value = readScriptArrayDataValue(data,origin_cursor,data_size);
    }else if (dataValue->type == script_type_strictarray){
        dataValue->strict_array_value = readScriptStrictArrayDataValue(data,origin_cursor,data_size);
    }
    else if (dataValue->type == script_type_date){
        dataValue->script_data_value = readScriptDateDataValue(data);
    }
    else if (dataValue->type == script_type_longstring){
        dataValue->long_string_value = readScriptLongStringDataValue(data);
    }
    return dataValue;
}


bi_flv_script_tag* readScriptTag(bi_data_buffer* data,uint32_t data_size){
    bi_flv_script_tag* scriptTag = (bi_flv_script_tag*)bi_malloc(sizeof(bi_flv_script_tag));
    uint32_t cursor = data->cursor;
    scriptTag->name = readScriptDataValue(data,cursor,data_size);
    scriptTag->value = readScriptDataValue(data,cursor,data_size);
    return scriptTag;
}

#pragma mark Script free

void freeScriptDataValue(bi_script_data_value* dataValue);

void freeScriptStringValue(bi_script_string_value* string){
    if (string == NULL) {
        return;
    }
    bi_free(string->string_data);
    bi_free(string);
}
void freeScriptLongStringValue(bi_script_long_string_value* string){
    if (string == NULL) {
        return;
    }
    bi_free(string->string_data);
    bi_free(string);
}

void freePropertieValue(bi_script_data_object_property_value* property_value){
    if (property_value == NULL) {
        return;
    }
    freeScriptDataValue(property_value->property_data);
    freeScriptStringValue(property_value->property_name);
    
    bi_free(property_value);
}

void freePropertiesList(bi_script_object_properties_list_node*  object_propertie_list){
    while (object_propertie_list != NULL) {
        bi_script_object_properties_list_node* cursor = object_propertie_list->next;
        freePropertieValue(object_propertie_list->property_value);
        bi_free(object_propertie_list);
        object_propertie_list = cursor;
    }
}

void freeScriptObjectValue(bi_script_object_value* object){
    if (object == NULL) {
        return;
    }
    freePropertiesList(object->object_propertie_list);
    bi_free(object);
}

void freeScriptArrayValue(bi_script_data_ecma_array_value* array){
    if (array == NULL) {
        return;
    }
    freePropertiesList(array->variables_list);
    bi_free(array);
}
void freeScriptStrictList(bi_script_strict_object_list_node* strict_object_list){
    while (strict_object_list != NULL) {
        bi_script_strict_object_list_node* cursor = strict_object_list->next;
        freeScriptDataValue(strict_object_list->strict_data);
        bi_free(strict_object_list);
        strict_object_list = cursor;
    }
}
void freeScriptStrictArrayValue(bi_script_data_strict_array_value* strictArray){
    if (strictArray == NULL) {
        return;
    }
    freeScriptStrictList(strictArray->strict_list);
    bi_free(strictArray);
}
void freeScriptDateValue(bi_script_data_date_value* date){
    if (date == NULL) {
        return;
    }
    bi_free(date);
}

void freeScriptDataValue(bi_script_data_value* dataValue){
    if (dataValue == NULL) {
        return;
    }
    if (dataValue->type == script_type_string){
        freeScriptStringValue(dataValue->string_value);
    }else if (dataValue->type == script_type_object){
        freeScriptObjectValue(dataValue->object_value);
    }else if (dataValue->type == script_type_ecmaarray){
        freeScriptArrayValue(dataValue->array_value);
    }else if (dataValue->type == script_type_strictarray){
        freeScriptStrictArrayValue(dataValue->strict_array_value);
    }
    else if (dataValue->type == script_type_date){
        freeScriptDateValue(dataValue->script_data_value);
    }
    else if (dataValue->type == script_type_longstring){
        freeScriptLongStringValue(dataValue->long_string_value);
    }
    bi_free(dataValue);
}

void freeFlvScriptTag(bi_flv_script_tag* tag){
    if (tag == NULL) {
        return;
    }
    freeScriptDataValue(tag->name);
    freeScriptDataValue(tag->value);
    bi_free(tag);
}

#pragma mark  video

bi_flv_video_NALU_data* readNALUData(bi_data_buffer* data){
    bi_flv_video_NALU_data* nalu_data = (bi_flv_video_NALU_data*)bi_malloc(sizeof(bi_flv_video_NALU_data));
    nalu_data->length = readByte(data,uint32_t);
    nalu_data->data = readNaluDataValue(data,nalu_data->length);
    nalu_data->length += 4;
    return nalu_data;
}

bi_flv_video_NALU_list_node* readNALUList(bi_data_buffer* data,uint32_t origin_cursor,uint32_t data_size){
    bi_flv_video_NALU_list_node* header = NULL;
    bi_flv_video_NALU_list_node* cursor = NULL;
    
    while (data->cursor<(data_size+origin_cursor)) {
        bi_flv_video_NALU_list_node* next = (bi_flv_video_NALU_list_node*)bi_malloc(sizeof(bi_flv_video_NALU_list_node));
        next->pre = NULL;
        next->next = NULL;
        next->nalu_data = readNALUData(data);
        if (header == NULL) {
            header = next;
            cursor = next;
        }else{
            cursor->next = next;
            next->pre = cursor;
            cursor = next;
        }
    }
    return header;
}

bi_flv_video_SPS_or_PPS_data* readSPSorPPSData(bi_data_buffer* data){
    bi_flv_video_SPS_or_PPS_data* sps_or_pps_data = (bi_flv_video_SPS_or_PPS_data*)bi_malloc(sizeof(bi_flv_video_SPS_or_PPS_data));
    sps_or_pps_data->length = readByte(data,uint16_t);
    sps_or_pps_data->data = readNaluDataValue(data,sps_or_pps_data->length);
    sps_or_pps_data->length += 4;
    return sps_or_pps_data;
}

bi_flv_video_SPS_PPS_list_node* readSPSPPSList(bi_data_buffer* data,uint8_t num){
    bi_flv_video_SPS_PPS_list_node* header = NULL;
    bi_flv_video_SPS_PPS_list_node* cursor = NULL;
    for (int i = 0; i<num; i++) {
        bi_flv_video_SPS_PPS_list_node* next = (bi_flv_video_SPS_PPS_list_node*)bi_malloc(sizeof(bi_flv_video_SPS_PPS_list_node));
        next->pre = NULL;
        next->next = NULL;
        next->sps_or_pps_data = readSPSorPPSData(data);
        if (header == NULL) {
            header = next;
            cursor = next;
        }else{
            cursor->next = next;
            next->pre = cursor;
            cursor = next;
        }
    }
    return header;
}

bi_avc_decoder_configuration_record* readVideoConfigurationRecord(bi_data_buffer* data,uint32_t data_size){
    bi_avc_decoder_configuration_record* record = (bi_avc_decoder_configuration_record*)bi_malloc(sizeof(bi_avc_decoder_configuration_record));
    record->configuration_version = readByte(data,uint8_t);
    record->avc_profile_indication = readByte(data,uint8_t);
    record->profile_compatibility = readByte(data,uint8_t);
    record->avc_level_indication = readByte(data,uint8_t);
    record->length_size_minus_one = readByte(data,uint8_t);
    record->length_size_minus_one = (record->length_size_minus_one)&0x0c;
    
    //sps
    record->num_of_sequence_parameter_sets = readByte(data,uint8_t);
    record->num_of_sequence_parameter_sets = (record->num_of_sequence_parameter_sets)&0x1f;
    record->sps_list = readSPSPPSList(data,record->num_of_sequence_parameter_sets);
    //pps
    record->num_of_picture_parameter_sets = readByte(data,uint8_t);
    record->pps_list = readSPSPPSList(data,record->num_of_sequence_parameter_sets);
    return record;
}

bi_flv_video_avc_package_data* readVideoAVCPackage(bi_data_buffer* data,uint32_t origin_cursor,uint32_t data_size){
    bi_flv_video_avc_package_data* package = (bi_flv_video_avc_package_data*)bi_malloc(sizeof(bi_flv_video_avc_package_data));
    package->avc_type = readByte(data,uint8_t);
    package->cts = readByteWithLength(data,int32_t,3);;
    if (package->avc_type == video_avc_sequenceheader) {
        package->configuration_record = readVideoConfigurationRecord(data,data_size-sizeof(uint32_t));
    }else{
        package->video_data = readNALUList(data,origin_cursor,data_size);
    }
    return package;
}
bi_flv_video_tag* readVideoFlvTag(bi_data_buffer* data,uint32_t data_size){
    bi_flv_video_tag* flvTag = (bi_flv_video_tag*)bi_malloc(sizeof(bi_flv_video_tag));
    
    uint32_t cursor = data->cursor;
    
    uint8_t t = readByte(data,uint8_t);
    flvTag->type = (t&0xf0)/16;
    flvTag->encode_id = t&0x0f;
    if (flvTag->encode_id == video_encoderid_AVC) {
        flvTag->avc_package = readVideoAVCPackage(data,cursor,data_size);
    }
    return flvTag;
}

#pragma mark video free

void freeSPSPPSData(bi_flv_video_SPS_or_PPS_data* sData){
    if (sData == NULL) {
        return;
    }
    bi_free(sData->data);
    bi_free(sData);
}

void freeSPSPPSList(bi_flv_video_SPS_PPS_list_node* list){
    while (list != NULL) {
        bi_flv_video_SPS_PPS_list_node* cursor = list->next;
        freeSPSPPSData(list->sps_or_pps_data);
        bi_free(list);
        list = cursor;
    }
}

void freeFlvAvcDecoderConfigurationRecord(bi_avc_decoder_configuration_record*    configuration_record){
    if (configuration_record == NULL) {
        return;
    }
    freeSPSPPSList(configuration_record->sps_list);
    freeSPSPPSList(configuration_record->pps_list);
    bi_free(configuration_record);
}


void freeNALUData(bi_flv_video_NALU_data* naluData){
    if (naluData == NULL) {
        return;
    }
    bi_free(naluData->data);
    bi_free(naluData);
}
void freeFlvNALUList(bi_flv_video_NALU_list_node*  list){
    while (list != NULL) {
        bi_flv_video_NALU_list_node* cursor = list->next;
        freeNALUData(list->nalu_data);
        bi_free(list);
        list = cursor;
    }
}

void freeFlvVideoAVCPackage(bi_flv_video_avc_package_data* package){
    if (package == NULL) {
        return;
    }
    if (package->avc_type == video_avc_sequenceheader) {
        freeFlvAvcDecoderConfigurationRecord(package->configuration_record);
    }else{
        freeFlvNALUList(package->video_data);
    }
    bi_free(package);
}

void freeFlvVideoTag(bi_flv_video_tag* flvTag){
    if (flvTag == NULL) {
        return;
    }
    if (flvTag->encode_id == video_encoderid_AVC) {
        freeFlvVideoAVCPackage(flvTag->avc_package);
    }
    bi_free(flvTag);
}

/*
 typedef struct bi_flv_audio_tag {
 uint8_t     format;//UB4 音频格式
 uint8_t     sampling_rate;//UB2 采样率
 uint8_t     rate_size;//UB1 采样大小
 uint8_t     channel;//UB1 声道
 void*       data; //声音数据
 }bi_flv_audio_tag;
 */
#pragma mark audio
//TODO
bi_flv_audio_tag* readAudioFlvTag(bi_data_buffer* data,uint32_t data_size){
     bi_flv_audio_tag* flvTag = (bi_flv_audio_tag*)bi_malloc(sizeof(bi_flv_audio_tag));
    uint8_t t = readByte(data,uint8_t);
    flvTag->data = readDataValue(data,data_size-sizeof(uint8_t));
    return flvTag;
}

#pragma mark audio free
void freeFlvAudioTag(bi_flv_audio_tag* tag){
    if (tag != NULL) {
        bi_free(tag->data);
    }
    bi_free(tag);
}

#pragma mark readFlvTag

bi_flv_tag* readFlvTag(bi_data_buffer* data){
    uint32_t previousTagSize = readByte(data,uint32_t);
//    printf("previousTagSize = %d\n",previousTagSize);
    uint32_t header_length = 11;
    if ((data->length - data->cursor) < header_length) {
        return NULL;
    }
    bi_flv_tag* flvTag = (bi_flv_tag*)bi_malloc(sizeof(bi_flv_tag));
    flvTag->type = readByte(data,uint8_t);
    flvTag->data_size = readByteWithLength(data,uint32_t,3);
    if ((data->length - data->cursor) < (flvTag->data_size+header_length)) {
        freeFlvTag(flvTag);
        return NULL;
    }
    flvTag->timestamp = readByteWithLength(data,uint32_t,3);
    flvTag->timestamp_extends = readByte(data,uint8_t);
    flvTag->stream_id = readByteWithLength(data,uint32_t,3);
    if (flvTag->type == flv_tag_type_audio) {
        flvTag->audio = readAudioFlvTag(data,flvTag->data_size);
    }else if (flvTag->type == flv_tag_type_video) {
        flvTag->video = readVideoFlvTag(data,flvTag->data_size);
    }if (flvTag->type == flv_tag_type_script) {
        flvTag->script = readScriptTag(data,flvTag->data_size);
    }
    return flvTag;
}

void freeFlvTag(bi_flv_tag* flvTag){
    if (flvTag->type == flv_tag_type_audio) {
        freeFlvAudioTag(flvTag->audio);
    }else if (flvTag->type == flv_tag_type_video) {
        freeFlvVideoTag(flvTag->video);
    }if (flvTag->type == flv_tag_type_script) {
        freeFlvScriptTag(flvTag->script);
    }
    bi_free(flvTag);
}
