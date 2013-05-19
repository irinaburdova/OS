#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define ERROR_EXIT 3

int main() {
    char* addr;
    int fd;

    /* Map file to memory */
    fd = open("./data", O_RDWR);
    if (fd == -1) {
        perror("Error while open data file");
        exit(ERROR_EXIT);
    }
    addr = mmap(NULL, 10, PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("Error while mmap data file");
        exit(ERROR_EXIT);
    }

    /* Periodically change data */
    while(1) {
        addr[8] = 'Q';
        if (msync(addr, 10, MS_SYNC) == -1) {
            perror("Error while sync data to file");
            exit(ERROR_EXIT);
        }
        sleep(1);
    }
}