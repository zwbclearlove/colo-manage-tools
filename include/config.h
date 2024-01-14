#pragma once
#include <unordered_map>

#define DEFAULT_CONFIG_FILE_PATH "/home/ubuntu/config/config.yaml"
#define DEFAULT_CONFIG_DOMAIN_PATH "/home/ubuntu/config/domain.yaml"
#define DEFAULT_SAVE_PATH "/home/ubuntu/config/colo_manage_tools/"
#define DEFAULT_SAVE_FILE "/home/ubuntu/config/colo_manage_tools/save.yaml"

enum COLO_NODE_STATUS {
    COLO_NODE_NONE,
    COLO_NODE_PRIMARY,
    COLO_NODE_SECONDARY,
    COLO_NODE_ERROR,
};



static std::unordered_map<std::string, COLO_NODE_STATUS> colo_node_status_map = {
    {"none", COLO_NODE_NONE},
    {"primary", COLO_NODE_PRIMARY}, 
    {"secondary", COLO_NODE_SECONDARY},
    {"error", COLO_NODE_ERROR},
};

static std::unordered_map<COLO_NODE_STATUS, std::string> colo_node_status_to_str_map = {
    {COLO_NODE_NONE, "none"},
    {COLO_NODE_PRIMARY, "primary"}, 
    {COLO_NODE_SECONDARY, "secondary"},
    {COLO_NODE_ERROR, "error"},
};

typedef struct _colo_status {
    COLO_NODE_STATUS local_status;
    std::string host_ip;
    std::string host_user;
    std::string host_file_path;
    std::string peer_ip;
    std::string peer_user;
    std::string peer_file_path;
} colo_status;

enum COLO_COMMAND_TYPE {
    COMMAND_CONNECT_PEER,
    COMMAND_CONNECT_STATUS,
    COMMAND_DEFINE,
    COMMAND_UNDEFINE,
    COMMAND_LIST,
    COMMAND_START,
    COMMAND_DESTROY,
    COMMAND_COLO_ENABLE,
    COMMAND_COLO_DISABLE,
    COMMAND_VM_STATUS,
    COMMAND_SET_PARAMS,
    COMMAND_DO_FAILOVER,
};

static std::unordered_map<std::string, COLO_COMMAND_TYPE> command_type_map = {
    {"connect-peer", COMMAND_CONNECT_PEER},
    {"connect-status", COMMAND_CONNECT_STATUS}, 
    {"define", COMMAND_DEFINE},
    {"undefine", COMMAND_UNDEFINE},
    {"list", COMMAND_LIST}, 
    {"start", COMMAND_START},
    {"destroy", COMMAND_DESTROY},
    {"colo-enable", COMMAND_COLO_ENABLE},
    {"colo-disable", COMMAND_COLO_DISABLE},
    {"vm-status", COMMAND_VM_STATUS},
    {"set-params", COMMAND_SET_PARAMS},
    {"do-failover", COMMAND_DO_FAILOVER},
};

static std::unordered_map<COLO_COMMAND_TYPE, std::string> colo_cmd_type_to_str_map = {
    {COMMAND_CONNECT_PEER, "connect-peer"},
    {COMMAND_CONNECT_STATUS, "connect-status"}, 
    {COMMAND_DEFINE, "define"},
    {COMMAND_UNDEFINE, "undefine"},
    {COMMAND_LIST, "list"}, 
    {COMMAND_START, "start"},
    {COMMAND_DESTROY, "destroy"},
    {COMMAND_COLO_ENABLE, "colo-enable"},
    {COMMAND_COLO_DISABLE, "colo-disable"},
    {COMMAND_VM_STATUS, "vm-status"},
    {COMMAND_SET_PARAMS, "set-params"},
    {COMMAND_DO_FAILOVER, "do-failover"},
};