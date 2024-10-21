#include"Pipeline.h"
#define DEBUG
int PipeLine(int semId, struct Buffer* shm){
    //从共享内存里取已经译码好了的Instruction
    STAGE IF,DE,EXE,MEM,WB,EMPTY;//全部是空的
    FunctionalUnit IM,RF_R,RF_W,ALU,FPU,DM;//全部不忙
    //前半周期写寄存器，后半周期读寄存器
    int fasttimes=0;
    InstructionQueue newqueue;

    while(1){
    fasttimes++;
    //指令获取和译码
    Instruction newinstr=Consumer(semId,shm);
    #ifdef DEBUG
    printf("Get %s.\n", newinstr.Instrtype);
    #endif
    newqueue.enqueue(newinstr);
    newinstr.timeAvail=fasttimes;

    if(newqueue.getend().instruction==0x00000000&&newqueue.count==1){
        printf("PipeLine HALT!!!\n");
        break;
    } 

    //指令执行完毕WB=>
    if(WB.valid==0){
        WB=EMPTY;
    }
    //MEM=>WB
    //MEM有指令，WB指令做完了，MEM阶段的指令需要写回，
    if( WB.valid==1 ){
        if(MEM.valid==0 && MEM.instr.Cycles2live>0){
            WB=MEM;
            WB.instr.Cycles2live--;
            MEM.valid=1;
        }
        else{
            WB=EMPTY;
        }
        
    }
   
    //EXE=>MEM
    if(  MEM.valid==1 && !DM.busy){
        if( EXE.valid==0 &&EXE.instr.Cycles2live>0){
            MEM=EXE;
            MEM.instr.Cycles2live--;
        }
        else{
            MEM=EMPTY;
        }
        ALU.busy=0;
        EXE.valid=1;
    }
   

    //DE=>EXE
    if( EXE.valid==1 && !ALU.busy){//ALU不冲突就可以了，有整数有浮点(暂不考虑)
        if(DE.valid==0 && DE.instr.Cycles2live>0){
            EXE=DE;
            EXE.instr.Cycles2live--;
            ALU.busy=1;
        }
        else{
            EXE=EMPTY;
        }
        DE.valid=1;
    }
   

    //IF=>DE
    if( DE.valid==1 
    && ((!(IF.instr.r2==DE.instr.r1 )&& !(IF.instr.r3==DE.instr.r1)) || DE.instr.instruction==0 ||IF.instr.instruction==0)){
        if(IF.valid==0 ){
            DE=IF;
            DE.instr.Cycles2live--;
            
        }
        else{
            DE=EMPTY;
        }
        IF.valid=1;
    }

    //newinstr=>IF
   if( newqueue.getend().timeAvail<=fasttimes && IF.valid==1 && !IM.busy){
        if( strcmp(DE.instr.Instrtype,"J")&& strcmp(DE.instr.Instrtype,"BEQ")){
            IF.instr=newqueue.dequeue();
            IF.instr.Cycles2live--;
            IF.valid=0;
        }
        else{
            IF=EMPTY;
        }
    }



    #ifdef DEBUG
    pipeprint(fasttimes,IF,DE,EXE,MEM,WB);
    #endif
    }
    
   
    return 0;
}


void pipeprint(int fasttimes,STAGE IF,STAGE DE,STAGE EXE,STAGE MEM,STAGE WB){
    printf("fasttime:%d\n",fasttimes);
    printf("IF\tDE\tEXE\tMEM\tWB\n") ;
    printf("%s\t%s\t%s\t%s\t%s\n",IF.instr.Instrtype,
    DE.instr.Instrtype,EXE.instr.Instrtype,
    MEM.instr.Instrtype,WB.instr.Instrtype);
    printf("%d\t%d\t%d\t%d\t%d\n",IF.instr.Cycles2live,
    DE.instr.Cycles2live,EXE.instr.Cycles2live,
    MEM.instr.Cycles2live,WB.instr.Cycles2live);
}