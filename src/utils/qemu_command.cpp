#include "qemu_command.h"


void qemu_command_add_arg(qemu_command& qc, const std::string& arg) {
    qc.args.push_back(arg);
    qc.nargs++;
}

void qemu_command_add_args(
    qemu_command& qc, const std::string& arg1, const std::string& arg2) {
    qc.args.push_back(arg1);
    qc.args.push_back(arg2);
    qc.nargs += 2;
}