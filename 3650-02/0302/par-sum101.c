#include <stdio.h>
#include <assert.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <semaphore.h>

void*
malloc_shared(size_t size)
{
    return mmap(
        0, size, PROT_READ|PROT_WRITE,
        MAP_SHARED|MAP_ANONYMOUS, -1, 0
    );
}

void
free_shared(void* ptr, size_t size)
{
    munmap(ptr, size);
}

int
main(int _ac, char* _av[])
{
    // A Billion
    const long TOP = 1000000000;
    const long NPP = TOP / 10;

    printf(
        "Summing numbers divisible by 101 from 0 to %ld\n",
        TOP - 1
    );

    long* sum = malloc_shared(sizeof(long));
    *sum = 0;

    sem_t* lock = malloc_shared(sizeof(sem_t));
    sem_init(lock, 1, 1);

    pid_t kids[10];

    for (int pp = 0; pp < 10; ++pp) {
        if ((kids[pp] = fork())) {
            // do nothing
        }
        else {
            long i0 = NPP*pp;
            long iN = NPP*(pp+1);

            long local_sum = 0;

            for (long ii = i0; ii < iN; ++ii) {
                if (ii % 101 == 0) {
                    local_sum += ii;
                }
            }

            sem_wait(lock); // lock
            *sum += local_sum;
            sem_post(lock); // unlocks

            munmap(sum, sizeof(long));
            exit(0);
        }
    }

    for (long pp = 0; pp < 10; ++pp) {
        waitpid(kids[pp], 0, 0);
    }

    printf("Sum = %ld\n", *sum);

    munmap(sum, sizeof(long));
    return 0;
}

