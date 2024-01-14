#pragma once
#include "config.h"
#include <vector>

struct domain_run_status {
    pid_t pid;
    std::string name;
    
};

struct run_status
{
	colo_status current_status;
    std::vector<domain_run_status> running_domains;
};

int colod_pretest();
int colod_test_exist();
