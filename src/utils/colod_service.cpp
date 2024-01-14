#include "colod_service.h"
#include <iostream>
#include <colod.h>
#include "config_parser.h"
#include "buttonrpc.hpp"

static run_status rs;

void rs_init() {
    rs.current_status.local_status = COLO_NODE_NONE;
    if (domains_init(rs.domains) < 0) {
        return;
    }
}

// colo_manage_tools interface
colod_ret_val colod_connect_peer(std::string config_file_path) {
    std::cout << "colod_connect_peer" << std::endl;
    if (parse_config_file(config_file_path, rs.current_status) < 0) {
        return {
            -1,
            "can not parse config file.",
        };
    }
    
    // todo: send to colod
    // buttonrpc remote_client;
    // remote_client.as_client(rs.current_status.peer_ip, 5678);
    // remote_client.set_timeout(2000);
    
    // auto ret = remote_client.call<colod_ret_val>("peer-save-status", rs.current_status);

    // if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {
    //     return {
    //         -1,
    //         "connect to peer colod timeout.",
    //     };
    // }

    // colod_ret_val crv = ret.val();
    // if (crv.code < 0) {
    //     return {
    //         -1,
    //         crv.msg,
    //     };
    // }


    if (save_colo_status(rs.current_status) < 0) {
        return {
            -1,
            "cannot save colo status",
        };
    }
    return {
        0,
        "connect peer success.",
    };
}


colod_ret_val colod_connect_status(int local) {
    std::string ret_val;
    ret_val += "-------------------------------------------\n";
    ret_val += "  local status : " + colo_node_status_to_str_map[rs.current_status.local_status] + "\n";
    ret_val += "       host ip : " + rs.current_status.host_ip + "\n";
    ret_val += "     host user : " + rs.current_status.host_user + "\n";
    ret_val += "host file path : " + rs.current_status.host_file_path + "\n";
    ret_val += "       peer ip : " + rs.current_status.peer_ip + "\n";
    ret_val += "     peer user : " + rs.current_status.peer_user + "\n";
    ret_val += "peer file path : " + rs.current_status.peer_file_path + "\n";
    
    // buttonrpc remote_client;
    // remote_client.as_client(rs.current_status.peer_ip, 5678);
    // remote_client.set_timeout(2000);
    
    // auto ret = remote_client.call<colod_ret_val>("connect-test");

    // if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {
    //     ret_val << " peer status : " << "inactive" << std::endl;
    // } else {
    //     colod_ret_val crv = ret.val();
    //     if (crv.code < 0) {
    //         ret_val << " peer status : " << "wrong" << std::endl;
    //     } else {
    //         ret_val << " peer status : " << "active" << std::endl;
    //     }
    // }
    
    ret_val += "-------------------------------------------\n";
    return {
        0,
        ret_val,
    };
}


colod_ret_val colod_define(std::string vm_file_path) {
    std::cout << "colod_define" << std::endl;
    return {
        0,
        "",
    };
}


colod_ret_val colod_undefine() {
    std::cout << "colod_undefine" << std::endl;
    return {
        0,
        "",
    };
}


colod_ret_val colod_list() {
    std::cout << "colod_list" << std::endl;
    return {
        0,
        "",
    };
}


colod_ret_val colod_start() {
    std::cout << "colod_start" << std::endl;
    return {
        0,
        "",
    };
}


colod_ret_val colod_destroy() {
    std::cout << "colod_destroy" << std::endl;
    return {
        0,
        "",
    };
}


colod_ret_val colod_colo_enable() {
    std::cout << "colod_colo_enable" << std::endl;
    return {
        0,
        "",
    };
}

colod_ret_val colod_colo_disable() {
    std::cout << "colod_colo_disable" << std::endl;
    return {
        0,
        "",
    };
}


colod_ret_val colod_vm_status() {
    std::cout << "colod_vm_status" << std::endl;
    return {
        0,
        "",
    };
}

colod_ret_val colod_set_params() {
    std::cout << "colod_set_params" << std::endl;
    return {
        0,
        "",
    };
}


colod_ret_val colod_do_failover() {
    std::cout << "colod_do_failover" << std::endl;
    return {
        0,
        "",
    };
}




// peer colod insterface
colod_ret_val colod_connect_test() {
    std::cout << "colod_connect_test" << std::endl;
    return {
        0,
        "",
    };
}

colod_ret_val colod_domain_test() {
    std::cout << "colod_domain_test" << std::endl;
    return {
        0,
        "",
    };
}

colod_ret_val peer_colod_save_status(colo_status cs) {
    std::cout << "peer_colod_save_status" << std::endl;
    return {
        0,
        "",
    };
}
