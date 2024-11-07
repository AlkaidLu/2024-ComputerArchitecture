#define _CRT_SECURE_NO_WARNINGS
#include "MIPSSimulator.h"



MIPSSimulator::MIPSSimulator(size_t memSize) : memory(memSize), pc(0x108), clock(0), cache(64,2){}

void MIPSSimulator::load_memory(const char* filename) {
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
            memory.storeWord(memoryOffset, instruction);
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

void MIPSSimulator::save_memory(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }

    // 逐字输出内存数据
    for (size_t addr = 0; addr < memory.getsize(); addr += sizeof(uint32_t)) {
        uint32_t word = memory.loadWord(addr);
        fprintf(file, "0x%08X\n", word); // 输出格式为16进制，32位的字
    }

    if (ferror(file)) {
        perror("Error writing to file");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    fclose(file);
}

void MIPSSimulator::print_iCache(const char* filename) {
        cache.print_iCache(filename);
    }


int MIPSSimulator::INSN_addi(int instruction) {
    //printf("addi\n");
    int rs=(instruction>>21) & 0x1F;
    int rt= (instruction>>16) & 0x1F;
    int imm= instruction & 0xFFFF;
    int result = registers.getRegister(rt)+imm;
    //printf("imm: %d \n",imm);
    //printf("result: %d \n",result);
    //printf("%x\n",instruction);
    registers.setRegister(rs, result); // Multiply and store in destination
    //if(rs==0b10000){printf("$s0:%d\n",registers.getRegister(rs));}
    return pc + 4;

}

int MIPSSimulator::INSN_lwcl(int instruction) {
    //printf("lwcl\n");
    int ft=(instruction>>21) & 0x1F;
    int base=(instruction>>16) & 0x1F;
    int offset =instruction & 0xFFFF;
    uint32_t addr = registers.getRegister(base) + offset;
    float value = memory.loadFloat(addr); 
    //printf("addr: %d \n",addr);
    //printf("value: %f \n",value);
    registers.setFloatRegister(ft, value); // Load float to floating register
    return pc + 4;
}

int MIPSSimulator::INSN_swcl(int instruction) {
    //printf("swcl\n");
    int ft=(instruction>>21) & 0x1F;
    int base=(instruction>>16) & 0x1F;
    int offset =instruction & 0xFFFF;
    uint32_t addr = registers.getRegister(base) + offset;
    float value = registers.getFloatRegister(ft);
    memory.storeFloat(addr, value); // Store float from register to memory
    return pc + 4;
}

int MIPSSimulator::INSN_mul_s(int instruction) {
    //printf("mul.s\n");
    int ft=(instruction>>16) & 0x1F;
    int fs=(instruction>>11) & 0x1F;
    int fd=(instruction>>6) &0x1F;
    float result = registers.getFloatRegister(fs) * registers.getFloatRegister(fd);
    registers.setFloatRegister(ft, result); // Multiply and store in destination
    return pc + 4;
}

int MIPSSimulator::INSN_add_s(int instruction) {
    //printf("add.s\n");
    int ft=(instruction>>16) & 0x1F;
    int fs=(instruction>>11) & 0x1F;
    int fd=(instruction>>6) &0x1F;
    float result = registers.getFloatRegister(fs) + registers.getFloatRegister(fd);
    //printf("result: %f \n",result);
    registers.setFloatRegister(ft, result); // Add and store in destination
    return pc + 4;
}

int MIPSSimulator::INSN_beq(int instruction) {
    //printf("beq\n");
    int rs=(instruction>>21) & 0x1F;
    int rt=(instruction>>16) & 0x1F;
    int offset =instruction & 0xFFFF;
    //printf("rs: %x    rt: %x ;",registers.getRegister(rs),registers.getRegister(rt));
    if (registers.getRegister(rs) == registers.getRegister(rt)) {
        return pc + 4 + (offset << 2); // Branch target is relative to the PC
    }
    return pc + 4; // No branch taken
}

int MIPSSimulator::INSN_j(int instruction) {
    //printf("j\n");
    int addr =instruction & 0x3FFFFFF;
    return (((pc + 4) & 0xF0000000) )|(addr << 2); // Branch target is relative to the PC
}


int MIPSSimulator::executeInstruction(int semId,struct Buffer *shm,int * fasttimes) {
    uint32_t instruction = memory.loadWord(pc);
    //假装从icache里拿数据
    cache.reach_iCache(semId, pc, fasttimes, memory);
    Instruction instr;
    instr.instruction=instruction;
    instr.opcode = (instruction >> 26) & 0x3F;
    instr.funct = instruction & 0x3F; // 6位
    instr.r1 = (instruction >> 21) & 0x1F;
    instr.r2 = (instruction >> 16) & 0x1F;
    instr.r3 = (instruction >> 11) & 0x1F;
    instr.r4 = (instruction >> 6) & 0x1F;
    instr.imm = instruction & 0xFFFF;
    instr.addr = instruction & 0x3FFFFFF;

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
        case OP_beq: 
            pc = INSN_beq(instruction); 
            strcpy(instr.Instrtype,"BEQ");
            instr.Cycles2live=2;
            break;
        case OP_FType: 
            switch(instr.funct){
                case FUNC_mul_s:
                pc = INSN_mul_s(instruction); 
                strcpy(instr.Instrtype,"MULS");
                instr.Cycles2live=5;
                break;
                case FUNC_add_s:
                pc = INSN_add_s(instruction); 
                strcpy(instr.Instrtype,"ADDS");
                instr.Cycles2live=5;
                break;
            }
            break;
        case OP_swcl: 
            pc = INSN_swcl(instruction); 
            strcpy(instr.Instrtype,"SWC1");
            instr.Cycles2live=4;
            break;
        case OP_j: 
            pc = INSN_j(instruction); 
            strcpy(instr.Instrtype,"J");
            instr.Cycles2live=2;
            break;
        default: printf("error: unimplemented instruction\n"); printf("pc:%lx\n instruction:%x\n",pc,instruction); exit(-1);
    }
    Producer(semId, shm, instr); 
    
    return 0;
}

void MIPSSimulator::run(int semId,struct Buffer *shm,int *fasttimes) {
    while (pc < memory.getsize()) {
        if(executeInstruction(semId,shm,fasttimes)==1){
            break;
        }
    }
}
