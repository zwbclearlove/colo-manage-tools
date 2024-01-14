#include "runvm.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shell_command.h"
#include "config_parser.h"

int run_shell_command_async(shell_command& cmd) {
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("fork error");
        return -1;
    } else if (pid > 0) {
        std::cout << "run async task, pid = " << pid << std::endl;
    } else if (pid == 0) {
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
    }
    return pid;
}

int run_shell_command_sync(const shell_command& cmd) {
    pid_t pid;
    int status;
    pid = fork();
    if (pid == -1) {
        perror("fork error");
        return -1;
    } else if (pid > 0) {
        std::cout << "run sync task, pid = " << pid << ", wait for process finish." << std::endl;
        if (waitpid(pid, &status, 0) > 0) {
            std::cout << "run sync task finish." << std::endl;
        } else {
            std::cout << "run sync task failed." << std::endl;
            return -1;
        }

    } else if (pid == 0) {
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
    }
    return pid;
}

int run_new_vm(const std::string& domain_name, shell_command& cmd) {
    pid_t pid;
    pid = run_shell_command_async(cmd);
    if (pid == -1) {
        perror("fork error");
        return -1;
    } else if (pid > 0) {
        std::cout << "run domain " << domain_name << " : pid = " << pid << std::endl;
        save_domain_pid(domain_name, pid);
    } else if (pid == 0) {
        perror("error child process");
    }
    return 0;
}

int destroy_domain(const std::string& domain_name) {
    int pid; 
    if (get_domain_pid(domain_name, pid) < 0) {
        return -1;
    }
    if (remove_domain_pid(domain_name) < 0) {
        return -1;
    }
    if (kill(pid, 0) < 0) {
        std::cerr << "pid is unvalid, domain is not running." << std::endl;
        return -1;
    }
    if (kill(pid, SIGKILL) < 0) {
        std::cerr << "cannot destroy domain." << std::endl;
        return -1;
    }
    return 0;
}

int transfer_file(const std::string& src, const std::string& dst) {
    pid_t pid;
    shell_command cmd;
    cmd.binaryPath = "scp";
    cmd.args = {
        "scp",
        src,
        dst,
    };
    cmd.nargs = 3;
    pid = run_shell_command_sync(cmd);
    if (pid == -1) {
        perror("fork error");
        return -1;
    } else if (pid > 0) {
        std::cout << "transfer file " << src << " success, pid = " << pid << std::endl;
    } else if (pid == 0) {
        perror("error child process");
    }
    return 0;
}
