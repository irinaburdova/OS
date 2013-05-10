#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

void cas(int*, int, int);
void lock(int*, int);
void unlock(int*);
void* worker();

unsigned long sum = 0;
int mutex = 0;

int main() {
    int threads_count = 100;
    pthread_t threads[threads_count];
    int ids[threads_count];
    int error, i;

    for (i = 0; i != threads_count; ++i) {
        ids[i] = i + 1;
        error = pthread_create(&threads[i], NULL, &worker, &ids[i]);
        if (error) {
            printf("Can't create thread: %d\n", error);
            return 13;
        }
    }
    
    for (i = 0; i != threads_count; ++i) {
        error = pthread_join(threads[i], NULL);
        if (error) {
            printf("Can't join thread %d: %d", i, error);
        }
    }
    printf("%lu\n", sum);
    return 0;
}

void cas(int* value, int cmp, int new_value) {
    //printf("%d, %d, %d\n", *value, new_value, cmp);
    int result;
    // The clobber list tells gcc which registers (or memory) are changed by the asm, but not listed as an output.
    asm volatile("cmpxchg %1, %2"
        :"=a"(result)
        :"r"(new_value), "m"(*value), "0"(cmp)
        :"memory"
        );
    //printf("%d, %d, %d, %d\n", *value, new_value, cmp, result);
    return;
}

void lock(int* lock, int who) {
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 10;
    do {
        cas(lock, 0, who);
        nanosleep(&t, NULL);
    } while (*lock != who);
    return;
}

void unlock(int* lock) {
    *lock = 0;
    return;
}

void* worker(void *number) {
    int i;
    int id = *((int *) number);
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 10;
    for (i = 0; i != 500; ++i) {
        lock(&mutex, id);
        //printf("TID: %d\n", id);
        ++sum;
        nanosleep(&t, NULL);
        unlock(&mutex);
    }
    return NULL;
}
