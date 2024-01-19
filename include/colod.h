#pragma once
#include "config.h"
#include "Serializer.hpp"
#include <unordered_map>
#include <stdlib.h>
#include <stdio.h>

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


inline void LOG(const std::string& log) {
    printf("%s %s : , File: %s, line: %05d, %s\n",
     __DATE__, __TIME__, __FILE__, __LINE__, log.c_str());
}