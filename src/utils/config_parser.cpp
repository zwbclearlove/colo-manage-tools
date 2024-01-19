#include "config_parser.h"
#include "yaml-cpp/yaml.h"
#include <fstream>
#include <iostream>


int parse_config_file(const std::string& config_filepath, colo_status& cs) {
    YAML::Node config = YAML::LoadFile(config_filepath);
    if (!config["colo"].IsDefined()) {
        std::cerr << "cannot find colo info." << std::endl;
        return -1;
    }
    if (!config["colo"]["local_status"].IsDefined()) {
        std::cerr << "cannot find colo local_status." << std::endl;
        return -1;
    }
    cs.local_status = colo_node_status_map[config["colo"]["local_status"].as<std::string>()];

    if (!config["colo"]["colod_port"].IsDefined()) {
        std::cerr << "cannot find colo colod_port." << std::endl;
        return -1;
    }
    cs.colod_port = config["colo"]["colod_port"].as<int>();


    if (!config["colo"]["host_ip"].IsDefined()) {
        std::cerr << "cannot find colo host_ip." << std::endl;
        return -1;
    }
    cs.host_ip = config["colo"]["host_ip"].as<std::string>();

    if (!config["colo"]["host_user"].IsDefined()) {
        std::cerr << "cannot find colo host_user." << std::endl;
        return -1;
    }
    cs.host_user = config["colo"]["host_user"].as<std::string>();

    if (!config["colo"]["host_file_path"].IsDefined()) {
        std::cerr << "cannot find colo host_file_path." << std::endl;
        return -1;
    }
    cs.host_file_path = config["colo"]["host_file_path"].as<std::string>();

    if (!config["colo"]["peer_ip"].IsDefined()) {
        std::cerr << "cannot find colo peer_ip." << std::endl;
        return -1;
    }
    cs.peer_ip = config["colo"]["peer_ip"].as<std::string>();

    if (!config["colo"]["peer_user"].IsDefined()) {
        std::cerr << "cannot find colo peer_user." << std::endl;
        return -1;
    }
    cs.peer_user = config["colo"]["peer_user"].as<std::string>();

    if (!config["colo"]["peer_file_path"].IsDefined()) {
        std::cerr << "cannot find colo peer_file_path." << std::endl;
        return -1;
    }
    cs.peer_file_path = config["colo"]["peer_file_path"].as<std::string>();
    return 0;
}

int save_colo_status(const colo_status& cs) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    std::ofstream fout(DEFAULT_SAVE_FILE);
    YAML::Node ss;
    ss["local_status"] = colo_node_status_to_str_map[cs.local_status];
    ss["colod_port"] = cs.colod_port;
    ss["host_ip"] = cs.host_ip;
    ss["peer_ip"] = cs.peer_ip;
    ss["host_user"] = cs.host_user;
    ss["peer_user"] = cs.peer_user;
    ss["host_file_path"] = cs.host_file_path;
    ss["peer_file_path"] = cs.peer_file_path;
    sf["save_status"] = ss;  
    fout << sf;
    fout.close();
    return 0;
}

int get_connect_status() {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    std::cout << "-------------------------------------------\n";
    std::cout << "  local status : " << sf["save_status"]["local_status"].as<std::string>() << std::endl;
    std::cout << "       host ip : " << sf["save_status"]["host_ip"].as<std::string>() << std::endl;
    std::cout << "     host user : " << sf["save_status"]["host_user"].as<std::string>() << std::endl;
    std::cout << "host file path : " << sf["save_status"]["host_file_path"].as<std::string>() << std::endl;
    std::cout << "       peer ip : " << sf["save_status"]["peer_ip"].as<std::string>() << std::endl;
    std::cout << "     peer user : " << sf["save_status"]["peer_user"].as<std::string>() << std::endl;
    std::cout << "peer file path : " << sf["save_status"]["peer_file_path"].as<std::string>() << std::endl;
    // todo: get peer status from colod
    std::cout << " peer status : " << "active" << std::endl;
    std::cout << "-------------------------------------------\n";
    return 0;
}

int get_connect_status(colo_status& cs) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    cs.host_ip = sf["save_status"]["host_ip"].as<std::string>();
    cs.host_user = sf["save_status"]["host_user"].as<std::string>();
    cs.host_file_path = sf["save_status"]["host_file_path"].as<std::string>();
    cs.peer_ip = sf["save_status"]["peer_ip"].as<std::string>();
    cs.peer_user = sf["save_status"]["peer_user"].as<std::string>();
    cs.peer_file_path = sf["save_status"]["peer_file_path"].as<std::string>();
    return 0;
}

int get_domain_list(bool show_all) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    if (!sf["domains"].IsDefined() || sf["domains"].IsNull()) {
        std::cout << "no defined domain.\n";    
    }
    std::cout << "-------------------------------------------\n";
    for (int i = 0; i < sf["domains"].size(); i++) {
        bool shutoff = (sf["domains"][i]["status"].as<std::string>().compare("shutoff") == 0);
        if (!show_all && shutoff) continue;
        std::cout << "       name : " << sf["domains"][i]["name"].as<std::string>() << std::endl;
        std::cout << "     status : " << sf["domains"][i]["status"].as<std::string>() << std::endl;
        std::cout << "colo enable : " << sf["domains"][i]["colo_enable"].as<bool>() << std::endl;
        std::cout << "-------------------------------------------\n";
    }
    return 0;
}

int domains_init(std::unordered_map<std::string, colod_domain_status>& ds_map) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    if (!sf["domains"].IsDefined() || sf["domains"].IsNull()) {
        return 0;
    }
    std::cout << "-------------------------------------------\n";
    for (int i = 0; i < sf["domains"].size(); i++) {
        colod_domain_status ds;
        ds.name = sf["domains"][i]["name"].as<std::string>();
        ds.pid = sf["domains"][i]["pid"].as<int>();
        ds.colo_enable = sf["domains"][i]["colo_enable"].as<bool>();
        ds.status = domain_status_map[sf["domains"][i]["name"].as<std::string>()];
        ds.colo_status = colo_domain_status_map[sf["domains"][i]["colo_status"].as<std::string>()];
        ds_map[ds.name] = ds;
    }
    return 0;
}


int save_domain_pid(const std::string& domain_name, int pid) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    if (!sf["domains"].IsDefined() || sf["domains"].IsNull()) {
        std::cout << "no defined domain.\n";    
    }
    for (int i = 0; i < sf["domains"].size(); i++) {
        if(sf["domains"][i]["name"].as<std::string>().compare(domain_name) == 0) {
            sf["domains"][i]["pid"] = pid;
            sf["domains"][i]["status"] = "running";
            break;
        }
    }
    std::ofstream fout(DEFAULT_SAVE_FILE);
    fout << sf;
    fout.close();
    return 0;
}

int remove_domain_pid(const std::string& domain_name) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    if (!sf["domains"].IsDefined() || sf["domains"].IsNull()) {
        std::cout << "no defined domain.\n";
        return -1;
    }
    for (int i = 0; i < sf["domains"].size(); i++) {
        if(sf["domains"][i]["name"].as<std::string>().compare(domain_name) == 0) { 
            sf["domains"][i]["pid"] = -1;
            sf["domains"][i]["status"] = "shutoff";
            break;
        }
    }
    std::ofstream fout(DEFAULT_SAVE_FILE);
    fout << sf;
    fout.close();
    return 0;
}

int set_domain_colo_enable(const std::string& domain_name) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    if (!sf["domains"].IsDefined() || sf["domains"].IsNull()) {
        std::cout << "no defined domain.\n";    
    }
    for (int i = 0; i < sf["domains"].size(); i++) {
        if(sf["domains"][i]["name"].as<std::string>().compare(domain_name) == 0) {
            if (sf["domains"][i]["status"].as<std::string>().compare("shutoff") == 0) {
                sf["domains"][i]["colo_enable"] = true;
                break;
            } else {
                std::cout << "shutoff domain first.\n"; 
                return -1;
            }
            
        }
    }
    std::ofstream fout(DEFAULT_SAVE_FILE);
    fout << sf;
    fout.close();
    return 0;
}

int set_domain_colo_disable(const std::string& domain_name) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    if (!sf["domains"].IsDefined() || sf["domains"].IsNull()) {
        std::cout << "no defined domain.\n";    
    }
    for (int i = 0; i < sf["domains"].size(); i++) {
        if(sf["domains"][i]["name"].as<std::string>().compare(domain_name) == 0) {
            if (sf["domains"][i]["status"].as<std::string>().compare("shutoff") == 0) {
                sf["domains"][i]["colo_enable"] = false;
                break;
            } else {
                std::cout << "shutoff domain first.\n"; 
                return -1;
            }
            
        }
    }
    std::ofstream fout(DEFAULT_SAVE_FILE);
    fout << sf;
    fout.close();
    return 0;
}