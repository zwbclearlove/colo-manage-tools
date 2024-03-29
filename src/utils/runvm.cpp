#include "runvm.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "shell_command.h"
#include "config_parser.h"

std::vector<char*> strlist(std::vector<std::string> &input) {
    std::vector<char*> result;

    // remember the nullptr terminator
    result.reserve(input.size()+1);

    std::transform(begin(input), end(input),
                   std::back_inserter(result),
                   [](std::string &s) { return s.data(); }
                  );
    result.push_back(nullptr);
    return result;
}

int run_shell_command_async(shell_command& cmd) {
    pid_t pid;
    pid = fork();
    if (pid == -1) {
        perror("fork error");
        return -1;
    } else if (pid > 0) {
        std::cout << "run async task, pid = " << pid << std::endl;
    } else if (pid == 0) {
        int fd;
        setsid(); //第二步
        chdir("/"); //第三步
        umask(0); //第四步
        // if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        //     dup2(fd, STDIN_FILENO);
        //     dup2(fd, STDOUT_FILENO);
        //     dup2(fd, STDERR_FILENO);

        //     if (fd > STDERR_FILENO) {
        //         close(fd);
        //     }
        // }
        std::vector<char*> vchar;
        vchar = strlist(cmd.args);
        execvp(cmd.binaryPath.c_str(), vchar.data());
        perror("error exec");
    }
    return pid;
}

int run_shell_command_sync(shell_command& cmd) {
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
        int fd;
        setsid(); //第二步
        chdir("/"); //第三步
        umask(0); //第四步
        std::vector<char*> vchar;
        vchar = strlist(cmd.args);
        execvp(cmd.binaryPath.c_str(), vchar.data());
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
        save_domain_pid(domain_name, pid);
    } else if (pid == 0) {
        perror("error child process");
    }
    return pid;
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
        return -1;
    }
    if (kill(pid, SIGKILL) < 0) {
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
