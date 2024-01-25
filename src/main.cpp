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
    cmd_parser.add<std::string>("qmp-command", 'q', "qmp-command", false, "");
    cmd_parser.add("all", '\0', "show all the vms");
    cmd_parser.add("colo", '\0', "start vm in colo mode");
    cmd_parser.add<int>("checkpoint-time", '\0', "checkpoint time", false, 1000);
    cmd_parser.add<int>("compare-timeout", '\0', "compare timeout", false, 1000);
    cmd_parser.add<int>("max-queue-size", '\0', "max queue size", false, 65535);
    cmd_parser.add("restart-colod", 'r', "restart colod");
    
    cmd_parser.parse_check(argc, argv);
    if (cmd_parser.exist("restart-colod")) {
        //restart colod
        std::cout << "restart colod." << std::endl;
        return 0;
    }

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
            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_CONNECT_STATUS]);

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
            
            if (!cmd_parser.exist("vm-file")) {
                std::cout << "no vm definition file" << std::endl;
                goto cleanup;
            } else {
                vm_file_path = cmd_parser.get<std::string>("vm-file");
                std::cout << "read vm definition file : " << vm_file_path << std::endl;
            }
            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_DEFINE], vm_file_path);

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
    case COMMAND_UNDEFINE:
        {
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            // if (vm_undefine(name) < 0) {
            //     std::cerr << "can not undefine vm" << std::endl;
            //     return 0;
            // }

            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_UNDEFINE], name);

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
    case COMMAND_LIST:
        {
            bool show_all = cmd_parser.exist("all");
            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_LIST], show_all);

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
    case COMMAND_START:
        {   

            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            bool colo = cmd_parser.exist("colo");
            std::string name = cmd_parser.get<std::string>("domain");
            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_START], name, colo);

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
    case COMMAND_DESTROY:
        {
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_DESTROY], name);

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
    case COMMAND_COLO_ENABLE:
        {
            colod_ret_val crv;
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            auto ret = local_client.call<colod_ret_val>("domain-test", name);

            if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {
                std::cout << "connect to colod timeout." << std::endl;
                goto cleanup;
            }

            crv = ret.val();
            if (crv.msg.compare("shutoff") != 0) {
                std::cout << "canot colo enable a running domain." << std::endl;
                goto cleanup;
            }

            if (colo_enable(name) < 0) {
                std::cout << "can not enable colo." << std::endl;
            }    
            ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_COLO_ENABLE], name);

            if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {
                std::cout << "connect to colod timeout." << std::endl;
                goto cleanup;
            }

            crv = ret.val();
            if (crv.code < 0) {
                std::cout << crv.msg << std::endl;
                goto cleanup;
            }
            std::cout << crv.msg << std::endl;        
        }
        break;
    case COMMAND_COLO_DISABLE:
        {
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            
            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_COLO_DISABLE], name);

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
    case COMMAND_VM_STATUS:
        {
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            
            

            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_VM_STATUS], name);

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
    case COMMAND_SET_PARAMS:
        {
            if (!cmd_parser.exist("domain")) {
                std::cout << "no domain name." << std::endl;
                return 0;
            }
            std::string name = cmd_parser.get<std::string>("domain");
            int param_count = 0, param_value;
            std::string property;
            
            if (cmd_parser.exist("checkpoint-time")) {
                param_count += 1;
                
                property = "checkpoint-time";
                param_value = cmd_parser.get<int>("checkpoint-time");
            }
            if (cmd_parser.exist("compare-timeout")) {
                param_count += 1;
                
                property = "compare-timeout";
                param_value = cmd_parser.get<int>("compare-timeout");
            }
            if (cmd_parser.exist("max-queue-size")) {
                param_count += 1;    
                
                property = "max-queue-size";
                param_value = cmd_parser.get<int>("max-queue-size");
            }
            if (param_count == 0) {
                std::cout << "no params to set." << std::endl;    
                goto cleanup;
            } else if (param_count > 1) {
                std::cout << "can only set one param." << std::endl;    
                goto cleanup;
            }
            auto ret = local_client.call<colod_ret_val>(colo_cmd_type_to_str_map[COMMAND_SET_PARAMS], name, property, param_value);

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
