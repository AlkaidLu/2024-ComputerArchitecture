#ifndef MIPSSIMULATOR_H
#define MIPSSIMULATOR_H

#include <cstdint>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "memory.h"
#include "fast.h"

class MIPSSimulator {
private:
    Memory memory;
    RegisterFile registers;
    size_t pc;
    int clock;

public:
    MIPSSimulator(size_t memSize);

    void load_memory(const char* filename);
    void save_memory(const char* filename);
    int INSN_lwcl(int instruction);
    int INSN_addi(int instruction);
    int INSN_beq(int instruction);
    int INSN_mul_s(int instruction);
    int INSN_add_s(int instruction);
    int INSN_swcl(int instruction);
    int INSN_j(int instruction);
    int executeInstruction();
    void run();

    void print_memory();
};

#endif // MIPSSIMULATOR_H