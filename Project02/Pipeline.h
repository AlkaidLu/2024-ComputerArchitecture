#pragma once

// 头文件内容
#include"producer_consumer.h"
#include"MIPSSimulator.h"
#include<string>

//定义指令和流水线参数
enum InstructionType {
    INT,
    FP,
    BRANCH,
    MEMORY_ACCESS
};

struct STAGE{
    int valid;
    struct  Instruction instr;
    STAGE(): valid(1),instr() {};
    STAGE& operator=(const STAGE& other) {
        if (this != &other) {
            // 复制valid成员变量
            valid = other.valid;
            
            // 利用Instruction的赋值运算符来复制instr成员变量
            instr = other.instr;
        }
        return *this;
    }
};
struct FunctionalUnit {
    int busy;
    int timeAvailable;
    FunctionalUnit() : busy(0), timeAvailable(0) {} // 使用花括号进行初始化

    FunctionalUnit& operator=(const FunctionalUnit& other) {
        if (this != &other) {
            busy = other.busy;
            timeAvailable = other.timeAvailable;
        }
        return *this; // 通常我们会添加const关键字
    }
};

struct InstructionQueue {
    Instruction queue[1000];
    int front;  // 队列头部索引
    int rear;   // 队列尾部索引
    int count;  // 队列中元素的数量

    InstructionQueue() : front(0), rear(0), count(0) {}

    bool isFull() {
        return count == 1000;
    }

    bool isEmpty() {
        return count == 0;
    }

    void enqueue(const Instruction& instr) {
        if (isFull()) {
            // 队列已满，不能加入新的指令
            printf("Queue is full!\n");
            return;
        }
        queue[rear] = instr;
        rear = (rear + 1) % 1000;
        ++count;
    }

    Instruction dequeue() {
        if (isEmpty()) {
            // 队列为空，没有指令可以出队
            printf("Queue is empty!\n");
            exit(EXIT_FAILURE);  // 或者返回一个错误指示
        }
        Instruction instr = queue[front];
        front = (front + 1) % 1000;
        --count;
        return instr;
    }
    Instruction getend(){
        if (isEmpty()) {
            // 队列为空，没有指令可以出队
            printf("Queue is empty!\n");
            exit(EXIT_FAILURE);  // 或者返回一个错误指示
        }
        Instruction instr = queue[front];
        return instr;
    }
};



void pipeprint(int fasttimes,STAGE IF,STAGE DE,STAGE EXE,STAGE MEM,STAGE WB);
int PipeLine(int semId,struct Buffer *shm);