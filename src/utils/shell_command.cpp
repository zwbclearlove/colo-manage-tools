#include "shell_command.h"


void shell_command_add_arg(shell_command& qc, const std::string& arg) {
    qc.args.push_back(arg);
    qc.nargs++;
}

void shell_command_add_args(
    shell_command& qc, const std::string& arg1, const std::string& arg2) {
    qc.args.push_back(arg1);
    qc.args.push_back(arg2);
    qc.nargs += 2;
}