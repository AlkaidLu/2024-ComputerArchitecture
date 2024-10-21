#pragma once
#include <cstdint>
#include <string>
struct Instruction {
    uint32_t instruction;
    uint32_t opcode;
    uint32_t r1, r2, r3, r4;
    uint32_t imm, addr, funct;
    int timeAvail;
    int Cycles2live;
    char Instrtype[10];
    Instruction() : instruction(0), opcode(0), r1(0), r2(0), r3(0), r4(0),
    imm(0), addr(0), funct(0),timeAvail(0),Cycles2live(0){strcpy(Instrtype, "");}
    Instruction& operator=(const Instruction& other) {
        if (this != &other) {
            instruction = other.instruction;
            opcode = other.opcode;
            funct = other.funct;
            r1 = other.r1;
            r2 = other.r2;
            r3 = other.r3;
            r4 = other.r4;
            imm = other.imm;
            addr = other.addr;
            timeAvail = other.timeAvail;
            Cycles2live = other.Cycles2live;
            strcpy(Instrtype ,other.Instrtype);  // 使用 std::string 的赋值运算符
        }
        return *this;
    }
};