#pragma once

#include "config.h"
#include <string>
#include "yaml-cpp/yaml.h"

int parse_config_file(const std::string& config_filepath, colo_status& cs);
int save_colo_status(const colo_status& cs);
int get_connect_status();
int get_domain_list(bool show_all);