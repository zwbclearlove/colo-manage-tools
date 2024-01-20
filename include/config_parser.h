#pragma once

#include "config.h"
#include <string>
#include "yaml-cpp/yaml.h"
#include "colod.h"

int parse_config_file(const std::string& config_filepath, colo_status& cs);
int save_colo_status(const colo_status& cs);
int get_connect_status();
int get_connect_status(colo_status& cs);
int get_domain_list(bool show_all);

int domains_init(std::unordered_map<std::string, colod_domain_status>& ds_map);
int save_domain_pid(const std::string& domain_name, int pid);
int remove_domain_pid(const std::string& domain_name);
int set_domain_colo_enable(const std::string& domain_name);
int set_domain_colo_disable(const std::string& domain_name);
