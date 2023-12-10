#ifndef SIM_PROC_H
#define SIM_PROC_H

#include <vector>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>


using namespace std;

typedef struct proc_params{
    unsigned long int rob_size;
    unsigned long int iq_size;
    unsigned long int width;
}proc_params;

typedef struct{
    bool valid;
    int tag;
} rename_map_table;

class Processor{
public:
    int head;
    int tail;
    int cycle;
    int inst_age;
    int width;
    int rob_size;
    int iq_size;
    bool file_empty;
    bool rob_full;

    class instruction{
    public:
        uint64_t pc;
        int src1;
        int src2;
        int dst;
        int optype;
        bool rob_rdy;
        bool rs1;
        bool rs2;
        bool valid;
        int reg_age;
        int age;
        int lat;
        int FE[2];
        int DE[2];
        int RN[2];
        int RR[2];
        int DI[2];
        int IS[2];
        int EX[2];
        int WB[2];
        int RT[2];   

        instruction(){
            pc = 0;
            src1 = -1;
            src2 = -1;
            dst = -1;
            optype = -1;
            reg_age = 1;
            rob_rdy = false;
            rs1 = false;
            rs2 = false;
            valid = false;
            age = -1;
            lat = 0;
            for(int i=0; i<2; i++){
                FE[i] = 0;
                DE[i] = 0;
                RN[i] = 0;
                RR[i] = 0;
                DI[i] = 0;
                IS[i] = 0;
                EX[i] = 0;
                WB[i] = 0;
                RT[i] = 0;
            }
        }
    };

    vector<instruction> DE;
    vector<instruction> RN;
    vector<instruction> RR;
    vector<instruction> DI;
    vector<instruction> EX;
    vector<instruction> WB;
    vector<instruction> RT;

    vector<instruction> IQ;
    vector<instruction> ROB;

    vector<rename_map_table> RMT;
    
    Processor(int rs, int iqs, int wth){

        head = 0;
        tail = 0;
        cycle = 0;
        inst_age = 0;
        rob_size = rs + 1;
        iq_size = iqs;
        width = wth;
        file_empty = false;
        rob_full = false;

        for(int i=0; i<rob_size; i++){
            ROB.push_back(instruction());
        }

        for(int i=0; i<iq_size; i++){
            IQ.push_back(instruction());
        }

        for(int i=0; i<width; i++){
            DE.push_back(instruction());
            RN.push_back(instruction());
            RR.push_back(instruction());
            DI.push_back(instruction());
        }

        for(int i=0; i<(width*5); i++){
            EX.push_back(instruction());
            WB.push_back(instruction());
            //RT.push_back(instruction());
        }

        RMT.resize(67);
        for(int i=0; i<67; i++){
            RMT[i].valid = false;
            RMT[i].tag = -1;
        }

    }

    bool Advance_Cycle();
    bool checkEmpty(vector<instruction> reg);
    int checkIQ();
    bool checkSpace(vector<instruction> reg);

    void Fetch(FILE* FP);
    void Decode();
    void Rename();
    void RegRead();
    void Dispatch();
    void Issue();
    void Execute();         
    void Writeback();
    void Retire();

};

// Put additional data structures here as per your requirement

#endif
