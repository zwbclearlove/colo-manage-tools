#pragma once
#include <unistd.h>
#include <string>
#include <vector>
#include <config.h>

typedef struct _shell_command
{
    
    int has_error; /* 0 on success, -1 on error  */

    std::string binaryPath; /* only valid if args[0] isn't absolute path */
    std::vector<std::string> args;
    int nargs;

} shell_command;

void shell_command_add_arg(shell_command& qc, const std::string& arg);
void shell_command_add_args(
    shell_command& qc, const std::string& arg1, const std::string& arg2);

