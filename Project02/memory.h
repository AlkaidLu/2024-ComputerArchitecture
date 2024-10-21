#pragma once

// 头文件内容
#include <vector>
#include <cstdint>
#include <cmath>
#include <array>
#include <iostream>
#include <bitset>
//32位小端机器
// 定义一个模拟存储器的类
using namespace std;
const int NUM_REGISTERS = 32;
const int NUM_FLOAT_REGISTERS = 32;

class Memory {
private:
    vector<uint8_t> mem; // 使用vector来存储字节

public:
    // 构造函数，初始化指定大小的存储器
    Memory(size_t size) : mem(size, 0) {}

    size_t getsize(){
        return mem.size();
    }

    float loadFloat(size_t addr) const {
        // 将内存中的4个字节解释为浮点数
        float value;
        memcpy(&value, &mem[addr], sizeof(float));
        return value;
    }

    void storeFloat(size_t addr, float value) {
        // 将浮点数存储到内存中
        memcpy(&mem[addr], &value, sizeof(float));
    }
    // 按字节读写
    void storeByte(size_t addr, uint8_t value) {
        if (addr < mem.size()) {
            mem[addr] = value;
        }
    }

    uint8_t loadByte(size_t addr) const {
        if (addr < mem.size()) {
            return mem[addr];
        }
        return 0; // 如果地址超出范围，返回0
    }
    //按半字读写（32位）
    void storeHalfword(size_t addr, uint16_t value) {
        if (addr + 1 < mem.size()) {
            for (int i = 0; i < 2; ++i) {
                mem[addr + i] = (value >> (i * 8)) & 0xFF;
            }
        }
    }

    uint16_t loadHalfword(size_t addr) const {
        if (addr + 1 < mem.size()) {
            uint16_t value = 0;
            for (int i = 0; i < 2; ++i) {
                value |= (uint16_t)mem[addr + i] << (i * 8);
            }
            return value;
        }
        return 0; // 如果地址超出范围，返回0
    }

    // 按字读写（假设系统是32位的）
    void storeWord(size_t addr, uint32_t value) {
        if (addr + 3 < mem.size()) {
            for (int i = 0; i < 4; ++i) {
                mem[addr + i] = (value >> (i * 8)) & 0xFF;
            }
        }
    }

    uint32_t loadWord(size_t addr) const {
        if (addr + 3 < mem.size()) {
            uint32_t value = 0;
            for (int i = 0; i < 4; ++i) {
                value |= (uint32_t)mem[addr + i] << (i * 8);
            }
            return value;
        }
        return 0; // 如果地址超出范围，返回0
    }
    // 按双字读写（假设系统是32位的）
    void storeDouble(size_t addr, uint64_t value) {
        if (addr + 7 < mem.size()) {
            for (int i = 0; i < 8; ++i) {
                mem[addr + i] = (value >> (i * 8)) & 0xFF;
            }
        }
    }

    uint64_t loadDouble(size_t addr) const {
        if (addr + 7 < mem.size()) {
            uint32_t value = 0;
            for (int i = 0; i < 8; ++i) {
                value |= (uint64_t)mem[addr + i] << (i * 8);
            }
            return value;
        }
        return 0; // 如果地址超出范围，返回0
    }
};


// 定义一个模拟寄存器文件的类
class RegisterFile {
private:
    std::array<uint32_t, NUM_REGISTERS> registers;
    std::array<float, NUM_FLOAT_REGISTERS> floatRegisters;

public:
   RegisterFile() : registers{0}, floatRegisters{0.0f} {
        // 初始化所有寄存器为0
        for (auto& reg : registers) {
            reg = 0;
        }
        // 初始化所有浮点寄存器为0.0f
        for (auto& reg : floatRegisters) {
            reg = 0.0f;
        }
    }

    uint32_t getRegister(int regNum) const {
        return registers[regNum];
    }

    void setRegister(int regNum, uint32_t value) {
        registers[regNum] = value;
    }

    float getFloatRegister(int regNum) const {
        return floatRegisters[regNum];
    }

    void setFloatRegister(int regNum, float value) {
        floatRegisters[regNum] = value;
    }
};


