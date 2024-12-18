#include"producer_consumer.h"

void Initialize(int *returnSemId, int *returnShmId, struct Buffer **returnShm)
{
    // 定义信号量ID和共享内存ID，初始化信号量的初始值
    int semId = -1, shmId = -1, values[SEMNUM] = {BUFNUM, 0, 1, 1};

    /*  semSet[0]: 信号量P,   empty, initial value n
        semSet[1]: 信号量V,   full,  initial value 0
        semSet[2]: 互斥信号量, mutex, initial value 1   */

     // 创建信号量集，包含SEMNUM（即3个）信号量，权限为0666
    semId = semget(SEMKEY, SEMNUM, IPC_CREAT | 0666);
    if(semId == -1)
    {
        printf("semaphore creation failed!\n");
        exit(EXIT_FAILURE);
    }

    // 设置信号量初始值
    int i = 0;
    union semun semUn;
    for(i = 0; i < SEMNUM; i ++)
    {
        semUn.val = values[i];
        if(semctl(semId, i, SETVAL, semUn) < 0)
        {
            printf("semaphore %d initialization failed!\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // 创建共享内存，大小为Buffer块大小，权限为0666
    shmId = shmget(SHMKEY, sizeof(struct Buffer), IPC_CREAT | 0666);

    if(shmId == -1)
    {
        printf("share memory creation failed!\n");
        exit(EXIT_FAILURE);
    }
    #ifdef DEBUG
     printf("share memory creation succeeded!\n");
    #endif

    // 将共享内存映射到当前进程的地址空间
    void *temp = NULL;
    struct Buffer *shm = NULL;
    temp = shmat(shmId, 0, 0);
    if(temp == (void *) -1)
    {
        printf("share memory attachment failed!\n");
        exit(EXIT_FAILURE);        
    }
     #ifdef DEBUG
     printf("share memory attachment succeeded!\n");
    #endif
    shm = (struct Buffer *) temp;
    shm -> fasttimes = 0;
    shm -> start = 0;
    shm -> end = 0;

    *returnSemId = semId;
    *returnShmId = shmId;
    *returnShm = shm;
}

void Add(struct Buffer *shm, Type buf)
{
    Type product = buf;
    shm -> buffer [shm -> end] = product;
    shm -> end = (shm -> end + 1) % BUFNUM;
}

Type Remove(struct Buffer *shm)
{
    Type product = shm -> buffer [shm -> start];
    shm -> start = (shm -> start + 1) % BUFNUM;
    return product;
}

void ShmDestroy(int semId, struct Buffer * shm)
{
    if(shmdt(shm) < 0)
    {
        printf("share memory detachment failed!\n");
        exit(EXIT_FAILURE);
    } 
    if(shmctl(semId, IPC_RMID, 0) < 0)
    {
        printf("share memory destruction failed!\n");
        exit(EXIT_FAILURE);        
    }
}

void SemWait(int semId, int semNum)
{
    struct sembuf semBuf;
    semBuf.sem_num = semNum;
    semBuf.sem_op = -1;
    semBuf.sem_flg = SEM_UNDO;
    if(semop(semId, &semBuf, 1) == -1)
    {
        printf("semaphore P operation failed!\n");
        exit(EXIT_FAILURE);
    }
}

void SemSignal(int semId, int semNum)
{
    struct sembuf semBuf;
    semBuf.sem_num = semNum;
    semBuf.sem_op = 1;
    semBuf.sem_flg = SEM_UNDO;
    if(semop(semId, &semBuf, 1) == -1)
    {
        printf("semaphore V operation failed!\n");
        exit(EXIT_FAILURE);
    }
}

void SemDestroy(int semId)
{
    union semun semUn;
    if(semctl(semId, 0, IPC_RMID, semUn) < 0)
    {
        printf("semaphore destruction failed!\n");
        exit(EXIT_FAILURE);
    }
}

void Destroy(int semId, int shmId, struct Buffer *shm)
{
    SemDestroy(semId);
    ShmDestroy(shmId, shm);
    printf("destruction finished! exit\n");
}

void Producer(int semId, struct Buffer *shm, Type buf)
{
    // wait empty region
    SemWait(semId, 0);
    // wait mutex
    SemWait(semId, 2);

    Add(shm,buf);

    // signal mutex
    SemSignal(semId, 2);
    // singal full region
    SemSignal(semId, 1);

    sleep(random() % 2);
}

Type Consumer(int semId, struct Buffer *shm)
{
    // wait full region
    SemWait(semId, 1);
    // wait mutex
    SemWait(semId, 2);

    Type product=Remove(shm);

    // signal mutex
    SemSignal(semId, 2);
    // singal empty region
    SemSignal(semId, 0);

    sleep(random() % 2);

    return product;
}