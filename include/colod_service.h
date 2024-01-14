#pragma once
#include "buttonrpc.hpp"
#include "config.h"

struct colod_ret_val {
    int code;
    std::string msg;


    // must implement
	friend Serializer& operator >> (Serializer& in, colod_ret_val& r) {
		in >> r.code >> r.msg;
		return in;
	}
	friend Serializer& operator << (Serializer& out, colod_ret_val r) {
		out << r.code << r.msg;
		return out;
	} 
};

void rs_init();

// colo_manage_tools interface
colod_ret_val colod_connect_peer(std::string config_file_path);
colod_ret_val colod_connect_status();
colod_ret_val colod_define(std::string vm_file_path);
colod_ret_val colod_undefine(std::string domain_name);
colod_ret_val colod_list(bool show_all);
colod_ret_val colod_start(std::string domain_name);
colod_ret_val colod_destroy(std::string domain_name);
colod_ret_val colod_colo_enable(std::string domain_name);
colod_ret_val colod_colo_disable(std::string domain_name);
colod_ret_val colod_vm_status(std::string domain_name);
colod_ret_val colod_set_params();
colod_ret_val colod_do_failover(std::string domain_name);


// peer colod insterface
colod_ret_val colod_connect_test();
colod_ret_val colod_domain_test();
colod_ret_val peer_colod_save_status(colo_status cs);
