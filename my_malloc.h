//
//  head_malloc.h
//  my_malloc.c
//
//  Created by Yijie Yan on 1/19/19.
//  Copyright © 2019 Yijie Yan. All rights reserved.
//

#ifndef head_malloc_h
#define head_malloc_h

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

//  size of meta storage block
#define META_SIZE sizeof(struct meta_block)

//  use linkedlist to implement memory space
//  use the linkedlist to define meta storage block
struct meta_block{
    size_t size;
    struct meta_block * next;
    int free;
};


//  header of memory space
struct meta_block * global=NULL;
//  end of the current memory space
struct meta_block * end=NULL;

//  First Fit malloc/free
void *ff_malloc(size_t size);
//  check the free space that can be used
struct meta_block * checkFree(size_t size);
//  alloc new space for use
struct meta_block * alloc(size_t size);
//  find the block before the block we want to free
struct meta_block * prev_block(void * ptr);
//  function to merge the newly freed region with any currently free adjacent regions
void merge(struct meta_block * ptr);
//  free memory space
void ff_free(void *ptr);
//  Best Fit malloc/free
void *bf_malloc(size_t size);
struct meta_block * checkBestFree(size_t size);
void bf_free(void *ptr);

unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in bytes


#endif /* head_malloc_h */
