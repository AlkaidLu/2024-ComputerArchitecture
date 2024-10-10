#include"Multicycle.h"

CPU::CPU(size_t memSize) 
    : pc(0x108),                    // 初始化程序计数器
      memory(memSize),               // 初始化内存
      registers(),                   // 初始化寄存器文件
      state(CPUState::FETCH),        // 初始化状态
      isFloat(false), regWrite(false), memRead(false), memWrite(false), // 初始化控制信号
      memData(0), FloatDatareg2mem(0.0f), Datareg2mem(0),// 初始化 ALU 和内存结果
      srcA(0), srcB(0), aluResult(0), // 初始化 ALU 输入
      floatA(0.0f), floatB(0.0f), floatResult(0.0f) // 初始化浮点数相关变量
{}


MultiCycleCPU::MultiCycleCPU(size_t memSize) : cpu(memSize), clock(0) {}

void MultiCycleCPU::print_RegisterByName(const std::string& regName) {
    if (regName[0] == '$') {
        if (regName[1] == 'f') {
            // 处理浮点寄存器
            int regNum = -1;
            if (regName == "$f0") regNum = FloatRegister::F0;
            else if (regName == "$f1") regNum = FloatRegister::F1;
            // 继续添加 $f2 - $f31 的映射...

            if (regNum >= 0 && regNum <= 31) {
                float value = cpu.registers.getFloatRegister(regNum);
                printf("Floating Point Register %s: %f\n", regName.c_str(), value);
            } else {
                printf("Invalid floating point register: %s\n", regName.c_str());
            }
        } else {
            // 处理整数寄存器
            int regNum = -1;
            if (regName == "$t0") regNum = IntegerRegister::T0;
            else if (regName == "$s0") regNum = IntegerRegister::S0;
            // 继续添加其他整数寄存器的映射...

            if (regNum >= 0 && regNum <= 31) {
                uint32_t value = cpu.registers.getRegister(regNum);
                printf("Integer Register %s: %u\n", regName.c_str(), value);
            } else {
                printf("Invalid integer register: %s\n", regName.c_str());
            }
        }
    } else {
        printf("Invalid register format: %s\n", regName.c_str());
    }
}

void MultiCycleCPU::load_memory(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char line[256];
    uint32_t instruction;
    size_t memoryOffset = 0x0;  // 从 0 地址开始加载

    while (fgets(line, sizeof(line), file)) {
        // 找到注释的开始
        char* comment = strstr(line, "//");
        if (comment != nullptr) {
            *comment = '\0';  // 删除注释部分
        }

        // 去掉前后空格和回车符
        char* endPtr;
        instruction = strtoul(line, &endPtr, 16);

        // 检查是否成功读取了指令/数据
        if (endPtr != line) {
            cpu.memory.storeWord(memoryOffset, instruction);
            memoryOffset += sizeof(instruction);
        }
    }

    if (ferror(file)) {
        perror("Error reading file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

void MultiCycleCPU::save_memory(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // 逐字输出内存数据
    for (size_t addr = 0; addr < cpu.memory.getsize(); addr += sizeof(uint32_t)) {
        uint32_t word = cpu.memory.loadWord(addr);
        fprintf(file, "0x%08X\n", word); // 输出格式为16进制，32位的字
    }

    if (ferror(file)) {
        perror("Error writing to file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

void MultiCycleCPU::run() {
    int flag=0;
    while (flag==0) {//halt
        clock++;
        if(clock>10000) {printf("deadcycle!");exit(-1);}
        switch (cpu.state) {
        case FETCH:
            fetch();
            cpu.state = CPUState::DECODE;
            break;
        case DECODE:
            flag = decode();
            cpu.state = CPUState::EXECUTE;
            break;
        case EXECUTE:
            execute();
            cpu.state = CPUState::MEMORY;
            break;
        case MEMORY:
            memoryAccess();
            cpu.state = CPUState::WRITEBACK;
            break;
        case WRITEBACK:
            writeBack();
            cpu.state = CPUState::FETCH;
            break;
    }
    }
    if(flag==1) printf("halt!");
}

void MultiCycleCPU::fetch() {
    // 模拟指令提取
    cpu.ir.instruction = cpu.memory.loadWord(cpu.pc);
    printf("Fetch: PC = %x \n",cpu.pc);
    cpu.pc += 4;  // 指令长度为4字节
}

int MultiCycleCPU::decode() {
    // 模拟指令解码
    // 假设我们有一个简化的指令集
    cpu.ir.opcode = (cpu.ir.instruction >> 26) & 0x3F;
    cpu.ir.r1=(cpu.ir.instruction>>21) & 0x1F;
    cpu.ir.r2= (cpu.ir.instruction>>16) & 0x1F;
    cpu.ir.r3=(cpu.ir.instruction>>11) & 0x1F;
    cpu.ir.r4=(cpu.ir.instruction>>6) & 0x1F;
    cpu.ir.func=cpu.ir.instruction & 0x3F; 
    cpu.ir.imm= cpu.ir.instruction & 0xFFFF; 
    cpu.ir.addr= cpu.ir.instruction & 0x3FFFFFF;
    if(cpu.ir.instruction==0x00000000){
        return 1;
    }
    switch (cpu.ir.opcode) {
        case OP_addi: 
            cpu.regWrite=1;
            cpu.memRead=0;
            cpu.memWrite=0;
            cpu.isFloat=0;
            cpu.srcA=cpu.registers.getRegister(cpu.ir.r2);
            break;
        case OP_lwcl: 
            cpu.regWrite=1;
            cpu.memRead=1;
            cpu.memWrite=0;
            cpu.isFloat=1;
            cpu.srcA=cpu.registers.getRegister(cpu.ir.r2);
            break;
        case OP_beq: 
            cpu.regWrite=0;
            cpu.memRead=0;
            cpu.memWrite=0;
            cpu.isFloat=0;
            cpu.srcA=cpu.registers.getRegister(cpu.ir.r1);
            cpu.srcB=cpu.registers.getRegister(cpu.ir.r2);
            break;
        case OP_FType: 
            switch(cpu.ir.func){
                case FUNC_mul_s:
                    cpu.regWrite=1;
                    cpu.memRead=0;
                    cpu.memWrite=0;
                    cpu.isFloat=1;
                    cpu.floatA=cpu.registers.getFloatRegister(cpu.ir.r3);//fs=(instruction>>11) & 0x1F;
                    cpu.floatB=cpu.registers.getFloatRegister(cpu.ir.r4);//fd=(instruction>>6) &0x1F;
                break;
                case FUNC_add_s:
                    cpu.regWrite=1;
                    cpu.memRead=0;
                    cpu.memWrite=0;
                    cpu.isFloat=1;
                    cpu.floatA=cpu.registers.getFloatRegister(cpu.ir.r3);//fs=(instruction>>11) & 0x1F;
                    cpu.floatB=cpu.registers.getFloatRegister(cpu.ir.r4);//fd=(instruction>>6) &0x1F;
                break;
            }
            break;
        case OP_swcl: 
            cpu.regWrite=0;
            cpu.memRead=0;
            cpu.memWrite=1;
            cpu.isFloat=1;
            cpu.FloatDatareg2mem=cpu.registers.getFloatRegister(cpu.ir.r1);
            //printf("Decode: swcl FloatData:%f\n",cpu.FloatDatareg2mem);
            cpu.srcA=cpu.registers.getRegister(cpu.ir.r2);
            break;
        case OP_j: 
            cpu.regWrite=0;
            cpu.memRead=0;
            cpu.memWrite=0;
            cpu.isFloat=0;
            break;
        default: printf("error: unimplemented instruction in decode session!\n");exit(-1);
}
    //printf("Decode\n");
    return 0;
}

void MultiCycleCPU::execute() {
    //printf("Execute\n");
    switch (cpu.ir.opcode) {
    case OP_addi: 
        //printf("addi\n");
        cpu.aluResult= cpu.srcA+cpu.ir.imm;
        break;
    case OP_lwcl: 
        cpu.aluResult=cpu.srcA + cpu.ir.imm;
        break;
    case OP_beq: 
        if(cpu.srcA-cpu.srcB==0)
            cpu.pc+= (cpu.ir.imm<<2);
        break;
    case OP_FType: 
        switch(cpu.ir.func){
            case FUNC_mul_s:
            cpu.floatResult=cpu.floatA * cpu.floatB;
            break;
            case FUNC_add_s:
            cpu.floatResult=cpu.floatA + cpu.floatB;
            break;
        }
        break;
    case OP_swcl: 
        cpu.aluResult=cpu.srcA + cpu.ir.imm;
        break;
    case OP_j: 
        cpu.aluResult= ((cpu.pc) & 0xF0000000) | (cpu.ir.addr << 2);
        cpu.pc = cpu.aluResult;
        break;
    default: printf("error: unimplemented instruction in excute session!\n");exit(-1);
}
}

void MultiCycleCPU::memoryAccess() {
    // 模拟内存访问
    //std::cout << "Memory Access" << std::endl;
    if (cpu.memRead) {
        if(cpu.isFloat)
            cpu.memFloatdata = cpu.memory.loadFloat(cpu.aluResult);
        else
            cpu.memData = cpu.memory.loadWord(cpu.aluResult);
    } 
    else if (cpu.memWrite) {
        if(cpu.isFloat){
            //printf("memWrite aluResult:%d\n",cpu.aluResult);
            cpu.memory.storeFloat(cpu.aluResult, cpu.FloatDatareg2mem);
            printf("float data %f is going to be stored at 0x%x\n",cpu.FloatDatareg2mem,cpu.aluResult);
        }
        else
            cpu.memory.storeWord(cpu.aluResult, cpu.Datareg2mem);
    }
    
}

void MultiCycleCPU::writeBack() {
    // 写回阶段，写入寄存器
    /*
     switch (cpu.ir.opcode) {
        case OP_addi: 
        printf("addi\n");
        printf("%x\n",cpu.ir.instruction);
        printf("%d\n",cpu.regWrite);
        break;
        default: ;
     }
     */
    if (cpu.regWrite) {
        if(cpu.isFloat){
            if(cpu.ir.opcode==OP_lwcl){
                cpu.registers.setFloatRegister(cpu.ir.r1,cpu.memFloatdata);}
            else if(cpu.ir.opcode==OP_FType)
                //printf("FloatResult:%f\n",cpu.floatResult);
                cpu.registers.setFloatRegister(cpu.ir.r2,cpu.floatResult);
        }
        else
            cpu.registers.setRegister(cpu.ir.r1, cpu.aluResult);
    
    }
    //std::cout << "Write Back" << std::endl;
}



