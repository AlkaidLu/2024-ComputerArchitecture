#pragma once

// 头文件内容#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <string.h>
#include"Instruction.h"

#define DEBUG
#define SEMKEY 123
#define SHMKEY 456
#define BUFNUM 10
#define SEMNUM 3
#define Type Instruction
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
/*   union   semun   is   defined   by   including   <sys/sem.h>   */ 
#else 
/*   according   to   X/OPEN   we   have   to   define   it   ourselves   */ 
union semun
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};
#endif

struct Buffer
{
    int start, end;
    Type buffer[BUFNUM];
};

void Initialize(int *returnSemId, int *returnShmId, struct Buffer **returnShm);
void Add(struct Buffer *shm);
Type Remove(struct Buffer *shm);
void ShmDestroy(int semId, struct Buffer * shm);
void SemWait(int semId, int semNum);
void SemSignal(int semId, int semNum);
void SemDestroy(int semId);
void Destroy(int semId, int shmId, struct Buffer *shm);
void Producer(int semId, struct Buffer *shm, Type buf);
Type Consumer(int semId, struct Buffer *shm);

