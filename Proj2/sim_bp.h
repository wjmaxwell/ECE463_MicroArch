#include <vector>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#ifndef SIM_BP_H
#define SIM_BP_H

using namespace std;

typedef struct bp_params{
    unsigned long int K;
    unsigned long int M1;
    unsigned long int M2;
    unsigned long int N;
    char*             bp_name;
}bp_params;


typedef struct {
    int count;
} Counter;

class Predictor{
public:
    int index_bits;
    int global_bits;
    int predict;
    int mispredict;
    uint32_t global_addr;
    vector<Counter> counters;

    Predictor(int m, int n){
        index_bits = m;
        global_bits = n;
        predict = 0;
        mispredict = 0;
        global_addr = 0x0;

        int counter_size = pow(2, m);

        counters.resize(counter_size);

        for(int i=0; i<counter_size; i++){
            counters[i].count = 2;
        }
    }

    void incrCount(uint32_t index){
        if(counters[index].count < 3){
            counters[index].count += 1;
        }
        
    }
    void decrCount(uint32_t index){
        if(counters[index].count > 0){
            counters[index].count -= 1;
        }
    }
    int getCount(int index){
        return counters[index].count;
    }

    void incrPred(){
        predict += 1;
    }
    void incrMiss(){
        mispredict += 1;
    }

    int getPred(){
        return predict;
    }
    int getMiss(){
        return mispredict;
    }
    int getIndex(){
        return index_bits;
    }
    int getGlobal(){
        return global_bits;
    }

    int bimodalIndex(uint32_t addr){
        addr = addr >> 2;
        uint32_t tmp = 0;
        for(int i=0; i<index_bits; i++){
            tmp = tmp << 1;
            tmp = tmp | 0x1;
        }
        
        return addr & tmp;
    }

    uint32_t gshareIndex(uint32_t addr){   
        addr = addr >> 2;

        uint32_t val = 0;
        if(global_bits > 0){
            uint32_t tmp = 0;
            for(int i=0; i<global_bits; i++){
                tmp = tmp << 1;
                tmp = tmp | 0x1;
            }
        
            val = (addr >> (index_bits - global_bits)) & tmp;
            val = (val ^ global_addr) << (index_bits - global_bits);
        }

        uint32_t tmp2 = 0;
        for(int i=0; i<(index_bits - global_bits); i++){
            tmp2 = tmp2 << 1;
            tmp2 = tmp2 | 0x1;
        }

        addr = addr & tmp2;

        if(global_bits > 0){
            addr = addr | val;
        }

        return addr;        

    }

    void updateGlobal(char outcome){     
        
        if(global_bits > 0){
            global_addr = global_addr >> 1;

            if(outcome == 't'){
                uint32_t tmp2 = 1;
                tmp2 = tmp2 << (global_bits-1);
                global_addr = global_addr | tmp2;
            }
        }

    }

    void bimodalPredictor(uint32_t addr, char outcome){
        int index = bimodalIndex(addr);
        int tmp_count = getCount(index);
        char tmp_out;

        if(tmp_count >= 2){
            tmp_out = 't';
        }
        else{
            tmp_out = 'n';
        }

        if(tmp_out != outcome){
            incrMiss();
        }

        if(outcome == 't'){
            incrCount(index);
        }
        else if(outcome == 'n'){
            decrCount(index);
        }
        
        incrPred();

    }

    void gsharePredictor(uint32_t addr, char outcome){
        uint32_t index = gshareIndex(addr);
        int tmp_count = getCount(index);
        char tmp_out;

        if(tmp_count >= 2){
            tmp_out = 't';
        }
        else{
            tmp_out = 'n';
        }

        if(tmp_out != outcome){
            incrMiss();
        }

        if(outcome == 't'){
            incrCount(index);
        }
        else if(outcome == 'n'){
            decrCount(index);
        }
        
        incrPred();

        updateGlobal(outcome);

    }
    
};

// Put additional data structures here as per your requirement

#endif
