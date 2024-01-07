#include "config_parser.h"
#include "yaml-cpp/yaml.h"
#include <fstream>


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

    if (!config["colo"]["host_ip"].IsDefined()) {
        std::cerr << "cannot find colo host_ip." << std::endl;
        return -1;
    }
    cs.host_ip = config["colo"]["host_ip"].as<std::string>();

    if (!config["colo"]["peer_ip"].IsDefined()) {
        std::cerr << "cannot find colo peer_ip." << std::endl;
        return -1;
    }
    cs.peer_ip = config["colo"]["peer_ip"].as<std::string>();
    return 0;
}

int save_colo_status(const colo_status& cs) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    std::ofstream fout(DEFAULT_SAVE_FILE);
    YAML::Node ss;
    ss["local_status"] = colo_node_status_to_str_map[cs.local_status];
    ss["host_ip"] = cs.host_ip;
    ss["peer_ip"] = cs.peer_ip;
    sf["save_status"] = ss;
    // if (!sf["save_status"].IsDefined()) {
        
    //     ss["local_status"] = colo_node_status_to_str_map[cs.local_status];
    //     ss["host_ip"] = cs.host_ip;
    //     ss["peer_ip"] = cs.peer_ip;
    //     sf["save_status"] = ss;
    // } else {
        
    //     ss["local_status"] = colo_node_status_to_str_map[cs.local_status];
    //     ss["host_ip"] = cs.host_ip;
    //     ss["peer_ip"] = cs.peer_ip;
    //     sf["save_status"] = ss;
    // }
    
    fout << sf;
    fout.close();
    return 0;
}

int get_connect_status() {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    std::cout << "-------------------------------------------\n";
    std::cout << "local status : " << sf["save_status"]["local_status"].as<std::string>() << std::endl;
    std::cout << "     host ip : " << sf["save_status"]["host_ip"].as<std::string>() << std::endl;
    std::cout << "     peer ip : " << sf["save_status"]["peer_ip"].as<std::string>() << std::endl;
    // todo: get peer status from colod
    std::cout << " peer status : " << "active" << std::endl;
    std::cout << "-------------------------------------------\n";
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
        std::cout << "   name : " << sf["domains"][i]["name"].as<std::string>() << std::endl;
        std::cout << " status : " << sf["domains"][i]["status"].as<std::string>() << std::endl;
        std::cout << "-------------------------------------------\n";
    }
    return 0;
}