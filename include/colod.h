#pragma once
#include "config.h"
#include <vector>

struct domain_status {
    pid_t pid;
    std::string name;
    DOMAIN_STATUS status;
    bool colo_enable;
};

struct run_status
{
	colo_status current_status;
    std::vector<domain_status> domains;
};




int colod_pretest();
int colod_test_exist();
