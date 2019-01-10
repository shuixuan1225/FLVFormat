//
//  BIMalloc.h
//  FlvFile
//
//  Created by kirk on 2019/1/4.
//  Copyright © 2019 kirk. All rights reserved.
//

#ifndef BIMalloc_h
#define BIMalloc_h

#include <stdio.h>

extern size_t total_alloc_size;

void*bi_malloc(uint32_t size);
void bi_free(void* des);


typedef struct bi_list_node{
    void* data;
    uint32_t size;
    struct bi_list_node* pre;
    struct bi_list_node* next;
}bi_list_node;

#endif /* BIMalloc_h */
