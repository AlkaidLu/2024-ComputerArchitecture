# README

这是我的流水线作业的说明！



## 安装和运行

### 前提条件

- Linux 环境
- `g++` 编译器
- Git

### 运行说明

`make`

`./main "code.txt"`



## 项目结构 

项目包含以下关键文件： 

- **code.asm**：用于仿真的汇编代码。 

- **code.txt**：作为仿真输入的机器码文件。 

- **comm.h**，**fast.h**，**memory.h**，**Instruction.h**：定义仿真器各组件的头文件。 

-  **MIPSSimulator.cpp**，**Pipeline.cpp**，**producer_consumer.cpp**：实现仿真器、流水线阶段和同步机制的源文件。 

-  **pipeline_output.txt**：包含流水线阶段信息的输出文件。 

-  **result.txt**：存储最终仿真结果的文本文件。

  





## 代码说明

是在Project01基础上修改的。

实现了时序和执行分离，二者分别在子进程和父进程里面，使用了生产者消费者模型这部分在`main.cpp`里面。

```c++
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
```



执行部分的主要逻辑实现在`MIPSSimulator.cpp`里面，

```c++
int MIPSSimulator::executeInstruction(int semId,struct Buffer *shm) {
    ···
if (instruction == 0x00000000) { // Check for halt instruction
            printf("!!!ATTENTION!!!\n");
            printf("halt!\n");
            printf("!!!ATTENTION!!!\n");
            Producer(semId, shm, instr); 
            return 1;
        }
    switch (instr.opcode) {
        case OP_addi: 
            pc = INSN_addi(instruction); 
            strcpy(instr.Instrtype,"ADDI");
            instr.Cycles2live=5;
            break;
        case OP_lwcl: 
            pc = INSN_lwcl(instruction); 
            strcpy(instr.Instrtype,"LWC1");
            instr.Cycles2live=4;
            break;
    ······
    }
    Producer(semId, shm, instr); 
    
    return 0;
}
```

通过上面这个函数，我把`instr`,一个Instruction类型的实例放进了共享内存



时序部分的实现主要是在`pipeline.cpp`里面

```c++
int PipeLine(int semId, struct Buffer* shm){
    //从共享内存里取已经译码好了的Instruction
    STAGE IF,DE,EXE,MEM,WB,EMPTY;//全部是空的
    FunctionalUnit IM,RF_R,RF_W,ALU,FPU,DM;//全部不忙
    //前半周期写寄存器，后半周期读寄存器
    int fasttimes=0;
    int instructionnumber=0;
    InstructionQueue newqueue;

    while(1){
    //得到译码后的指令
    if(newqueue.queue[newqueue.rear].instruction!=0x00000000||newqueue.count==0){
        
        Instruction newinstr=Consumer(semId,shm);
        newinstr.timeAvail=fasttimes;
        newqueue.enqueue(newinstr);
        
        #ifdef DEBUG
        printf("Get %s.\n", newinstr.Instrtype);
        #endif

    }
    //流水线在读入halt后还应该把里面的代码执行完
    if((newqueue.getend().instruction==0x00000000&&newqueue.count==1&&
    WB.valid==1&&MEM.valid==1&&EXE.valid==1&&DE.valid==1&&IF.valid==1)){
        printf("PipeLine HALT!!!\n");
        printf("PipeLine is EMPTY now!\n");
        printf("The number of cycles is:%d.\n",fasttimes);
        printf("The number of instructions is:%d.\n",instructionnumber);
        printf("The CPI if my model pipeline is:%f.\n",float(fasttimes)/float(instructionnumber));
        break;
    } 

    //时钟周期++
    fasttimes++;

    //指令执行完毕WB=>
    if(WB.valid==0){
        WB=EMPTY;
    }

    //MEM=>WB
    //MEM有指令，WB指令做完了，MEM阶段的指令需要写回，
    if( MEM.valid==0 && WB.valid==1 ){
        if( MEM.instr.Cycles2live>0){
            WB=MEM;
            WB.instr.Cycles2live--;
            
        }
        MEM=EMPTY;  
    }
   
    //EXE=>MEM
    if(EXE.valid==0 && MEM.valid==1 && !DM.busy){
        if( EXE.instr.Cycles2live>0){
            MEM=EXE;
            MEM.instr.Cycles2live--;
        }
        ALU.busy=0;
        EXE=EMPTY;
    }
   

    //DE=>EXE
    if( DE.valid==0 && EXE.valid==1 && !ALU.busy){//ALU不冲突就可以了，有整数有浮点(暂不考虑)
        if(DE.instr.Cycles2live>0){
            EXE=DE;
            EXE.instr.Cycles2live--;
            ALU.busy=1;
        }
        DE=EMPTY;
    }
   

    //IF=>DE
    if(IF.valid==0 &&  DE.valid==1 
    && ((!(IF.instr.r2==DE.instr.r1 )&& !(IF.instr.r3==DE.instr.r1)) || DE.instr.instruction==0 ||IF.instr.instruction==0)){
        if(IF.instr.Cycles2live>0){
            DE=IF;
            DE.instr.Cycles2live--;
        }
        IF=EMPTY;
    }

    //newinstr=>IF
   if(newqueue.getend().instruction!=0x00000000&&
    newqueue.getend().timeAvail<=fasttimes && IF.valid==1 && !IM.busy){
        if( strcmp(DE.instr.Instrtype,"J")&& strcmp(DE.instr.Instrtype,"BEQ")){
            IF.instr=newqueue.dequeue();
            instructionnumber++;//统计指令数量
            IF.instr.Cycles2live--;
            IF.valid=0;
        }
        else{
            IF=EMPTY;
        }
    }


    pipeprint_tofile(fasttimes,IF,DE,EXE,MEM,WB);

    #ifdef DEBUG
    pipeprint(fasttimes,IF,DE,EXE,MEM,WB);
    #endif
    }
    
   
    return 0;
}
```





## 结果

结果主要存在pipeline.txt里。看起来应该行为没有问题！

然后这个是运行完毕的截图。

这下面的不会出现在pipeline.txt里面。

![image-20241021221129958](C:\Users\Lianyi\AppData\Roaming\Typora\typora-user-images\image-20241021221129958.png)





