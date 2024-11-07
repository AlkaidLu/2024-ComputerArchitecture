#pragma once

// 头文件内容
#include <vector>
#include <cstdint>
#include <cmath>
#include <array>
#include <iostream>
#include <bitset>
#include "memory.h"
#define DEBUG_CACHE

using namespace std;

template <size_t LineSize>
struct Data{
    array<uint32_t, LineSize> line; // 一行高速缓存能够存放 `LineSize` 个指令
};

enum Status{
    INVALID,
    VALID,
    DIRTY
};

struct Tag{
    int tag;
    Status status;
    int trdy;//最近访问时间
};

template <size_t blockSize>
class iCache{
private:
    int numSets;//组数
    int setSize;//相联度
    static constexpr size_t LineSize = blockSize / 4;
public:
    vector<vector<Tag>> tags;
    vector<vector<Data<LineSize>>> data;
    iCache(int numSets=64, int setSize=2) : numSets(numSets), setSize(setSize) {
        tags.resize(numSets);
        data.resize(numSets);
        for (int i = 0; i < numSets; ++i) {
            tags[i].resize(setSize);
            data[i].resize(setSize);
            for (int j = 0; j < setSize; ++j) {
                tags[i][j].status = INVALID; // 初始化状态为INVALID
                tags[i][j].tag = 0;          // 初始化tag为0
                tags[i][j].trdy = 0;         // 初始化trdy为0
            }
        }
    }

    
    bool find(uint32_t addr, int time, int &slot) {
        int indexBits = log2(numSets);
        int blkOffsetBits = log2(LineSize); 
        int tag = addr >> (indexBits + blkOffsetBits);
        int setIndex = (addr >> blkOffsetBits) & (numSets - 1); 

        if (setIndex < 0 || setIndex >= numSets) return false;

        int lruSlot = -1;
        int lruTime = time;
        for (int i = 0; i < setSize; i++) {
            Tag &t = tags[setIndex][i];
            //命中了
            if (t.tag == tag && t.status == VALID) {
                slot = i;
                t.trdy = time;
                return true;
            }
            // 找空闲行或者lru行
            if (t.status == INVALID || t.trdy < lruTime) {
                lruTime = t.trdy;
                lruSlot = i;
            }
        }
        slot = lruSlot; // Return the least recently used slot
        return false;
    }

// 插入完整缓存行 (8 条指令)
    void insert(uint32_t addr, int time, int slot, Memory& M) {
        int indexBits = log2(numSets);
        int blkOffsetBits = log2(LineSize);
        int tag = addr >> (indexBits + blkOffsetBits);
        int setIndex = (addr >> blkOffsetBits) & (numSets - 1);

        if (setIndex < 0 || setIndex >= numSets) return;
        
        array<uint32_t, LineSize> instructions;
        for(size_t i=0;i<LineSize;i++){
            instructions[i]=M.loadWord(addr+i*4);
        }
        
        tags[setIndex][slot] = {tag, VALID, time}; // Update tag and status
        data[setIndex][slot].line = instructions;  // Insert the 8 instructions
    }

    void reach_iCache(int semId, uint32_t addr, int *fasttime, Memory& M){
        int blkOffsetBits = log2(LineSize);
        int setIndex = (addr >> blkOffsetBits) & (numSets - 1);

        int slot=-1;
        int flag=find(addr, *fasttime, slot);
        if (flag) {
            #ifdef DEBUG_CACHE
            SemWait(semId,3);
            cout<<"fasttime: "<<*fasttime<<endl;
            cout << "Cache hit at set index "<<setIndex<< " with slot: " << slot << endl;
            SemSignal(semId,3);
            #endif
        } else {

            SemWait(semId,3);
            *fasttime+=50;//不命中惩罚
            SemSignal(semId,3);

            #ifdef DEBUG_CACHE
            SemWait(semId,3);
            cout<<"fasttime: "<<*fasttime<<endl;
            cout << "Cache miss, inserting at set index "<<setIndex<< " with slot: " << slot << endl;
            SemSignal(semId,3);
            #endif

            insert(addr, *fasttime, slot, M); // 缓存未命中，将数据加载到指定 slot 中
        }

    }
    
    void print_iCache(const char* filename) {
        FILE* file = fopen(filename, "w");
        if (file == NULL) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }
        // 逐行输出cache数据
        // 遍历每组，每个槽位，将标签、状态和数据写入文件
        for (int setIdx = 0; setIdx < numSets; ++setIdx) {
            for (int slot = 0; slot < setSize; ++slot) {
                const Tag& t = tags[setIdx][slot];
                const Data<LineSize>& d = data[setIdx][slot];
                
                fprintf(file, "Set %d, Slot %d: Tag = %d, Status = %d, trdy = %d\n", 
                        setIdx, slot, t.tag, t.status, t.trdy);
                
                fprintf(file, "Data: ");
                for (uint32_t instruction : d.line) {
                    fprintf(file, "%08x ", instruction);
                }
                fprintf(file, "\n");
            }
        }

        if (ferror(file)) {
            perror("Error writing to file");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        fclose(file);
    }

};
