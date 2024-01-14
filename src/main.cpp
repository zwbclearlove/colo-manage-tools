#include <iostream>
#include "yaml-cpp/yaml.h"
#include "commandline.h"
#include "shell_command.h"
#include "config.h"
#include "domain.h"
#include "config_parser.h"
#include "runvm.h"
#include "colod.h"
#include "colod_service.h"


int main(int argc, char *argv[]) {  
    if (colod_pretest() < 0) {
        return 0;
    }
    cmdline::parser cmd_parser;
    cmd_parser.add<std::string>("command-type", 't', "command type", true, "");
    cmd_parser.add<std::string>("vm-file", 'v', "vm definition file", false, "");
    cmd_parser.add<std::string>("config-file", 'c', "configuration file", false, "");
    cmd_parser.add<std::string>("domain", 'd', "domain name", false, "");
    cmd_parser.add("all", '\0', "show all the vms");
    cmd_parser.add<int>("checkpoint-time", '\0', "checkpoint time", false, 1000);
    cmd_parser.add<int>("compare-timeout", '\0', "compare timeout", false, 1000);
    cmd_parser.add<int>("max_queue_size", '\0', "max_queue_size", false, 65535);
    

    // YAML::Node config = YAML::LoadFile("/home/ubuntu/config/config.yaml");
    // const std::string username = config["username"].as<std::string>();
    // const std::string password = config["password"].as<std::string>();
    // std::cout << username << "---" << password << std::endl;
    cmd_parser.parse_check(argc, argv);

    buttonrpc local_client;
	local_client.as_client("127.0.0.1", 5678);
	local_client.set_timeout(2000);

    std::string cur_command_type_str = cmd_parser.get<std::string>("command-type");

    if (!command_type_map.count(cur_command_type_str)) {
        std::cout << "command " << cur_command_type_str << " is not defined!" << std::endl;
        return 0;
    } 
    COLO_COMMAND_TYPE cur_command_type = command_type_map[cur_command_type_str];
    
    switch (cur_command_type)
    {
    case COMMAND_CONNECT_PEER:
        {
            std::string config_file_path = DEFAULT_CONFIG_FILE_PATH;
            if (!cmd_parser.exist("config-file")) {
                std::cout << "read deafult config file" << std::endl;
            } else {
                config_file_path = cmd_parser.get<std::string>("config-file");
                std::cout << "read custom config file : " << config_file_path << std::endl;
            }
            
            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_CONNECT_PEER], config_file_path);

            if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {
                std::cout << "connect to colod timeout." << std::endl;
                goto cleanup;
            }

            colod_ret_val crv = ret.val();
            if (crv.code < 0) {
                std::cout << crv.msg << std::endl;
                goto cleanup;
            }
            std::cout << crv.msg << std::endl;
            //std::cout << "connect peer success." << std::endl;

        }
        break;

    case COMMAND_CONNECT_STATUS:
        {
            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_CONNECT_STATUS], 0);

            if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {
                std::cout << "connect to colod timeout." << std::endl;
                goto cleanup;
            }

            colod_ret_val crv = ret.val();
            if (crv.code < 0) {
                std::cout << crv.msg << std::endl;
                goto cleanup;
            }
            std::cout << crv.msg << std::endl;
            
        }
        break;
    case COMMAND_DEFINE:
        {
            std::string vm_file_path;
            domain d;
            if (!cmd_parser.exist("vm-file")) {
                std::cout << "no vm definition file" << std::endl;
                return 0;
            } else {
                vm_file_path = cmd_parser.get<std::string>("vm-file");
                std::cout << "read vm definition file : " << vm_file_path << std::endl;
            }
            if (vm_define(vm_file_path, d) < 0) {
                std::cerr << "can not define vm" << std::endl;
                return 0;
            }

            // YAML::Node config = YAML::LoadFile(vm_file_path);
            // todo: add a vm to colod 
        }
        break;
    case COMMAND_UNDEFINE:
        {
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            if (vm_undefine(name) < 0) {
                std::cerr << "can not undefine vm" << std::endl;
                return 0;
            }
        }
        break;
    case COMMAND_LIST:
        {
            bool show_all = cmd_parser.exist("all");

            if (get_domain_list(show_all) < 0) {
                std::cout << "cannot save colo status" << std::endl;
                return 0;
            }
        }
        break;
    case COMMAND_START:
        {   
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            domain d;
            shell_command cmd;
            if (get_domain(name, d) < 0) {
                std::cout << "can not get domain data." << std::endl;
                return 0;
            }
            // todo: get colo status from colod 
            // colo_status cs;
            // if (parse_config_file(DEFAULT_CONFIG_FILE_PATH, cs) < 0) {
            //     std::cerr << "cannot get colo status." << std::endl;
            //     return -1;
            // }
            //std::cout << "---------------------------------------" << std::endl;
            generate_vm_cmd(d, cmd);
            // std:: cout << "---------------------------------------" << std::endl;
            // generate_pvm_cmd(d, cs, cmd);  
            // std::cout << "---------------------------------------" << std::endl;
            // shell_command cmptmp;
            // generate_svm_cmd(d, cs, cmptmp);  
            // std::cout << "---------------------------------------" << std::endl;
            // for (int i = 0; i < cmptmp.nargs; i++) {
            //     std::cout << cmptmp.args[i] << " ";
            // }
            // std::cout << std::endl;
            // std::cout << "---------------------------------------" << std::endl;
            
            run_new_vm(d.name, cmd);

        }

        break;
    case COMMAND_DESTROY:
        {
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            
            if (destroy_domain(name) < 0) {
                std::cout << "can not destroy domain." << std::endl;
                return 0;
            }
            std::cout << "domain " << name <<" is destroyed." << std::endl;
        }
        break;
    case COMMAND_COLO_ENABLE:
        {
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            
            if (colo_enable(name) < 0) {
                std::cout << "can not prebuild colo domain." << std::endl;
                return 0;
            }
            std::cout << "build colo domain " << name <<" success." << std::endl;         
        }
        break;
    case COMMAND_COLO_DISABLE:
        {
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            
            if (colo_disable(name) < 0) {
                std::cout << "can not disable colo domain." << std::endl;
                return 0;
            }
            std::cout << "domain " << name <<" is already disabled." << std::endl;        
        }
        break;
    case COMMAND_VM_STATUS:
        {
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            
            if (get_domain_status(name) < 0) {
                std::cout << "can not get domain status." << std::endl;
                return 0;
            }
        }
        break;
    case COMMAND_SET_PARAMS:
        {

        }
        break;
    case COMMAND_DO_FAILOVER:
        {

        }
        break;
    default:
        break;
    }
    
cleanup:
    exit(0);
}
