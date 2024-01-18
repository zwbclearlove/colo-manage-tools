#pragma once
#include "config.h"
#include "Serializer.hpp"
#include <unordered_map>

struct colod_domain_status {
    pid_t pid;
    std::string name;
    DOMAIN_STATUS status;
    bool colo_enable;
    COLO_DOMAIN_STATUS colo_status;
    std::string config_file_path;

    // must implement
	friend Serializer& operator >> (Serializer& in, colod_domain_status& cds) {
		in >> cds.pid >> cds.name >> cds.status >> cds.colo_enable 
        >> cds.colo_status >> cds.config_file_path;
		return in;
	}
	friend Serializer& operator << (Serializer& out, colod_domain_status cds) {
        out << cds.pid << cds.name << cds.status << cds.colo_enable 
        << cds.colo_status << cds.config_file_path;
		return out;
	} 
};

struct run_status
{
	colo_status current_status;
    std::unordered_map<std::string, colod_domain_status> domains;
};




int colod_pretest();
int colod_test_exist();
