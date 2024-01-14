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
colod_ret_val colod_connect_status(int local);
colod_ret_val colod_define(std::string vm_file_path);
colod_ret_val colod_undefine();
colod_ret_val colod_list();
colod_ret_val colod_start();
colod_ret_val colod_destroy();
colod_ret_val colod_colo_enable();
colod_ret_val colod_colo_disable();
colod_ret_val colod_vm_status();
colod_ret_val colod_set_params();
colod_ret_val colod_do_failover();


// peer colod insterface
colod_ret_val colod_connect_test();
colod_ret_val colod_domain_test();
colod_ret_val peer_colod_save_status(colo_status cs);
