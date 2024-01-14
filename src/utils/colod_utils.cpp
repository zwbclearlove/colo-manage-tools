#include "colod.h"
#include <stdio.h>


int get_pid_by_name(char * process_name)
{
    char line[50] = {'\0'};
    char cmd[50] = {'\0'};
    sprintf(cmd, "pidof %s", process_name);
    FILE * command = popen(cmd,"r");
    fgets(line, 50, command); 
    pid_t pid = strtoul(line, NULL, 10);
    pclose(command);
    return pid;
}

int colod_pretest() {
    char name[10] = "colod";
    pid_t pid = get_pid_by_name(name);
    if (pid == 0) {
        printf("please start colod.\n");
        return -1;
    }
    printf("colod is running with pid : %d\n", pid);
    return 0;
}

int colod_test_exist() {
    char name[10] = "colod";
    pid_t pid = get_pid_by_name(name);
    if (pid != 0) {
        
        printf("colod is already running with pid : %d\n", pid);
        return -1;
    }
    printf("start colod.\n");
    return 0;
}
 