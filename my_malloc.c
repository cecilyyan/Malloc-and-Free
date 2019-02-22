//
//  ECE 650 HW1
//  my_malloc.c
//
//  Created by Yijie Yan (yy226) on 1/19/19.
//  Copyright © 2019 Yijie Yan. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "my_malloc.h"

struct meta_block * checkFree(size_t size){
    struct meta_block * curr=global;
    //  scan through the linked list memory space to find the free space
    //  if not found, then keep looping
    while((curr!=NULL)&&((curr->free==0)||(curr->size<size))){
        //  set the end block to curr
        curr=curr->next;
    }
    //  curr point to the fited space block when while loop ends
    return curr;
}
struct meta_block * alloc(size_t size){
    struct meta_block * block=NULL;
    block=sbrk(0);
    void * pointer=sbrk(size+META_SIZE);
    //  fail to call sbrk function, return NULL
    if(pointer==(void *)-1){
        return NULL;
    }
    //  when it is not initialization then end is not NULL
    if(end!=NULL){
        end->next=block;
    }
    end=block;
    end->size=size;
    end->free=0;
    end->next=NULL;
    return end;
}
void *ff_malloc(size_t size){
    if(size<=0) return NULL;
    struct meta_block * block=NULL;
    //  if it is the first time to execuate malloc
    if(global==NULL){
        block=alloc(size);
        //  check error when call alloc function
        if(block==NULL){
            return NULL;
        }
        //  set global as block
        global=block;
    }else{
        //  when it's not the first time to execuate malloc
        //  find the free space to use
        block=checkFree(size);
        if(block==NULL){
            //  no free space to use
            //  need alloc new space
            block=alloc(size);
        }else{
            //  enough space to use
            block->free=0;
        }
    }
    
    return (block+1);
}


struct meta_block * prev_block(void * ptr){
    struct meta_block * prev=NULL;
    if(global==NULL||ptr==global) return prev;
    struct meta_block * curr=global;
    while(curr->next!=ptr){
        curr=curr->next;
    }
    
    prev=curr;
    
    return prev;
    
}
void merge(struct meta_block  * ptr){
    //  find the block before ptr
    struct meta_block * prev=prev_block(ptr);
    //  find the block after ptr
    struct meta_block * next=ptr->next;
    if(prev==NULL&&next==NULL){
        //  linkedlist memory space is NULL
        return;
    }else if((prev==NULL||ptr==global)){
        //  when ptr points at header of linkedlist
        //  check and merge ptr and next
        if(next->free==1){
            //  if can merge
            ptr->size=ptr->size+next->size;
            ptr->next=next->next;
            ptr->free=1;
        }
    }else if((next==NULL || ptr==end)){
        //  ptr points at end of linkedlist
        //  check and merge prev and ptr
        if(prev->free==1){
            //  if can merge
            prev->size=prev->size+ptr->size;
            prev->next=ptr->next;
            prev->free=1;
        }
    }else{
       
        //  check and merge prev, ptr, next
        if(prev->free==1&&next->free==1){
            //  merge three blocks
            prev->size=prev->size+ptr->size+next->size;
            prev->next=next->next;
            prev->free=1;
        }else if(prev->free==1&&next->free==0){
            //  merge prev and ptr
            prev->size=prev->size+ptr->size;
            prev->next=ptr->next;
            prev->free=1;
        }else if(prev->free==0&&next->free==1){
            //  merge ptr and next
            ptr->size=ptr->size+next->size;
            ptr->next=next->next;
            ptr->free=1;
        }else{
            //  can not merge
        }
    }
    
    
}
void ff_free(void *ptr){
    //  set curr poniter points at the same block as ptr
    if(ptr==NULL) return;
    
    struct meta_block * to_free=ptr;
    to_free->free=1;
    merge(to_free);
  
}
struct meta_block * checkBestFree(size_t size){
    //  use curr pointer to go through whole linkedlist
    struct meta_block * curr=NULL;
    if (size<=0) return curr;
    curr=global;
    //  find the best block
    size_t bestSize=0;
    struct meta_block * block=NULL;
    while(curr->next!=NULL){
        if(curr->size>=size){
            //  if there is free block that can be used
            //  find the optimal block
            if(curr->size<bestSize){
                bestSize=curr->size;
                block=curr;
            }
        }
        curr=curr->next;
    }
    
    return block;
}
void *bf_malloc(size_t size){
    if(size<=0) return NULL;
    struct meta_block * block=NULL;
    //  if it is the first time to execuate malloc
    if(global==NULL){
        block=alloc(size);
        //  check error when call alloc function
        if(block==NULL){
            return NULL;
        }
        //  set global as block
        global=block;
    }else{
        //  when it's not the first time to execuate malloc
        //  find the free space to use
        block=checkBestFree(size);
        if(block==NULL){
            //  no free space to use
            //  need alloc new space
            block=alloc(size);
        }else{
            //  enough space to use
            block->free=0;
        }
    }
    
    return (block+1);
}
void bf_free(void *ptr){
    if(ptr==NULL) return;
    ff_free(ptr);
}

unsigned long get_data_segment_size(){
    struct meta_block * curr=global;
    unsigned long res=0;
    while(curr->next!=NULL){
        res+=curr->size;
    }
    return res;
}
unsigned long get_data_segment_free_space_size(){
    struct meta_block * curr=global;
    unsigned long res=0;
    while(curr->next!=NULL){
        if(curr->free==1){
            res+=curr->size;
            
        }
    }
    return res;
}
