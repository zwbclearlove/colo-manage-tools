#pragma once
#include "commandline.h"
#include <unordered_map>

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