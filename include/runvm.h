#pragma once
#include <string>
#include "domain.h"
#include <config.h>

int run_new_vm(const std::string& domain_name, shell_command& cmd);
int destroy_domain(const std::string& domain_name);
int transfer_file(const std::string& src, const std::string& dst);