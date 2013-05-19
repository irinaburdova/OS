#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define ERROR_EXIT 3

int main() {
    int ep, s, fd;
    char* addr;
    struct sockaddr_in socket_info;
    struct epoll_event ev, events[10];

    /* Map file to memory */
    fd = open("./data", O_RDONLY);
    if (fd == -1) {
        perror("Error while open data file");
        exit(ERROR_EXIT);
    }
    addr = mmap(NULL, 10, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("Error while mmap data file");
        exit(ERROR_EXIT);
    }

    /* Create, bind and listen server socket */
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        perror("Error while create socket");
        exit(ERROR_EXIT);
    }
    memset(&socket_info, 0, sizeof(struct sockaddr_in));
    socket_info.sin_family = AF_INET;
    socket_info.sin_port = htons(2013);
    if (bind(s, (struct sockaddr *) &socket_info, sizeof(struct sockaddr_in)) == -1) {
        perror("Error while bind socket");
        exit(ERROR_EXIT);
    }
    if (listen(s, 50) == -1) {
        perror("Error while listen socket");
        exit(ERROR_EXIT);
    }

    /* Create epoll and add socket for monitoring */
    ep = epoll_create(100);
    if (ep == -1) {
        perror("Error while create epoll");
        exit(ERROR_EXIT);
    }
    ev.events = EPOLLIN;
    ev.data.fd = s;
    if (epoll_ctl(ep, EPOLL_CTL_ADD, s, &ev) == -1) {
        perror("Error while add socket to epoll");
        exit(ERROR_EXIT);
    }

    /* Loop */
    while(1) {
        int n = epoll_wait(ep, events, 10, -1);
        if (n == -1) {
            perror("Errore while epoll wait");
            exit(ERROR_EXIT);
        }
        int i;
        for(i = 0; i != n; ++i) {
            if (events[i].data.fd == s) {
                /* Event on server socket, can accept */
                fputs("Event on listen socket!\n", stderr);
                int c;
                c = accept(s, NULL, 0);
                if (c == -1) {
                    perror("Error while accept client");
                    break;
                }
                /* Add client socket to epoll */
                ev.events = EPOLLIN;
                ev.data.fd = c;
                if (epoll_ctl(ep, EPOLL_CTL_ADD, c, &ev) == -1) {
                    perror("Error while add client socket to epoll");
                    exit(ERROR_EXIT);
                }
            } else {
                /* Event on client socket, can read req and write res */
                fputs("Event on client socket!\n", stderr);
                char buf[50];
                char * data = "test\n";
                int size;
                size = recv(events[i].data.fd, buf, 50, 0);
                if (size == 0) {
                    /* Socket was closed, remove it from epoll */
                    ev.data.fd = events[i].data.fd;
                    ev.events = EPOLLIN;
                    if (epoll_ctl(ep, EPOLL_CTL_DEL, events[i].data.fd, &ev) == -1) {
                        perror("Error while del client socket to epoll");
                        exit(ERROR_EXIT);
                    }
                } else {
                    send(events[i].data.fd, addr, 10, 0);
                }
            }
        }
    }
    return 0;
}
