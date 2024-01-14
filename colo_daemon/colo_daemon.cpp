#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>

#define PID_LENGTH 12

volatile sig_atomic_t _running = 1;
 
void sigterm_handler(int arg)
{
    _running = 0;
}

int get_pid_by_name(char * process_name)
{
    char line[PID_LENGTH] = {'\0'};
    char cmd[50] = {'\0'};
    sprintf(cmd, "pidof %s", process_name);
    FILE * command = popen(cmd,"r");
    fgets(line, PID_LENGTH, command); 
    pid_t pid = strtoul(line, NULL, 10);
    pclose(command);
    return pid;
}

int colod_pretest() {
    pid_t pid = get_pid_by_name("colod");
    if (pid == 0) {
        printf("please start colod.\n");
        return -1;
    }
    printf("colod is running with pid : %d\n", pid);
    return 0;
}
 
int main()
{
    pid_t pc;
    time_t t;
    int i, fd, len;
    char *buf;
    if (colod_pretest() < 0) {
        return 0;
    }

    
    

    pc = fork(); //第一步
    if(pc < 0)                                                   
    {
        printf("error fork\n");
        exit(1);
    }
    else if(pc > 0) {
        printf("colod now is running with pid = %d.\n", pc);
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
    while(_running)
    {
        if((fd = open("/tmp/dameon.log", O_CREAT | O_RDWR | O_APPEND, 0666)) < 0) \
        {
            perror("open");
            exit(1);
        }
        t = time(0);
        buf = asctime(localtime(&t));
        write(fd, buf, strlen(buf));
        close(fd);
        sleep(10); 
    }
    return 0;
}
