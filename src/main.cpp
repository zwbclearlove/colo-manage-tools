#include <iostream>
#include "utils.h"
#include "yaml-cpp/yaml.h"
#include "commandline.h"
#include "config.h"

int main(int argc, char *argv[]) {
    
#ifdef DEBUG
    std::cout << "in debug mode" << std::endl;
#endif

    cmdline::parser cmd_parser;
    cmd_parser.add<std::string>("command-type", 't', "command type", true, "");
    cmd_parser.add<std::string>("vm-file", 'v', "vm definition file", false, "");
    cmd_parser.add<std::string>("config-file", 'c', "configuration file", false, "");
    cmd_parser.add<std::string>("domain", 'd', "configuration file", false, "");
    cmd_parser.add("all", '\0', "show all the vms");
    cmd_parser.add<int>("checkpoint-time", '\0', "checkpoint time", false, 1000);
    cmd_parser.add<int>("compare-timeout", '\0', "compare timeout", false, 1000);
    cmd_parser.add<int>("max_queue_size", '\0', "max_queue_size", false, 65535);
    

    // YAML::Node config = YAML::LoadFile("/home/ubuntu/config/config.yaml");
    // const std::string username = config["username"].as<std::string>();
    // const std::string password = config["password"].as<std::string>();
    // std::cout << username << "---" << password << std::endl;
    cmd_parser.parse_check(argc, argv);

    std::string cur_command_type_str = cmd_parser.get<std::string>("command-type");

    if (!command_type_map.count(cur_command_type_str)) {
        std::cout << "command " << cur_command_type_str << " is not defined!" << std::endl;
        return 0;
    } 
    COLO_COMMAND_TYPE cur_command_type = command_type_map[cur_command_type_str];
    switch (cur_command_type)
    {
    case COMMAND_CONNECT_PEER:
        /* code */
        break;

    case COMMAND_CONNECT_STATUS:

        break;
    case COMMAND_DEFINE:

        break;
    case COMMAND_UNDEFINE:

        break;
    case COMMAND_LIST:

        break;
    case COMMAND_START:

        break;
    case COMMAND_DESTROY:

        break;
    case COMMAND_COLO_ENABLE:

        break;
    case COMMAND_COLO_DISABLE:

        break;
    case COMMAND_VM_STATUS:

        break;
    case COMMAND_SET_PARAMS:

        break;
    case COMMAND_DO_FAILOVER:

        break;
    default:
        break;
    }
    

    return 0;
}
