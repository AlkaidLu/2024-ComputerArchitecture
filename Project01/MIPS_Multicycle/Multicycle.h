#ifndef MULTICYCLE_CPU_H
#define MULTICYCLE_CPU_H

#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "memory.h"
#include "fast.h"
#include"PrintRegisters.h"

// 指令结构体
struct Instruction {
    uint32_t instruction;
    uint32_t opcode;
    uint32_t r1, r2, r3, r4;
    uint32_t imm, addr, func;
};

// CPU 状态的枚举类型
enum CPUState {
    FETCH, DECODE, EXECUTE, MEMORY, WRITEBACK
};

// CPU 结构体
struct CPU {
    uint32_t pc; // 程序计数器
    Memory memory; // 模拟内存
    RegisterFile registers; // 寄存器文件
    CPUState state; // 当前状态

    // 控制信号
    bool isFloat;
    bool regWrite;
    bool memRead;
    bool memWrite;

    // 指令及相关数据
    Instruction ir;  
    uint32_t srcA;
    uint32_t srcB;
    uint32_t aluResult;
    float floatA;
    float floatB;
    float floatResult;
    float memFloatdata;
    int memData;
    float FloatDatareg2mem;
    int Datareg2mem;

    // 构造函数
    CPU(size_t memSize);
};

// 多周期 CPU 类
class MultiCycleCPU {
public:
    CPU cpu;      // CPU 实例
    size_t clock; // 时钟计数

    MultiCycleCPU(size_t memSize);
    void print_RegisterByName(const std::string& regName);
    // 加载和保存内存
    void load_memory(const char* filename);
    void save_memory(const char* filename);

    // 主运行函数
    void run();

private:
    // 各个阶段的操作
    void fetch();
    int decode();
    void execute();
    void memoryAccess();
    void writeBack();
};

#endif // MULTICYCLE_CPU_H
