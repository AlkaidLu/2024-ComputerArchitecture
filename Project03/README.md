# README

这是增加了指令cache的流水线作业的说明！

## 安装和运行

### 前提条件

- Linux 环境
- `g++` 编译器
- Git

## 使用说明

### 编译项目

使用 `make` 命令编译项目。

### 运行项目

运行项目时，使用以下命令：

```bash
./main "code.txt"
```

## 项目结构

项目包含以下关键文件：

- **code.asm**：用于仿真的汇编代码。
- **code.txt**：作为仿真输入的机器码文件。
- **comm.h**，**fast.h**，**memory.h**，**Instruction.h**：定义仿真器各组件的头文件。
- **MIPSSimulator.cpp**，**Pipeline.cpp**，**producer_consumer.cpp**：实现仿真器、流水线阶段和同步机制的源文件。
- **pipeline_output.txt**：包含流水线阶段信息的输出文件。
- **result.txt**：存储最终仿真结果的文本文件。
- **main.cpp**：main函数。

主要新增文件：

* **cache.txt**：包含了最终状态的cache内容的输出。
* **iCache.h**：指令cache实现的头文件。

## 代码说明

是在Project02基础上修改的。

## 指令缓存模拟器 (iCache) 说明

### 概述

本项目中的指令缓存模拟器 (`iCache`) 是一个模拟 MIPS 架构指令缓存的组件。它支持 LRU (最近最少使用) 替换策略和二路组相联映射。该模拟器是理解缓存工作原理和性能影响的关键部分。

### 代码说明

**主要内容在iCache类里**

#### `iCache` 类

`iCache` 类是模拟器的核心，负责处理缓存的查找、插入和替换逻辑。

##### 主要方法

- `find(uint32_t addr, int time, int &slot)`：查找指定地址的指令是否在缓存中，若命中则更新访问时间，否则返回用于插入的槽位。
- `insert(uint32_t addr, int time, int slot, const Memory& M)`：在缓存未命中时，从内存中加载指令并插入到指定槽位。
- `reach_iCache(int semId, uint32_t addr, int *fasttime, const Memory& M)`：封装地址查找及插入流程，控制并更新访问时间，用于模拟缓存命中和未命中的惩罚。
- `print_iCache(const char* filename)`：将缓存内容写入文件，便于调试和分析。

```cpp
class iCache{
private:
    int numSets;//组数
    int setSize;//相联度
    static constexpr size_t LineSize = blockSize / 4;
public:
    vector<vector<Tag>> tags;
    vector<vector<Data<LineSize>>> data;
    iCache(int numSets=64, int setSize=2) : numSets(numSets), setSize(setSize) {}
    bool find(uint32_t addr, int time, int &slot) {}
    void insert(uint32_t addr, int time, int slot, const Memory& M) {}
    void reach_iCache(int semId, uint32_t addr, int *fasttime, const Memory& M){}
};
```

### 功能特点

- **LRU 替换策略**：当缓存满时，最近最少使用的缓存行将被替换。
- **二路组相联映射**：缓存被分为多个组，每组有两个槽位，可以存储两条指令。
- **缓存行大小可配置**：每行可容纳的指令数量 (`LineSize`) 可以根据需要配置。(在pipeline.h里的class )
  ```cpp
  // 流水线类
  class Pipeline {
  private:
      STAGE IF, DE, EXE, MEM, WB, EMPTY;
      FunctionalUnit IM, RF_R, RF_W, ALU, FPU, DM;
      int  fasttimes;
      iCache<16> cache;
      int instructionnumber;
      InstructionQueue newqueue;

  public:
      Pipeline() : fasttimes(0), cache(4,2), instructionnumber(0) {
          newqueue.queue.resize(1000);
      }

  void executeCycle(int semId, Buffer* shm, const Memory& memory);
  void pipeprint_tofile( STAGE IF, STAGE DE, STAGE EXE, STAGE MEM, STAGE WB);
  void pipeprint(STAGE IF,STAGE DE,STAGE EXE,STAGE MEM,STAGE WB);
  void print_iCache(const char* filename);
  };
  ```
- **缓存状态管理**：每个缓存行都有有效的状态标记，包括 `INVALID`、`VALID` 和 `DIRTY`。

### `fasttimes` 一致性说明

`fasttimes` 是一个用于跟踪模拟器执行进度的变量，它记录了从模拟开始以来的时钟周期数。这个变量在整个模拟器中被多个组件共享，以确保所有部分都能同步地更新和访问。使用了互斥锁以保证读写的同步性。

以下是 `fasttimes` 读写的关键点：

* **同步更新** ：在每个时钟周期结束时，`fasttimes` 会被递增，以反映模拟时间的流逝。
* **缓存命中和未命中** ：在 `reach_iCache` 方法中，当发生缓存未命中时，`fasttimes` 会被用来增加额外的惩罚周期，以模拟加载数据到缓存所需的额外时间。
* **流水线同步** ：在流水线模拟器中，`fasttimes` 确保了不同阶段的指令能够在正确的时间执行和完成。
* **结果验证** ：`fasttimes` 可以用来验证模拟器是否按照预期执行，以及是否所有指令都在正确的时钟周期内完成。

### 查看结果

缓存的输出结果将被写入到 `cache.txt` 文件中，可以查看该文件了解缓存的详细状态。
