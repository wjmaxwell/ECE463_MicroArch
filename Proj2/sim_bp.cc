#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sim_bp.h"

/*  argc holds the number of command line arguments
    argv[] holds the commands themselves

    Example:-
    sim bimodal 6 gcc_trace.txt
    argc = 4
    argv[0] = "sim"
    argv[1] = "bimodal"
    argv[2] = "6"
    ... and so on
*/
int main (int argc, char* argv[])
{
    FILE *FP;               // File handler
    char *trace_file;       // Variable that holds trace file name;
    bp_params params;       // look at sim_bp.h header file for the the definition of struct bp_params
    char outcome;           // Variable holds branch outcome
    unsigned long int addr; // Variable holds the address read from input file
    
    if (!(argc == 4 || argc == 5 || argc == 7))
    {
        printf("Error: Wrong number of inputs:%d\n", argc-1);
        exit(EXIT_FAILURE);
    }
    
    params.bp_name  = argv[1];
    
    // strtoul() converts char* to unsigned long. It is included in <stdlib.h>
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
    {
        if(argc != 4)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);
        params.N        = 0;
        trace_file      = argv[3];
        printf("COMMAND\n%s %s %lu %s\n", argv[0], params.bp_name, params.M2, trace_file);
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
    {
        if(argc != 5)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.M2       = strtoul(argv[2], NULL, 10);   //changed to params.M2 instead of params.M1 for simplicity
        params.N        = strtoul(argv[3], NULL, 10);
        trace_file      = argv[4];
        printf("COMMAND\n%s %s %lu %lu %s\n", argv[0], params.bp_name, params.M2, params.N, trace_file); //changed to params.M2 instead of params.M1 for simplicity
        if(params.N > params.M2)
        {
            printf("Error: %s wrong number of global branch bits:%d\n", params.bp_name, params.N);
            exit(EXIT_FAILURE);
        }

    }
    else if(strcmp(params.bp_name, "hybrid") == 0)          // Hybrid
    {
        if(argc != 7)
        {
            printf("Error: %s wrong number of inputs:%d\n", params.bp_name, argc-1);
            exit(EXIT_FAILURE);
        }
        params.K        = strtoul(argv[2], NULL, 10);
        params.M1       = strtoul(argv[3], NULL, 10);
        params.N        = strtoul(argv[4], NULL, 10);
        params.M2       = strtoul(argv[5], NULL, 10);
        trace_file      = argv[6];
        printf("COMMAND\n%s %s %lu %lu %lu %lu %s\n", argv[0], params.bp_name, params.K, params.M1, params.N, params.M2, trace_file);

    }
    else
    {
        printf("Error: Wrong branch predictor name:%s\n", params.bp_name);
        exit(EXIT_FAILURE);
    }
    
    // Open trace_file in read mode
    FP = fopen(trace_file, "r");
    if(FP == NULL)
    {
        // Throw error and exit if fopen() failed
        printf("Error: Unable to open file %s\n", trace_file);
        exit(EXIT_FAILURE);
    }

    Predictor *branchPred = new Predictor(params.M2, params.N); // create Predictor object
    
    char str[2];
    while(fscanf(FP, "%lx %s", &addr, str) != EOF)
    {
        
        outcome = str[0];
        /*
        if (outcome == 't')
            printf("%lx %s\n", addr, "t");           // Print and test if file is read correctly
        else if (outcome == 'n')
            printf("%lx %s\n", addr, "n");           // Print and test if file is read correctly
        */
        if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal
        {
            branchPred->bimodalPredictor(addr, outcome);
        }
        else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare
        {
            branchPred->gsharePredictor(addr, outcome);
        }

        /*************************************
            Add branch predictor code here
        **************************************/
    }

    printf("OUTPUT\n");
    printf("number of predictions:       %d\n", branchPred->getPred());
    printf("number of mispredictions:    %d\n", branchPred->getMiss());
    printf("misprediction rate:          %0.2f%%\n", 100*((float)branchPred->getMiss() / (float)branchPred->getPred()));
    if(strcmp(params.bp_name, "bimodal") == 0)              // Bimodal output
    {        
        printf("FINAL BIMODAL CONTENTS\n");
    }
    else if(strcmp(params.bp_name, "gshare") == 0)          // Gshare output
    {        
        printf("FINAL GSHARE CONTENTS\n");
    }

    int hold = pow(2, params.M2);
    for(int i=0; i<hold; i++){
        printf("%d      %d\n", i, branchPred->getCount(i));
    }

    printf("misprediction rate:          %0.2f%%\n", 100*((float)branchPred->getMiss() / (float)branchPred->getPred()));

    return 0;
}
