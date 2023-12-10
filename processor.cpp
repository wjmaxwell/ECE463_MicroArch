#include "sim_proc.h"
#include <vector>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>



bool Processor::Advance_Cycle(){
    //check if all registers are empty && trace file is empty
    if(checkEmpty(DE) && checkEmpty(RN) && checkEmpty(RR) && checkEmpty(DI) && checkEmpty(IQ) && checkEmpty(EX) && checkEmpty(WB) && checkEmpty(RT) && file_empty){
        return false;  
    }

    //for testing
    //if(cycle >= 12000){
    //    return false;
    //}
        

    cycle += 1;
    //printf("%d\n", cycle);
    return true;
}

bool Processor::checkEmpty(vector<instruction> reg){
    // returns false if reg contains a valid instruction
    for(unsigned int i=0; i<reg.size(); i++){
        if(reg[i].valid){
            return false;
        }
    }
    return true;
}

int Processor::checkIQ(){
    //returns number of free spaces in the IQ
    int count = 0;
    for(int i=0; i<iq_size; i++){
        if(!IQ[i].valid){
            count += 1;
        }
    }
    return count;
}

bool Processor::checkSpace(vector<instruction> reg){
    //returns true if there is 1 space open in reg
    for(unsigned int i=0; i<reg.size(); i++){
        if(!reg[i].valid){
            return true;
        }
    }
    return false;
}

void Processor::Fetch(FILE* FP){
    int optype, dst, src1, src2;
    unsigned long int pc;

    if(checkEmpty(DE)) {
        for(int i=0; i<width; i++){
            //reads in width # of instructions
            if(fscanf(FP, "%lx %d %d %d %d", &pc, &optype, &dst, &src1, &src2) == EOF){
                //printf("END OF FILE, CYCLE %d\n", cycle);
                file_empty = true;
                break;
            }
            // assign instructions to DE
            DE[i].valid = true;
            DE[i].pc = pc;
            DE[i].optype = optype;
            DE[i].dst = dst;
            DE[i].src1 = src1;
            DE[i].src2 = src2;
            DE[i].age = inst_age;
            DE[i].FE[0] = cycle;
            DE[i].FE[1] = 1;
            DE[i].reg_age = 1;

            inst_age += 1;

            DE[i].DE[0] = cycle + 1;

            //printf("FETCH %d %d %d %d\n", DE[i].optype, DE[i].dst, DE[i].src1, DE[i].src2);
        }
    }

}

void Processor::Decode(){
    if(!checkEmpty(DE) && checkEmpty(RN)){
        //if DE is not empty and RN is empty
        for(int i=0; i<width; i++){
            if(DE[i].valid){
                //passes valid instructions to RN, sets old instruction in DE to false
                RN[i] = DE[i];
                RN[i].valid;
                RN[i].RN[0] = cycle + 1;
                RN[i].DE[1] = DE[i].reg_age;

                DE[i].valid = false;
                RN[i].reg_age = 1;
                    //printf("DECODE %d %d %d %d\n", RN[i].optype, RN[i].dst, RN[i].src1, RN[i].src2);
            }
        }
    }
    else{
        //increment time instruction is in DE if there is a stall
        for(int i=0; i<width; i++){
            DE[i].reg_age += 1;
        }
    }
}

    void Processor::Rename(){
        if(!checkEmpty(RN) && checkEmpty(RR) && ((rob_size - abs(tail - head) - 1) >= width)){
            //check if tail will overlap/overwrite head once incremented
            bool overlap = true;
            for(int k=1; k<=width; k++){
                int hold = tail;
                hold = hold + k;
                if(hold >= rob_size){
                    hold = hold % (rob_size);
                }

                if(hold != head){
                    overlap = false;
                }
                else{
                    overlap = true;
                    break;
                }
            }
            //if not then do not stall
            for(int i=0; (i<width && !overlap); i++){
                if(RN[i].valid /*&& !overlap*/){
                    //allocate space in ROB for intruction
                    ROB[tail] = RN[i];
                    
                    //pass instruction to RN
                    RR[i] = RN[i];

                    //rename source registers
                    if(RR[i].src1 != -1){
                        if(RMT[RR[i].src1].valid){
                            RR[i].src1 = RMT[RR[i].src1].tag;
                        }
                        else{
                            RR[i].rs1 = true;
                        }
                    }

                    //preset certain edge case IQ ready values
                    if(RR[i].src1 == -1){
                        RR[i].rs1 = true;
                    }

                    if(RR[i].src2 != -1){
                        if(RMT[RR[i].src2].valid){
                            RR[i].src2 = RMT[RR[i].src2].tag;
                        }
                        else {
                            RR[i].rs2 = true;
                        }
                    }

                    if(RR[i].src2 == -1){
                        RR[i].rs2 = true;
                    }

                    //update RMT
                    if(RR[i].dst != -1){
                        RMT[RR[i].dst].valid = true;
                        RMT[RR[i].dst].tag = tail;
                    }

                    //rename destination register
                    RR[i].dst = tail;
                    
                    if(tail == rob_size - 1){
                        tail = 0;
                    }
                    else{
                        tail += 1; //increment tail to for next ROB entry
                    }

                    RR[i].RR[0] = cycle + 1;    
                    RR[i].RN[1] = RN[i].reg_age;

                    RN[i].valid = false;
                    RR[i].reg_age = 1;

                    //printf("RENAME %d %d %d %d\n", RR[i].optype, RR[i].dst, RR[i].src1, RR[i].src2);
                    //printf("%d %d %d %d\n", ROB[i].optype, ROB[i].dst, ROB[i].src1, ROB[i].src2);
                }
            }
        }
        //increment time instruction is in RN in case of a stall
        for(int i=0; (i<width && RN[i].valid); i++){
            RN[i].reg_age += 1;
        }
    }  

    void Processor::RegRead(){
        //check if RR is not empty and DI is empty
        if(!checkEmpty(RR) && checkEmpty(DI)){
            for(int i=0; i<width; i++){
                if(RR[i].valid){
                    //check if ROB entry is ready for each source register
                    if(ROB[RR[i].src1].rob_rdy && RR[i].src1 != -1){
                        RR[i].rs1 = true;
                    }

                    if(ROB[RR[i].src2].rob_rdy && RR[i].src2 != -1){
                        RR[i].rs2 = true;
                    }
                    
                    //pass instruction to DI
                    DI[i] = RR[i];
                    DI[i].DI[0] = cycle + 1;
                    DI[i].RR[1] = RR[i].reg_age;            

                    RR[i].valid = false;
                    DI[i].reg_age = 1; 

                    //printf("%llx REGREAD %d %d %d %d\n", DI[i].pc, DI[i].optype, DI[i].dst, DI[i].src1, DI[i].src2);
                }
            }
        }
        else{
            //increment time in register in case of a stall
            for(int i=0; i<width; i++){
                RR[i].reg_age += 1;
            }
        }
        
    }

    void Processor::Dispatch(){
        //check if there's space in the IQ
        int freeiq = checkIQ();
        if(!checkEmpty(DI) && (freeiq >= width)){
            //pass the instructions from DI to the free spaces in the IQ
            for(int i=0; i<width; i++){
                //find free space in IQ
                for(int k=0; k<iq_size; k++){
                    if(!IQ[k].valid){
                        if(DI[i].valid){
                            IQ[k] = DI[i];
                            IQ[k].IS[0] = cycle + 1;
                            IQ[k].DI[1] = DI[i].reg_age;      

                            DI[i].valid = false;
                            IQ[k].reg_age = 1;
                            break;                
                            //printf("%llx DISPATCH %d %d %d %d\n", IQ[iq_spots[i]].pc, IQ[iq_spots[i]].optype, IQ[iq_spots[i]].dst, IQ[iq_spots[i]].src1, IQ[iq_spots[i]].src2);
                        }
                    }
                }
            }
        }
        else{
            //increment time in reg in case of a stall
            for(int i=0; i<width; i++){
                DI[i].reg_age += 1;
            }
        }
    }     

    void Processor::Issue(){    
        //check if there is space in EX and IQ is not empty
        if(checkSpace(EX) && !checkEmpty(IQ)){
            int next_issue;
            for(int i=0; (i<width && checkSpace(EX)); i++){
                next_issue = -1;
                //find the oldest ready instruction in the IQ
                for(int j=0; j<iq_size; j++){
                    if(IQ[j].valid && IQ[j].rs1 && IQ[j].rs2){
                        if(next_issue == -1){
                            next_issue = j;
                        }
                        else if(IQ[j].age < IQ[next_issue].age){
                            next_issue = j;
                        }
                    }
                }
                //if no ready instrucions in the IQ then break
                if(next_issue == -1){
                    break;
                }
                //assert(next_issue < iq_size);
                //assert(IQ[next_issue].valid && IQ[next_issue].rs1 && IQ[next_issue].rs2);

                //pass ready instruction from IQ to EX
                for(int j=0; j<width*5; j++){
                    if(!EX[j].valid){
                        //assert(IQ[next_issue].valid);

                        EX[j] = IQ[next_issue];
                        IQ[next_issue].valid = false;
                        EX[j].EX[0] = cycle + 1;
                        EX[j].IS[1] = EX[j].reg_age;                       

                        //set the EX latency
                        if(EX[j].optype == 2){
                            EX[j].lat = 5;
                        }
                        else if(EX[j].optype == 1){
                            EX[j].lat = 2;
                        }
                        else{
                            EX[j].lat = 1;
                        }  
                        EX[j].reg_age = 1;
                        //printf("%llx ISSUE %d %d %d %d\n", EX[j].pc, EX[j].optype, EX[j].dst, EX[j].src1, EX[j].src2);
                        break;
                        
                    }
                }
            }            
            
        }
        //increment time in IQ until instruction is issued from IQ
        for(int i=0; i<iq_size; i++){
            IQ[i].reg_age += 1;
        }
    }  

    void Processor::Execute(){
        if(!checkEmpty(EX)){
        for(int i=0; i<width*5; i++){
            //pass instruction to WB once it has spent latency amount of time in EX
            if(EX[i].valid && (EX[i].lat == EX[i].reg_age)){
                for(int j=0; j<width*5; j++){
                    if(!WB[j].valid){
                        WB[j] = EX[i];
                        WB[j].WB[0] = cycle + 1;
                        WB[j].EX[1] = WB[j].reg_age;
                        WB[j].reg_age = 1;
                        EX[i].valid = false;
                        break;
                    }
                }

                //wakeup dependent instructions in IQ
                for(int j=0; j<iq_size; j++){
                    if(IQ[j].valid){
                        if(IQ[j].src1 == EX[i].dst){
                            IQ[j].rs1 = true;
                        }
                        if(IQ[j].src2 == EX[i].dst){
                            IQ[j].rs2 = true;
                        }
                    }
                }

                //wakeup dependent instructions in DI and RR
                for(int j=0; j<width; j++){
                    if(DI[j].valid){
                        if(DI[j].src1 == EX[i].dst){
                            DI[j].rs1 = true;
                        }
                        if(DI[j].src2 == EX[i].dst){
                            DI[j].rs2 = true;
                        }
                    }
                    if(RR[j].valid){
                        if(RR[j].src1 == EX[i].dst){
                            RR[j].rs1 = true;
                        }
                        if(RR[j].src2 == EX[i].dst){
                            RR[j].rs2 = true;
                        }
                    }
                }
                //printf("%llx EXECUTE %d %d %d %d\n", WB[i].pc, WB[i].optype, WB[i].dst, WB[i].src1, WB[i].src2);
            }
        }

        //increment time spent in reg until it reaches the latency
        for(int i=0; i<width*5; i++){
            if(EX[i].valid){
                EX[i].reg_age += 1;
            }
        }
        }
    }          

    void Processor::Writeback(){
        if(!checkEmpty(WB)){
        for(int i=0; i<width*5; i++){
            if(WB[i].valid){
                //set the instructions ROB entry to ready
                ROB[WB[i].dst].rob_rdy = true;
                
                WB[i].RT[0] = cycle + 1;
                WB[i].WB[1] = WB[i].reg_age;
                WB[i].reg_age = 1;

                //rename dst and source regs to original names
                if(WB[i].dst != -1) {
                    WB[i].src1 = ROB[WB[i].dst].src1;
                    WB[i].src2 = ROB[WB[i].dst].src2;
                }
                WB[i].dst = ROB[WB[i].dst].dst;

                //pass instruction from WB to RT
                RT.push_back(WB[i]);
                WB[i].valid = false;

                //printf("%llx WRITEBACK %d %d %d %d\n", RT[i].pc, RT[i].optype, RT[i].dst, RT[i].src1, RT[i].src2);                
            }
        }
        }
    }

    void Processor::Retire(){  
        if(!checkEmpty(RT)){
            //in case faulty instruction comes through, delete the instruction
            for(unsigned int i=0; i<RT.size(); i++){
                if(RT[i].optype == -1){
                    RT.erase(RT.begin() + i);
                }
            }
            
            int retire_count = 0;
            while(retire_count < width){
                //if head entry of ROB is ready
                if(ROB[head].rob_rdy){
                    //invalidate RMT entry if head of ROB is most recent entry
                    if(ROB[head].dst >= 0){
                        if(RMT[ROB[head].dst].tag == head){
                            RMT[ROB[head].dst].valid = false;
                        }
                    }
                    for(unsigned int i=0; i<RT.size(); i++){
                        //retire instruction in RT that matches the head ROB entry
                        if(RT[i].age == ROB[head].age){
                        
                            RT[i].RT[1] = RT[i].reg_age;
                            printf("%d fu{%d} src{%d,%d} dst{%d} FE{%d,%d} DE{%d,%d} RN{%d,%d} RR{%d,%d} DI{%d,%d} IS{%d,%d} EX{%d,%d} WB{%d,%d} RT{%d,%d}\n", RT[i].age, RT[i].optype, 
                                RT[i].src1, RT[i].src2, RT[i].dst, RT[i].FE[0], RT[i].FE[1], RT[i].DE[0], RT[i].DE[1], RT[i].RN[0], RT[i].RN[1], RT[i].RR[0], RT[i].RR[1], 
                                RT[i].DI[0], RT[i].DI[1], RT[i].IS[0], RT[i].IS[1], RT[i].EX[0], RT[i].EX[1], RT[i].WB[0], RT[i].WB[1], RT[i].RT[0], RT[i].RT[1]);
                            //erase from RT to free space
                            RT.erase(RT.begin() + i);

                            //increment head of ROB
                            if(head == rob_size - 1){
                                head = 0;
                            }
                            else{
                                head += 1;
                            }
                            break;
                        }
                    }
                    
                    retire_count += 1;
                }
                else {
                    break;
                }
            }

            //increment time spent in reg until instruction is retired
            for(unsigned int i=0; i<RT.size(); i++){
                RT[i].reg_age += 1;
            }
        
    }
}