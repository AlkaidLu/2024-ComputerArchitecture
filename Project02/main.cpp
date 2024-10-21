#include <iostream>
#include <time.h>
#include "MIPSSimulator.h"
#include "Pipeline.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include"comm.h"
#include"producer_consumer.h"


int main(int argc, char *argv[])
{
    if (argc < 2) {
            std::cout << "Usage: " << argv[0] << " <program_file>" << std::endl;
            return 1;
        }
    int semId = -1, shmId = -1;
    struct Buffer *shm = NULL;

    Initialize(&semId, &shmId, &shm);

    int pid=fork();
    if(pid < 0)
        {
            printf("fork failed!\n");
            exit(EXIT_FAILURE);
        }
    if(pid==0){
        PipeLine(semId, shm);//输入是共享内存那部分
        exit(EXIT_SUCCESS);
    }
    else{
        MIPSSimulator simulator(1024); // Initialize simulator with 1024 bytes of memory
        simulator.load_memory(argv[1]); // Load the program into memory
        simulator.save_memory("instruction_data.txt");
        simulator.run(semId,shm); // Start executing instructions
        //在这个部分把译码的放入共享内存里
        simulator.save_memory("result.txt");
        
        int status;
        waitpid(pid, &status, 0); // 等待子进程结束
        printf("OK!Child exited normally!\n");
        if (WIFEXITED(status)) {
            printf("Child exited with status %d\n", WEXITSTATUS(status));
        }

        exit(EXIT_SUCCESS);
     
    }
    
    getchar();
    Destroy(semId, shmId, shm);
    return 0;
}