#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <iostream>
#include <cmath>
#include "sim.h"

using namespace std;

/* "argc" holds the number of command-line arguments.
   "argv[]" holds the arguments themselves.
   Example:
   ./sim 32 8192 4 262144 8 3 10 gcc_trace.txt
   argc = 9
   argv[0] = "./sim"
   argv[1] = "32"
   argv[2] = "8192"
   ... and so on 
*/

int main (int argc, char *argv[]) {
   FILE *fp;			// File pointer.
   char *trace_file;		// This variable holds the trace file name.
   cache_params_t params;	// Look at the sim.h header file for the definition of struct cache_params_t.
   char rw;			// This variable holds the request's type (read or write) obtained from the trace.
   uint32_t addr;		// This variable holds the request's address obtained from the trace.
				// The header file <inttypes.h> above defines signed and unsigned integers of various sizes in a machine-agnostic way.  "uint32_t" is an unsigned integer of 32 bits.

   // Exit with an error if the number of command-line arguments is incorrect.
   if (argc != 9) {
      printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
      exit(EXIT_FAILURE);
   }
   if(ceil(log2(atoi(argv[1]))) != floor(log2(atoi(argv[1])))) {
      printf("Error: BLOCKSIZE must be a power of 2\n");
      exit(EXIT_FAILURE);
   }
    
   // "atoi()" (included by <stdlib.h>) converts a string (char *) to an integer (int).
   params.BLOCKSIZE = (uint32_t) atoi(argv[1]);
   params.L1_SIZE   = (uint32_t) atoi(argv[2]);
   params.L1_ASSOC  = (uint32_t) atoi(argv[3]);
   params.L2_SIZE   = (uint32_t) atoi(argv[4]);
   params.L2_ASSOC  = (uint32_t) atoi(argv[5]);
   params.PREF_N    = (uint32_t) atoi(argv[6]);
   params.PREF_M    = (uint32_t) atoi(argv[7]);
   trace_file       = argv[8];

   if(ceil(log2(params.L1_SIZE / (params.L1_ASSOC * params.BLOCKSIZE))) != floor(log2(params.L1_SIZE / (params.L1_ASSOC * params.BLOCKSIZE)))){
      printf("Error: # of SETS must be a power of 2\n");
      exit(EXIT_FAILURE);
   }



   // Open the trace file for reading.
   fp = fopen(trace_file, "r");
   if (fp == (FILE *) NULL) {
      // Exit with an error if file open failed.
      printf("Error: Unable to open file %s\n", trace_file);
      exit(EXIT_FAILURE);
   }
    
   
   // Print simulator configuration.
   printf("===== Simulator configuration =====\n");
   printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
   printf("L1_SIZE:    %u\n", params.L1_SIZE);
   printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
   printf("L2_SIZE:    %u\n", params.L2_SIZE);
   printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
   printf("PREF_N:     %u\n", params.PREF_N);
   printf("PREF_M:     %u\n", params.PREF_M);
   printf("trace_file: %s\n", trace_file);
   //printf("===================================\n");
   

   Cache *L1_Cache = new Cache(params.BLOCKSIZE, params.L1_ASSOC, params.L1_SIZE);
   Cache *L2_Cache = NULL;
   if(params.L2_SIZE != 0 && params.L2_ASSOC != 0){
      L2_Cache = new Cache(params.BLOCKSIZE, params.L2_ASSOC, params.L2_SIZE);
      L1_Cache->insertCache(L2_Cache);
   }

   

   // Read requests from the trace file and echo them back.
   while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {	// Stay in the loop if fscanf() successfully parsed two tokens as specified.
      /*
      if (rw == 'r') {
         printf("r %x\n", addr);
      }
      else if (rw == 'w') {
         printf("w %x\n", addr);
      }
      else {
         printf("Error: Unknown request type %c.\n", rw);
	      exit(EXIT_FAILURE);
      }
      */


     L1_Cache->nWaySetAssociativeL1(addr, rw);


      ///////////////////////////////////////////////////////
      // Issue the request to the L1 cache instance here. 
      ///////////////////////////////////////////////////////
   }

   printf("\n");

   printf("===== L1 contents =====\n");
   for(int i=0; i<L1_Cache->sets; i++){
      printf("set       %d:   ", i);

      int lru_counter = 0;
      while(lru_counter < L1_Cache->ways) {
         for(int j=0; j<L1_Cache->ways; j++){
            if(L1_Cache->blocks[i][j].lru == lru_counter){
               printf("%x ", L1_Cache->blocks[i][j].tag);
               if(L1_Cache->blocks[i][j].dirty == 1){
                  printf("D   ");
               }
               else{
                  printf("    ");
               }
            }
         }

         lru_counter += 1;
      }

      printf("\n");
   }
   
   printf("\n");


   
   if(L2_Cache){
      printf("===== L2 contents =====\n");
      for(int i=0; i<L2_Cache->sets; i++){
         printf("set       %d:   ", i);

         int lru_counter = 0;
         while(lru_counter < L2_Cache->ways) {
            for(int j=0; j<L2_Cache->ways; j++){
               if(L2_Cache->blocks[i][j].lru == lru_counter){
                  printf("%x ", L2_Cache->blocks[i][j].tag);
                  if(L2_Cache->blocks[i][j].dirty == 1){
                     printf("D   ");
                  }
                  else{
                     printf("    ");
                  }
               }
            }

            lru_counter += 1;
         }

         printf("\n");
      }
   
      printf("\n");

   }
   

   printf("===== Measurements =====\n");
   printf("a. L1 reads:                      %d\n", L1_Cache->read);
   printf("b. L1 read misses:                %d\n", L1_Cache->readmiss);
   printf("c. L1 writes:                     %d\n", L1_Cache->write);
   printf("d. L1 write misses:               %d\n", L1_Cache->writemiss);
   printf("e. L1 miss rate:                  %.4f\n", ((double)(L1_Cache->readmiss + L1_Cache->writemiss) / (double)(L1_Cache->read + L1_Cache->write)));
   printf("f. L1 writebacks:                 %d\n", L1_Cache->writeback);
   printf("g. L1 prefetches:                 0\n");

   if(L2_Cache){
      printf("h. L2 reads (demand):             %d\n", L2_Cache->read);
      printf("i. L2 read misses (demand):       %d\n", L2_Cache->readmiss);
      printf("j. L2 reads (prefetch):           0\n");
      printf("k. L2 read misses (prefetch):     0\n");
      printf("l. L2 writes:                     %d\n", L2_Cache->write);
      printf("m. L2 write misses:               %d\n", L2_Cache->writemiss);
      printf("n. L2 miss rate:                  %.4f\n", ((double)L2_Cache->readmiss / (double)L2_Cache->read));
      printf("o. L2 writebacks:                 %d\n", L2_Cache->writeback);
      printf("p. L2 prefetches:                 0\n");
      printf("q. memory traffic:                %d\n", (L2_Cache->readmiss + L2_Cache->writemiss + L2_Cache->writeback));
   }
   else{
      printf("h. L2 reads (demand):             0\n");
      printf("i. L2 read misses (demand):       0\n");
      printf("j. L2 reads (prefetch):           0\n");
      printf("k. L2 read misses (prefetch):     0\n");
      printf("l. L2 writes:                     0\n");
      printf("m. L2 write misses:               0\n");
      printf("n. L2 miss rate:                  0.0000\n");
      printf("o. L2 writebacks:                 0\n");
      printf("p. L2 prefetches:                 0\n");
      printf("q. memory traffic:                %d\n", (L1_Cache->readmiss + L1_Cache->writemiss + L1_Cache->writeback));
   }
   
   

   return(0);
}
