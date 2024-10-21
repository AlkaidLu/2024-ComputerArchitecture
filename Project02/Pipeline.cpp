#include"Pipeline.h"
#define DEBUG

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

void pipeprint_tofile(int fasttimes, STAGE IF, STAGE DE, STAGE EXE, STAGE MEM, STAGE WB) {
    // 打开文件，追加写入
    std::ofstream outfile("pipeline_output.txt", std::ios_base::app);
    
    if (!outfile.is_open()) {
        std::cerr << "Error opening file for output." << std::endl;
        return;
    }

    // 设置每列的宽度
    int colWidth = 10;

    // 将输出写入文件
    outfile << "fasttime: " << fasttimes << "\n";
    
    // 表头，使用 setw 和 left 保持左对齐并设置固定宽度
    outfile << std::left << std::setw(colWidth) << "IF"
            << std::left << std::setw(colWidth) << "DE"
            << std::left << std::setw(colWidth) << "EXE"
            << std::left << std::setw(colWidth) << "MEM"
            << std::left << std::setw(colWidth) << "WB" << "\n";

    // 指令类型
    outfile << std::left << std::setw(colWidth) << IF.instr.Instrtype
            << std::left << std::setw(colWidth) << DE.instr.Instrtype
            << std::left << std::setw(colWidth) << EXE.instr.Instrtype
            << std::left << std::setw(colWidth) << MEM.instr.Instrtype
            << std::left << std::setw(colWidth) << WB.instr.Instrtype << "\n";

    // 剩余周期
    outfile << std::left << std::setw(colWidth) << IF.instr.Cycles2live
            << std::left << std::setw(colWidth) << DE.instr.Cycles2live
            << std::left << std::setw(colWidth) << EXE.instr.Cycles2live
            << std::left << std::setw(colWidth) << MEM.instr.Cycles2live
            << std::left << std::setw(colWidth) << WB.instr.Cycles2live << "\n";

    // 关闭文件
    outfile.close();
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