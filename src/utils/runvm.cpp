#include "runvm.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <glib-2.0/glib.h>
#include "qemu_command.h"

int run_new_vm(qemu_command& cmd) {
    pid_t pid;
    // for (std::string& s : cmd.args) {
    //     std::cout << s << " \\\n";
    // }
    pid = fork();
    if (pid == -1) {
        perror("fork error");
        exit(1);
    } else if (pid > 0) {
        std::cout << "parent process : pid = " << getpid() << std::endl
                << "child process : pid = " << pid << std::endl;

    } else if (pid == 0) {
        //std::cout << "child process : pid = " << pid << std::endl;
        
        std::vector<char*> vchar;
        for (const auto& str : cmd.args) {
            vchar.push_back(const_cast<char*>(str.c_str()));
        }
        char **arg = new char*[cmd.nargs + 1];
        for (int i = 0; i < cmd.nargs; i++) {
            arg[i] = vchar[i];
        }
        arg[cmd.nargs] = NULL;

        execvp(cmd.binaryPath.c_str(), arg);
        perror("error exec");
        //std::cout << "child process : pid = " << pid << std::endl;
    }
    return 0;
}