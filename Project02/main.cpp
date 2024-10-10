#include <iostream>
#include "MIPSSimulator.h"
#include "Pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include"comm.h"


int main(int argc, char* argv[]) {

    
    int pid=fork();
    if(pid==0){
        int shmid = GetShm(4096);
	    void *addr = shmat(shmid,NULL,0);
        PipeLine();//输入是已经译码好的部分
    }
    else{
        if (argc < 2) {
            std::cout << "Usage: " << argv[0] << " <program_file>" << std::endl;
            return 1;
        }
        int shmid=CreateShm(4096);
	    Instruction *addr = (Instruction *)shmat(shmid,NULL,0);

        MIPSSimulator simulator(1024); // Initialize simulator with 1024 bytes of memory
        simulator.load_memory(argv[1]); // Load the program into memory
        simulator.save_memory("instruction_data.txt");
        simulator.run(); // Start executing instructions
        //在这个部分把译码的放入共享内存或者pipe里（）
        simulator.save_memory("result.txt");
        DestroyShm(shmid);
    }

    return 0;
}