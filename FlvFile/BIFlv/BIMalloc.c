//
//  BIMalloc.c
//  FlvFile
//
//  Created by kirk on 2019/1/4.
//  Copyright © 2019 kirk. All rights reserved.
//

#include "BIMalloc.h"
#include <stdlib.h>
#include <string.h>

size_t total_alloc_size = 0;
bi_list_node* bi_alloc_list = NULL;

void insertPointer(void* pointer,uint32_t size){
    bi_list_node* node = (bi_list_node*)malloc(sizeof(bi_list_node));
    node->data = pointer;
    node->size = size;
    node->pre = bi_alloc_list;
    node->next = NULL;
    if (bi_alloc_list != NULL) {
        bi_alloc_list->next = node;
    }
    bi_alloc_list = node;
    total_alloc_size += size;
}

uint8_t deletePointer(void* pointer){
    bi_list_node* iterator = bi_alloc_list;
    while (iterator != NULL) {
        if (iterator->data == pointer) {
            total_alloc_size -= iterator->size;
            if (iterator->next != NULL) {
                iterator->next->pre = iterator->pre;
            }
            if (iterator->pre != NULL) {
                iterator->pre->next = iterator->next;
            }
            if(iterator == bi_alloc_list){
                bi_alloc_list = bi_alloc_list->pre;
            }
            free(iterator);
            return 1;
        }else{
            iterator = iterator->pre;
        }
    }
    
    return 0;
}

void*bi_malloc(uint32_t size){
    void* point = malloc(size);
    memset(point, 0, size);
    if (point == NULL) {
        //
    }else{
        insertPointer(point,size);
    }
    return point;
}

void bi_free(void* des){
    if (des == NULL) {
        return;
    }
    deletePointer(des);
    free(des);
}






