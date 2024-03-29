#include "colod_server.h"
#include "colod_service.h"

ColodServer::ColodServer() {

}

ColodServer::ColodServer(int pid, int port) {
    this->pid = pid;
    this->port = port;
}

ColodServer::~ColodServer() {
    
}

void ColodServer::server_init() {
    this->brpc.as_server(this->port);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_CONNECT_PEER], colod_connect_peer);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_CONNECT_STATUS], colod_connect_status);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_DEFINE], colod_define);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_UNDEFINE], colod_undefine);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_LIST], colod_list);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_START], colod_start);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_DESTROY], colod_destroy);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_COLO_ENABLE], colod_colo_enable);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_COLO_DISABLE], colod_colo_disable);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_VM_STATUS], colod_vm_status);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_SET_PARAMS], colod_set_params);
    this->brpc.bind(colo_cmd_type_to_str_map[COMMAND_DO_FAILOVER], colod_do_failover);
    this->brpc.bind("connect-test", peer_colod_connect_test);
    this->brpc.bind("connect-reply", peer_colod_connect_test_reply);
    this->brpc.bind("domain-test", colod_domain_test);
    this->brpc.bind("peer-save-status", peer_colod_save_status);
    this->brpc.bind("peer-save-domain", peer_colod_save_domain);
    this->brpc.bind("peer-start-domain", peer_colod_start_domain);
    this->brpc.bind("peer-destroy-domain", peer_colod_destroy_domain);
    this->brpc.bind("peer-send-qmpcmds", peer_colod_send_qmpcmds);
    
}

void ColodServer::run() {
    this->brpc.run();
}