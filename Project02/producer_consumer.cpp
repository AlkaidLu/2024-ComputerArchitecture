#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>


#define SEMKEY 123
#define SHMKEY 456
#define BUFNUM 10
#define SEMNUM 3

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
    char buffer[BUFNUM];
};

void Initialize(int *returnSemId, int *returnShmId, struct Buffer **returnShm)
{
    int semId = -1, shmId = -1, values[SEMNUM] = {BUFNUM, 0, 1};

    /*  semSet[0]: empty, initial value: n
        semSet[1]: full, initial value 0
        semSet[2]: mutex, initial value 1   */

    semId = semget(SEMKEY, SEMNUM, IPC_CREAT | 0666);
    if(semId == -1)
    {
        printf("semaphore creation failed!\n");
        exit(EXIT_FAILURE);
    }

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

    shmId = shmget(SHMKEY, sizeof(struct Buffer), IPC_CREAT | 0666);
    if(shmId == -1)
    {
        printf("share memory creation failed!\n");
        exit(EXIT_FAILURE);
    }

    void *temp = NULL;
    struct Buffer *shm = NULL;
    temp = shmat(shmId, 0, 0);
    if(temp == (void *) -1)
    {
        printf("share memory attachment failed!\n");
        exit(EXIT_FAILURE);        
    }
    shm = (struct Buffer *) temp;

    shm -> start = 0;
    shm -> end = 0;
    for(i = 0; i < BUFNUM; i++)
    {
        shm -> buffer[i] = ' ';
    }

    *returnSemId = semId;
    *returnShmId = shmId;
    *returnShm = shm;
}

void Add(struct Buffer *shm)
{
    char product = 'A' + rand() % 26;
    printf("producer %d: added product %c into buffer:\t", getpid(), product);
    shm -> buffer [shm -> end] = product;
    shm -> end = (shm -> end + 1) % BUFNUM;
    printf("|%s|\n", shm -> buffer);
}

void Remove(struct Buffer *shm)
{
    char product = shm -> buffer [shm -> start];
    printf("consumer %d: removed product %c from buffer:\t", getpid(), product);
    shm -> buffer [shm -> start] = ' ';
    shm -> start = (shm -> start + 1) % BUFNUM;
    printf("|%s|\n", shm -> buffer);
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

void Producer(int semId, struct Buffer *shm)
{
    do{
        // wait empty region
        SemWait(semId, 0);
        // wait mutex
        SemWait(semId, 2);

        Add(shm);

        // signal mutex
        SemSignal(semId, 2);
        // singal full region
        SemSignal(semId, 1);

        sleep(random() % 2);

    }while(1);
}

void Consumer(int semId, struct Buffer *shm)
{
    do{
        // wait full region
        SemWait(semId, 1);
        // wait mutex
        SemWait(semId, 2);

        Remove(shm);

        // signal mutex
        SemSignal(semId, 2);
        // singal empty region
        SemSignal(semId, 0);

        sleep(random() % 2);

    }while(1);
}

int main(int argc, char *argv[])
{
    int semId = -1, shmId = -1, i=0;
    int processNum = atoi(argv[2]);
    if(processNum <= 0) processNum = 1;
    struct Buffer *shm = NULL;

    Initialize(&semId, &shmId, &shm);
    for(i = 0; i < 2 * processNum; i ++)
    {
        pid_t pid = fork();
        if(pid < 0)
        {
            printf("fork failed!\n");
            exit(EXIT_FAILURE);
        }
        else if(pid == 0)
        {
            sleep(1);
            if(i % 2 == 0)
            {
                printf("producer process %d created\n", getpid());
                Producer(semId, shm);            
            }
            else
            {
                printf("consumer process %d created\n", getpid());
                Consumer(semId, shm);
            }
            return 0;
        }
    }
    getchar();
    Destroy(semId, shmId, shm);
    return 0;
}
