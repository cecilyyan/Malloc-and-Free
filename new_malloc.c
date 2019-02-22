//
//  my_malloc.c
//
//  Copyright © 2019 Yijie Yan. All rights reserved.
//

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "my_malloc.h"



void split(struct meta_block * pos, size_t size){
    if(pos->size <= size+META_SIZE) return;
    size_t oldsize=pos->size;
    struct meta_block * new_block=NULL;
    new_block=(struct meta_block *)((void*)(pos+1)+size);
    new_block->size=oldsize-size-META_SIZE-META_SIZE;
    new_block->free=1;
    new_block->next=NULL;
    new_block->prev=NULL;
    new_block->next_mem=NULL;
    new_block->prev_mem=NULL;
    
    
    pos->free=0;
    pos->size=size;
    
    addfree(new_block);
    addmem(new_block);
}

void addfree(struct meta_block * ptr){
    if(freelist==NULL){
        freelist=ptr;
        return;
    }
    ptr->next=freelist;
    ptr->prev=NULL;
    
    if(freelist!=NULL){
        freelist->prev=ptr;
    }
    freelist=ptr;
}

void addmem(struct meta_block * ptr){
    if(global==NULL&&end==NULL){
        ptr->prev_mem=NULL;
        ptr->next_mem=NULL;
        global=ptr;
        end=ptr;
        return;
    }
    if(global==end && global->next_mem==NULL){
        //only one node
        global->next_mem=ptr;
        ptr->prev_mem=global;
        ptr->next_mem=NULL;
        end=ptr;
        return;
    }
    ptr->prev_mem=end;
    end->next_mem=ptr;
    ptr->next_mem=NULL;
    end=ptr;
}

struct meta_block * alloc(size_t size){
    size_t size_alloc=(size_t)(512 * 512);
    if(size+META_SIZE > size_alloc){
        size_alloc = size+META_SIZE;
    }
    void * pointer =sbrk(0);
    if(pointer==(void *)-1){
        return NULL;
    }
    sbrk(size_alloc);
    struct meta_block * block=(struct meta_block *)pointer;
    block->next=NULL;
    block->prev=NULL;
    block->free=0;
    block->size=size;
    block->next_mem=NULL;
    block->prev_mem=NULL;
    
    return block;
}

void init(size_t size){
    if(global!=NULL || end !=NULL) return;
    global=alloc(size);
    global->size=global->size-META_SIZE;
    global->free=0;
    global->next=NULL;
    global->prev=NULL;
    global->prev_mem=NULL;
    global->next_mem=NULL;
    
    end=global;
    
    
}
struct meta_block * checkFree(size_t size){
    struct meta_block * block=NULL;
    if(size<=0) return block;
    if(freelist==NULL) return NULL;
    
    
    struct meta_block * curr=freelist;
    while(curr!=NULL){
        if(curr->size >=size+META_SIZE){
            //  find the fitted one
            block=curr;
            
            return block;
        }
        curr=curr->next;
    }
    //  can not find the fitted free block
    return NULL;
}

void rmfreelist(struct meta_block * ptr){
    if(freelist==NULL || ptr==NULL) return;
    if(freelist==ptr){
        freelist=ptr->next;
    }
    if(ptr->next!=NULL){
        ptr->next->prev=ptr->prev;
    }
    if(ptr->prev!=NULL){
        ptr->prev->next=ptr->next;
    }
    ptr->prev=NULL;
    ptr->next=NULL;
    return;
    
    
}

void rmmemlist(struct meta_block * ptr){
    if(global==NULL || ptr==NULL) return;
    if(global==ptr){
        global=ptr->next_mem;
    }
    if(ptr->next_mem!=NULL){
        ptr->next_mem->prev_mem=ptr->prev_mem;
    }
    if(ptr->prev_mem!=NULL){
        if(ptr==end){
            end=ptr->prev_mem;
        }
        ptr->prev_mem->next_mem=ptr->next_mem;
    }
    end->next_mem=NULL;
    ptr->prev_mem=NULL;
    ptr->next_mem=NULL;
    
    return;
    
}

void do_alloc(size_t size){
    struct meta_block * block=NULL;
    
    //  check the fitted free block
    block=checkFree(size);
    //  if not found fitted block
    if(block==NULL){
        block=alloc(size);
        addmem(block);
        split(block, size);
        return;
    }
    //if found
    block->free=0;
    rmfreelist(block);
    
    split(block, size);
    
}

void *ff_malloc(size_t size){
    struct meta_block * block=NULL;
    if(size<=0) return block;
    
    //  first time to call malloc
    if (global == NULL){
        init(size);
        split(global, size);
        return (global+1);
    }
    
    do_alloc(size);
    //  return the block we newly alloc
    return (end+1);
}


struct meta_block * merge(struct meta_block * first, struct meta_block * second){
    size_t new_size=first->size+second->size;
    first->size=new_size;
    first->free=1;
    rmfreelist(second);
    rmmemlist(second);
    return first;
    
}

void ff_free(void *ptr){
    //  convert the address
    struct meta_block * toFree=(struct meta_block * )ptr - 1;
    toFree->free=1;
    if(freelist==NULL){
        freelist=toFree;
    }else{
        //  add to freelist
        toFree->next=freelist->next;
        freelist->next=toFree;
        toFree->prev=freelist;
        
    }
    if(toFree->prev_mem!=NULL && toFree->prev_mem->free==1){
        //  merge previous node
        toFree=merge(toFree->prev_mem, toFree);
    }
    if(toFree->next_mem!=NULL && toFree->next_mem->free==1){
        //  merge next node
        toFree=merge(toFree, toFree->next_mem);
    }
    
    
}




struct meta_block * checkBestFree(size_t size){
    struct meta_block * block=NULL;
    size_t bestsize=0;
    if(size<=0) return block;
    if(freelist==NULL) return NULL;
    
    
    struct meta_block * curr=freelist;
    
    while(curr!=NULL){
        if(curr->size >=size+META_SIZE){
            //  find the fitted one
            if(curr->size < bestsize){
                bestsize=curr->size;
                block=curr;
            }
        }
        curr=curr->next;
    }
    //  can not find the fitted free block
    return block;
    
}
void do_alloc_BF(size_t size){
    struct meta_block * block=NULL;
    
    //  check the fitted free block
    block=checkBestFree(size);
    //  if not found fitted block
    if(block==NULL){
        block=alloc(size);
        
        if(global->next_mem==NULL){
            //  if there is only one block in list, initialize global->next_mem
            global->next_mem=block;
        }
    }else{
        //  remove fitted one from freelist
        rmfreelist(block);
    }
    size_t real_size = block->size - META_SIZE;
    
    //  store the current end pointer
    struct meta_block * temp = end;
    
    
    if(end!=NULL){
        end->next=block;
    }
    end = block;
    end->size = real_size;
    end->free=1;
    end->next_mem=NULL;
    end->prev_mem=temp;
}

void *bf_malloc(size_t size){
    struct meta_block * block=NULL;
    if(size<=0) return block;
    
    //  first time to call malloc
    if (global == NULL){
        init(size);
        return global + 1;
    }
    
    do_alloc_BF(size);
    //  return the block we newly alloc
    return (end+1);
}


void bf_free(void *ptr){
    if (ptr==NULL) return;
    ff_free(ptr);
}
//  get total block number
unsigned long get_data_segment_size(){
    struct meta_block * curr=global;
    unsigned long res=0;
    while(curr->next_mem!=NULL){
        res+=curr->size;
        curr=curr->next_mem;
    }
    return res;
}
//  get free block number
unsigned long get_data_segment_free_space_size(){
    struct meta_block * curr=freelist;
    unsigned long res=0;
    if(freelist==NULL) return 0;
    while(curr->next!=NULL){
        
        res+=curr->size;
        curr=curr->next;
        
        
    }
    return res;
}
