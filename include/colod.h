#pragma once
#include "config.h"
#include <unordered_map>

struct domain_status {
    pid_t pid;
    std::string name;
    DOMAIN_STATUS status;
    bool colo_enable;
};

struct run_status
{
	colo_status current_status;
    std::unordered_map<std::string, domain_status> domains;
};




int colod_pretest();
int colod_test_exist();
