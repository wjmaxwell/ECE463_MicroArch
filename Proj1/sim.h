#ifndef SIM_CACHE_H
#define SIM_CACHE_H
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <vector>
#include <cmath>

using namespace std;


typedef 
struct {
   uint32_t BLOCKSIZE;
   uint32_t L1_SIZE;
   uint32_t L1_ASSOC;
   uint32_t L2_SIZE;
   uint32_t L2_ASSOC;
   uint32_t PREF_N;
   uint32_t PREF_M;
} cache_params_t;

typedef
struct {
   int valid;
   int dirty;
   uint32_t tag;
   uint32_t addr;
   int lru;
} Block;

class Cache{
public:
   Cache* next;

   int blockSize;
   int assoc;
   int size;

   int read;
   int readmiss;
   int write;
   int writemiss;
   int writeback;

   int sets;
   int ways;
   uint32_t index_bits;
   uint32_t block_offset_bits;

   vector< vector<Block> > blocks;
   
   Cache(int b, int a, int s) {
      next = NULL;

      blockSize = b;
      assoc = a;
      size = s;

      read = 0;
      readmiss = 0;
      write = 0;
      writemiss = 0;
      writeback = 0;

      sets = size / (assoc * blockSize);
      ways = assoc;

      index_bits = log2(sets);
      block_offset_bits = log2(blockSize);
      
      blocks.resize(sets);

      for(int i=0; i<sets; i++){
         blocks[i].resize(ways);
      }
     
      uint32_t lru_counter = 0;
      for (int i = 0; i < sets; i++){
         lru_counter = 0;
         for(int j = 0; j < ways; j++){
            blocks[i][j].valid = 0;
            blocks[i][j].dirty = 0;
            blocks[i][j].tag = 0;
            blocks[i][j].lru = lru_counter;
            blocks[i][j].addr = 0;

            lru_counter += 1;
         }
      }  
      
      
   }
   void insertCache(Cache* c);

   void incrRead();
   void incrReadMiss();
   void incrWrite();
   void incrWriteMiss();
   void incrWriteBack();

   int getRead();
   int getReadMiss();
   int getWrite();
   int getWriteMiss();
   int getWriteBack();

   uint32_t getIndex(uint32_t addr);
   uint32_t getBlockOff(uint32_t addr);
   uint32_t getTag(uint32_t addr);

   void nWaySetAssociativeL1(uint32_t addr, char rw);

};

#endif