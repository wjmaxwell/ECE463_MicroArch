#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <iostream>

void Cache::insertCache(Cache* c){
    next = c;
}

void Cache::incrRead(){
    read += 1;
}

void Cache::incrReadMiss(){
    readmiss += 1;
}

void Cache::incrWrite(){
    write += 1;
}

void Cache::incrWriteMiss(){
    writemiss += 1;
}

void Cache::incrWriteBack(){
    writeback += 1;
}

int Cache::getRead(){
    return read;
}

int Cache::getReadMiss(){
    return readmiss;
}

int Cache::getWrite(){
    return write;
}
int Cache::getWriteMiss(){
    return writemiss;
}

int Cache::getWriteBack(){
    return writeback;
}

uint32_t Cache::getIndex(uint32_t addr) {
    uint32_t tmp = 0;
    for(uint32_t i=0; i<index_bits; i++){
        tmp = tmp << 1;
        tmp = tmp | 0x1;
    }

    tmp = tmp << block_offset_bits;

    return ((addr & tmp) >> block_offset_bits);
}

uint32_t Cache::getBlockOff(uint32_t addr){
    uint32_t tmp = 0;
    for(uint32_t i=0; i<block_offset_bits; i++){
        tmp = tmp << 1;
        tmp = tmp | 0x1;
    }

    return addr & tmp;
}

uint32_t Cache::getTag(uint32_t addr){
    return addr >> (index_bits + block_offset_bits);
}


void Cache::nWaySetAssociativeL1(uint32_t addr, char rw){
    uint32_t tmp_index = getIndex(addr);

    Block tmp_block;
    Block lru_block;

    bool tag_present = false;
    int tmp_lru;
    for(int i=0; i<ways; i++) {
        if(blocks[tmp_index][i].tag == getTag(addr)){
            tag_present = true;
            tmp_lru = i;
            tmp_block = blocks[tmp_index][i];
            break;
        }
    }

    if(!tag_present){
        //find the lru block
        for(int i=0; i<ways; i++) {
            if(blocks[tmp_index][i].lru == ways-1){
                tmp_block = blocks[tmp_index][i];
                tmp_lru = i;
                break;
            }
        }
    }

    //increment all lru in blocks within the set
    for(int i=0; i<ways; i++){
        if(tmp_block.lru > blocks[tmp_index][i].lru){
            lru_block = blocks[tmp_index][i];
            lru_block.lru += 1;
            blocks[tmp_index][i] = lru_block;
        }
    }

    //set referenced block to mru
    tmp_block.lru = 0;

    if(tmp_block.valid == 0) {
        tmp_block.valid = 1;

        if(rw == 'w') {
            incrWrite();
            incrWriteMiss();

            if(next){
                next->nWaySetAssociativeL1(addr, 'r');
            }
            
            tmp_block.dirty = 1;
        }
        else if(rw == 'r'){
            incrRead();
            incrReadMiss();

            if(next){
                next->nWaySetAssociativeL1(addr, 'r');
            }
        }
    }
    else if(tmp_block.valid == 1){
        if(rw == 'w') {
            incrWrite();

            if(tmp_block.tag != getTag(addr)){
                incrWriteMiss();
                if(tmp_block.dirty == 1){
                    incrWriteBack();

                    //NEED TO WRITEBACK TO L2 HERE
                    if(next){
                        next->nWaySetAssociativeL1(tmp_block.addr, 'w');
                    }
         
                    tmp_block.dirty = 0;
                }


                //NEED READ TO L2 HERE
                if(next){
                    next->nWaySetAssociativeL1(addr, 'r');
                }

            }
         
            tmp_block.dirty = 1;
        }
        else if(rw == 'r'){
            incrRead();

            if(tmp_block.tag != getTag(addr)){
                incrReadMiss();

                if(tmp_block.dirty == 1){
                    incrWriteBack();
         
                    //NEED TO WRITEBACK TO L2 HERE
                    if(next){
                        next->nWaySetAssociativeL1(tmp_block.addr, 'w');
                    }
         
                    tmp_block.dirty = 0;
                }

                //NEED READ REQUEST L2_CACHE HERE
                if(next){
                    next->nWaySetAssociativeL1(addr, 'r');
                }
            
            }
        }

        
    }

    tmp_block.tag = getTag(addr);
    tmp_block.addr = addr;
    blocks[tmp_index][tmp_lru] = tmp_block;
}