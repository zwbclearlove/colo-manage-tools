#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include "colod.h"
#include "colod_server.h"
#include "colod_service.h"

#define PID_LENGTH 12

volatile sig_atomic_t _running = 1;
 
void sigterm_handler(int arg)
{
    _running = 0;
}

int colod_daemon() {
    pid_t pc;
    int fd;
    pc = fork(); //第一步
    if(pc < 0)                                                   
    {
        exit(1);
    }
    else if(pc > 0) {
        std::cout << "colod now is running with pid = " << pc << std::endl;
        exit(0);
    }

    setsid(); //第二步
    chdir("/"); //第三步
    umask(0); //第四步
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);

        if (fd > STDERR_FILENO) {
            close(fd);
        }
    }
    signal(SIGTERM, sigterm_handler);
    return pc;
}


int main(int argc, char *argv[])
{
    int pid;
    pid = colod_daemon();
    rs_init();
    ColodServer server(pid, 5678);
    server.server_init();
    server.run();
    exit(0);
}
