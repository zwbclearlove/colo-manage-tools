#include "colod_service.h"
#include <iostream>
#include <fstream>
#include <csignal>


#include <domain.h>
#include <runvm.h>
#include "colod.h"
#include "config_parser.h"
#include "buttonrpc.hpp"

static run_status rs;
static buttonrpc remote_client;
static bool remote_client_init;
static bool peer_init;
void rs_init() {
    rs.current_status.local_status = COLO_NODE_NONE;
    peer_init = false;
    if (domains_init(rs.domains) < 0) {
        return;
    }
    
}

DOMAIN_STATUS test_domain_status(const std::string& domain_name);

void set_remote_client(std::string ip, int port) {
    remote_client.as_client(ip, port);
    remote_client.set_timeout(20000);
}

// colo_manage_tools interface
colod_ret_val colod_connect_peer(std::string config_file_path) {
    LOG("colod_connect_peer");
    if (parse_config_file(config_file_path, rs.current_status) < 0) {
        return {
            -1,
            "can not parse config file.",
        };
    }
    LOG("current peer status : " + rs.current_status.peer_ip);
    // todo: send to colod
    set_remote_client(rs.current_status.peer_ip, rs.current_status.colod_port);
    remote_client_init = true;
    
    auto ret = remote_client.call<colod_ret_val>("peer-save-status", rs.current_status);
    if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {
        return {
            -1,
            "connect to peer colod timeout.",
        };
    }
    
    colod_ret_val crv = ret.val();
    if (crv.code < 0) {
        return {
            -1,
            crv.msg,
        };
    }
    
    std::cout << "colod_save_peer_status success" << std::endl;
    if (save_colo_status(rs.current_status) < 0) {
        return {
            -1,
            "cannot save colo status",
        };
    }
    peer_init = true;
    return {
        0,
        "connect peer success.",
    };
}


colod_ret_val colod_connect_status() {
    if (!peer_init) {
        return {
            -1,
            "connect-peer first",
        };
    }
    std::string ret_val;
    ret_val += "-------------------------------------------\n";
    ret_val += "  local status : " + colo_node_status_to_str_map[rs.current_status.local_status] + "\n";
    ret_val += "       host ip : " + rs.current_status.host_ip + "\n";
    ret_val += "     host user : " + rs.current_status.host_user + "\n";
    ret_val += "host file path : " + rs.current_status.host_file_path + "\n";
    ret_val += "       peer ip : " + rs.current_status.peer_ip + "\n";
    ret_val += "     peer user : " + rs.current_status.peer_user + "\n";
    ret_val += "peer file path : " + rs.current_status.peer_file_path + "\n";
    
    if (!remote_client_init) {
        set_remote_client(rs.current_status.peer_ip, rs.current_status.colod_port);
    }
    auto ret = remote_client.call<colod_ret_val>("connect-reply");

    if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {
        ret_val += "   peer status : inactive\n";
        peer_init = false;
    } else {
        colod_ret_val crv = ret.val();
        ret_val += "   peer status : " + crv.msg + "\n";
    }
    
    ret_val += "-------------------------------------------\n";
    return {
        0,
        ret_val,
    };
}

int save_domain_info(colod_domain_status& ds, std::string& err) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    YAML::Node dom;
    dom["name"] = ds.name;
    dom["path"] = DEFAULT_SAVE_PATH + ds.name + ".yaml";;
    dom["pid"] = -1;
    dom["status"] = "shutoff";
    dom["colo_enable"] = false;
    dom["colo_status"] = "none";
    if (!sf["domains"].IsDefined() || sf["domains"].IsNull()) {
        sf["domains"].push_back(dom);
    } else if (!sf["domains"].IsSequence()) {
        err = "wrong format in save file.";
        return -1;
    } else {
        bool redefined = false;
        for (int i = 0; i < sf["domains"].size(); i++) {
            if (ds.name.compare(sf["domains"][i]["name"].as<std::string>()) == 0) {
                err = "domain " + ds.name + " redefined"; 
                sf["domains"][i] = dom;
                redefined = true;
            }
        }
        if (!redefined) {
            sf["domains"].push_back(dom);
        }
    }
    std::ofstream fout1(DEFAULT_SAVE_FILE);
    fout1 << sf;
    fout1.close();
    return 0;
}

colod_ret_val colod_define(std::string vm_file_path) {
    domain d;
    std::string err;
    if (vm_define(vm_file_path, d, err) < 0) {
        return {
            -1,
            err,
        };
    }
    
    colod_domain_status ds;
    ds.name = d.name;
    ds.pid = -1;
    ds.status = DOMAIN_SHUT_OFF;
    ds.colo_enable = false;
    ds.colo_status = COLO_DOMAIN_NONE;
    ds.config_file_path = DEFAULT_SAVE_PATH + ds.name + ".yaml";
    if (save_domain_info(ds, err) < 0) {
        return {
            -1,
            err,
        };
    }
    rs.domains[d.name] = ds;
    return {
        0,
        "define domain : " + ds.name + " success.",
    };
}


colod_ret_val colod_undefine(std::string domain_name) {
    if (test_domain_status(domain_name) != DOMAIN_SHUT_OFF) {
        return {
            -1,
            "domain is already running.",
        };
    }

    string err;
    if (vm_undefine(domain_name, err) < 0) {
        return {
            -1,
            err,
        };
    }
    rs.domains.erase(domain_name);
    return {
        0,
        "domain : " + domain_name + " undefined",
    };
}

DOMAIN_STATUS test_domain_status(const std::string& domain_name) {
    colod_domain_status ds = rs.domains[domain_name];
    if (ds.pid == -1) {
        return DOMAIN_SHUT_OFF;
    }
    if (kill(ds.pid, 0) < 0) {
        rs.domains[domain_name].status =  DOMAIN_SHUT_OFF;
        remove_domain_pid(domain_name);
        return DOMAIN_SHUT_OFF;
    }
    return ds.status;
}


colod_ret_val colod_list(bool show_all) {
    string ret_val;
    if (rs.domains.empty()) {
        return {
            0,
            "no defined domain.",
        };
    }
    ret_val += "-------------------------------------------\n";
    for(auto it = rs.domains.begin(); it != rs.domains.end(); it++) {
        test_domain_status(it->first);
        bool shutoff = (it->second.status == DOMAIN_SHUT_OFF);
        if (!show_all && shutoff) continue;
        ret_val += "       name : " + it->second.name + "\n";
        ret_val += "     status : " + domain_status_to_str_map[it->second.status] + "\n";
        if (it->second.colo_enable) {
            ret_val += "colo enable : on\n";
            ret_val += "colo status : " + colo_domain_status_to_str_map[it->second.colo_status] + "\n";
        } else {
            ret_val += "colo enable : off\n";
        }
        
        ret_val += "-------------------------------------------\n";
    }

    return {
        0,
        ret_val,
    };
}

int send_qmp_cmds(std::vector<std::string> qmp_cmds) {
    qmp_socket qs("127.0.0.1", 4444);
    int ret = qs.qmp_connect();
    if (ret < 0) {
        LOG("qmp socket connect failed.");
        return ret;
    }
    std::string msg;
    ret = qs.qmp_recv(msg);
    if (ret < 0) {
        LOG("qmp socket recv failed.");
        return ret;
    }
    LOG(msg);
    for (auto& cmd : qmp_cmds) {
        ret = qs.qmp_send(cmd);
        if (ret < 0) {
            LOG("qmp socket send failed.");
            return ret;
        }
        ret = qs.qmp_recv(msg);
        if (ret < 0) {
            LOG("qmp socket recv failed.");
            return ret;
        }
        LOG(msg);
        usleep(10000);
    }
    return 0;
}


colod_ret_val colod_start(std::string domain_name, bool colo_enable) {
    domain d;
    COLO_DOMAIN_STATUS cur_status = rs.domains[domain_name].colo_status;
    if (test_domain_status(domain_name) != DOMAIN_SHUT_OFF) {
        return {
            -1,
            "domain is already running.",
        };
    }

    if (get_domain(domain_name, d) < 0) {
        return {
            -1,
            "can not get domain data.",
        };
    }
    // todo: get colo status from colod 
    if (!colo_enable) {
        shell_command cmd;
        generate_vm_cmd(d, cmd);
        int pid = run_new_vm(d.name, cmd);
        rs.domains[domain_name].pid = pid;
        rs.domains[domain_name].status = DOMAIN_RUNNING;    
        return {
            0,
            "domain " + domain_name + " start.",
        };
    } 
    //start colo domain
    if (!rs.domains[domain_name].colo_enable) {
        return {
            0,
            "domain " + domain_name + " is not colo enable.",
        };
    }
    
    // start peer vm
    
    if (!remote_client_init) {
        set_remote_client(rs.current_status.peer_ip, rs.current_status.colod_port);
    }
    auto ret = remote_client.call<colod_ret_val>("peer-start-domain", domain_name, colo_enable);

    if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {  
        peer_init = false;
        return {
            -1,
            "colo domain " + domain_name + " secondary start failed.",
        };
    } else {
        colod_ret_val crv = ret.val();
        if (crv.code < 0) {
            return {
                -1,
                crv.msg,
            };  
        }
    }

    // start host vm

    shell_command cmd;
    if (cur_status == COLO_DOMAIN_PRIMARY) {
        generate_pvm_cmd(d, rs.current_status, cmd);
    } else if (cur_status == COLO_DOMAIN_SECONDARY) {
        generate_svm_cmd(d, rs.current_status, cmd);
    }
    
    int pid = run_new_vm(d.name, cmd);
    if (pid < 0) {
        return {
            -1,
            "colo domain " + domain_name + " start failed.",
        };
    }
    rs.domains[domain_name].pid = pid;
    rs.domains[domain_name].status = DOMAIN_COLO_ENABLED;

    // send qmp command
    
    if (cur_status == COLO_DOMAIN_PRIMARY) {
        // send peer snd qmp
        ret = remote_client.call<colod_ret_val>("peer-send-qmpcmds", domain_name, COLO_DOMAIN_SECONDARY);
        if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {  
            peer_init = false;
            return {
                -1,
                "colo domain " + domain_name + " start failed :  secondary send qmpcmd failed.",
            };
        } else {
            colod_ret_val crv = ret.val();
            if (crv.code < 0) {
                return {
                    -1,
                    crv.msg,
                };  
            }
        }


        std::vector<std::string> qmp_cmds;
        generate_pvm_qmpcmd(d, rs.current_status, rs.domains[domain_name], qmp_cmds);
        if (send_qmp_cmds(qmp_cmds) < 0) {
            return {
                -1,
                "colo domain " + domain_name + " start failed : primary send qmp cmds failed.",
            };
        }

    } else if (cur_status == COLO_DOMAIN_SECONDARY) {
        std::vector<std::string> qmp_cmds;
        generate_svm_qmpcmd(d, rs.current_status, rs.domains[domain_name], qmp_cmds);
        if (send_qmp_cmds(qmp_cmds) < 0) {
            return {
                -1,
                "colo domain " + domain_name + " start failed : secondary send snd qmp cmds failed.",
            };
        }

        // send peer pri qmp
        ret = remote_client.call<colod_ret_val>("peer-send-qmpcmds", domain_name, COLO_DOMAIN_PRIMARY);
        if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {  
            peer_init = false;
            return {
                -1,
                "colo domain " + domain_name + " start failed :  primary send qmpcmd failed.",
            };
        } else {
            colod_ret_val crv = ret.val();
            if (crv.code < 0) {
                return {
                    -1,
                    crv.msg,
                };  
            }
        }
    }


    return {
        0,
        "colo domain " + domain_name + " start.",
    };
}


colod_ret_val colod_destroy(std::string domain_name) {
    if (test_domain_status(domain_name) == DOMAIN_SHUT_OFF) {
        return {
            -1,
            "domain is shutoff.",
        };
    }
    if (rs.domains[domain_name].status == DOMAIN_RUNNING) {
        if (destroy_domain(domain_name) < 0) {
            return {
                -1,
                "can not destroy domain.",
            };
        }
        rs.domains[domain_name].pid = -1;
        rs.domains[domain_name].status = DOMAIN_SHUT_OFF;
        return {
            0,
            "domain " + domain_name + " destroy.",
        };
    } else if (rs.domains[domain_name].status == DOMAIN_COLO_ENABLED) {
        //destroy peer domain
        if (!remote_client_init) {
            set_remote_client(rs.current_status.peer_ip, rs.current_status.colod_port);
        }
        auto ret = remote_client.call<colod_ret_val>("peer-destroy-domain", domain_name);

        if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {  
            peer_init = false;
            return {
                -1,
                "colo domain " + domain_name + " peer destroy failed.",
            };
        } else {
            colod_ret_val crv = ret.val();
            if (crv.code < 0) {
                return {
                    -1,
                    crv.msg,
                };  
            }
        }

        //destroy host domain
        if (destroy_domain(domain_name) < 0) {
            return {
                -1,
                "can not destroy domain.",
            };
        }
        rs.domains[domain_name].pid = -1;
        rs.domains[domain_name].status = DOMAIN_SHUT_OFF;
        return {
            0,
            "colo domain " + domain_name + " destroy.",
        };

    }
    return {
        -1,
        "can not destroy domain " + domain_name + " : wrong status.",
    };
    
}


colod_ret_val colod_colo_enable(std::string domain_name) {
    std::string err;
    if (rs.current_status.local_status == COLO_NODE_NONE ||
        rs.current_status.local_status == COLO_NODE_ERROR) {
        return {
            -1,
            "please connect peer colod first.",
        };
    }
    //todo: send domain info to peer colod 
    rs.domains[domain_name].colo_enable = true;
    if (rs.current_status.local_status == COLO_NODE_PRIMARY) {
        rs.domains[domain_name].colo_status = COLO_DOMAIN_PRIMARY;
    } else if (rs.current_status.local_status == COLO_NODE_SECONDARY) {
        rs.domains[domain_name].colo_status = COLO_DOMAIN_SECONDARY;
    }
    if (!remote_client_init) {
        set_remote_client(rs.current_status.peer_ip, rs.current_status.colod_port);
    }
    auto ret = remote_client.call<colod_ret_val>("peer-save-domain", rs.domains[domain_name]);
    if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {
        err = "connect to peer colod timeout.";
        rs.domains[domain_name].colo_enable = false;
        rs.current_status.local_status == COLO_NODE_NONE;
        peer_init = false;
        return {
            -1,
            err,
        };
    }
    
    colod_ret_val crv = ret.val();
    if (crv.code < 0) {
        err = crv.msg;
        rs.domains[domain_name].colo_enable = false;
        rs.current_status.local_status == COLO_NODE_NONE;
        return {
            -1,
            err,
        };
    }

    
     
    if (set_domain_colo_enable(domain_name, rs.domains[domain_name].colo_status) < 0) {
        err = "can not set config file.";
        rs.domains[domain_name].colo_enable = false;
        rs.current_status.local_status == COLO_NODE_NONE;
        return {
            -1,
            err,
        };
    }
    
    return {
        0,
        "build colo domain " + domain_name + " success.",
    };
    
}

colod_ret_val colod_colo_disable(std::string domain_name) {
    if (colo_disable(domain_name) < 0) {
        return {
            -1,
            "can not disable colo domain.",
        };
    }
    rs.domains[domain_name].colo_enable = false;
    return {
        0,
        "colo disable success",
    };
}


colod_ret_val colod_vm_status(std::string domain_name) {
    std::string msg;
    domain d;
    if (get_domain(domain_name, d) < 0) {
        msg = "can not get domain data.";
        return {
            -1,
            msg,
        };
    }
    msg += "-------------------------------------------\n";
    msg += "       Name: " + d.name + "\n";
    msg += "    OS Type: " + d.os_type + "\n";
    msg += "     Status: " + domain_status_to_str_map[rs.domains[domain_name].status] + "\n";
    msg += "     CPU(s): " + std::to_string(d.vcpu) + "\n";
    msg += "     Memory: " + std::to_string(d.memory_size) + " MB" + "\n";
    msg += "  Disk Size: " + std::to_string(d.disk.disk_size) + " MB" + "\n";
    if (rs.domains[domain_name].colo_enable) {
        msg += "Colo Status: on\n";
    } else {
        msg += "Colo Status: off\n";
    }
    msg += "-------------------------------------------\n";
    return {
        0,
        msg,
    };
}

colod_ret_val colod_set_params() {
    std::cout << "colod_set_params" << std::endl;
    return {
        0,
        "",
    };
}


colod_ret_val colod_do_failover(std::string domain_name) {
    std::cout << "colod_do_failover" << std::endl;
    return {
        0,
        "",
    };
}





// peer colod insterface

colod_ret_val peer_colod_connect_test_reply() {
    return {
        0,
        colo_node_status_to_str_map[rs.current_status.local_status],
    };
}

colod_ret_val peer_colod_connect_test() {
    if (!peer_init) {
        return {
            0,
            "connect-peer first.",
        };
    }
    if (!remote_client_init) {
        set_remote_client(rs.current_status.peer_ip, rs.current_status.colod_port);
    }
    auto ret = remote_client.call<colod_ret_val>("connect-reply");
    if (ret.error_code() != buttonrpc::RPC_ERR_SUCCESS) {
        peer_init = false;
        return {
            -1,
            "connect to peer colod timeout.",
        };
    }
    
    colod_ret_val crv = ret.val();
    return {
        0,
        crv.msg,
    };
}

colod_ret_val colod_domain_test(std::string domain_name) {
    DOMAIN_STATUS ds = test_domain_status(domain_name);
    return {
        0,
        domain_status_to_str_map[ds],
    };
}

colod_ret_val peer_colod_save_status(colo_status cs) {
    LOG("peer_colod_save_status");
    
    if (cs.local_status == COLO_NODE_PRIMARY) {
        rs.current_status.local_status = COLO_NODE_SECONDARY;
    } else if (cs.local_status == COLO_NODE_SECONDARY) {
        rs.current_status.local_status = COLO_NODE_PRIMARY;
    } else {
        rs.current_status.local_status = COLO_NODE_NONE;
    }
    rs.current_status.host_ip = cs.peer_ip;
    rs.current_status.host_user = cs.peer_user;
    rs.current_status.host_file_path = cs.peer_file_path;
    rs.current_status.peer_ip = cs.host_ip;
    rs.current_status.peer_user = cs.host_user;
    rs.current_status.peer_file_path = cs.host_file_path;
    rs.current_status.colod_port = cs.colod_port;
    
    if (save_colo_status(rs.current_status) < 0) {
        return {
            -1,
            "cannot save colo status",
        };
    }
    peer_init = true;
    return {
        0,
        "peer colod save status success.",
    };
}

colod_ret_val peer_colod_save_domain(colod_domain_status ds) {
    
    colod_domain_status nds = ds;
    if (ds.colo_enable) {
        nds.colo_status = (ds.colo_status == COLO_DOMAIN_PRIMARY)
         ? COLO_DOMAIN_SECONDARY : COLO_DOMAIN_PRIMARY;
    }
    std::string err;
    if (save_domain_info(nds, err) < 0) {
        return {
            -1,
            err,
        };
    }
    if (set_domain_colo_enable(nds.name, nds.colo_status) < 0) {
        return {
            -1,
            "can not set colo peer enable",
        };
    }
    rs.domains[ds.name] = nds;  
    
    return {
        0,
        "peer colod save domain success.",
    };
}

colod_ret_val peer_colod_start_domain(std::string domain_name, bool colo_enable) {
    if (!colo_enable) {
        return {
            -1,
            "peer colod can only start colo vm.",
        };    
    }
    if (!rs.domains[domain_name].colo_enable) {
        return {
            -1,
            "please run colo enable first.",
        };
    }
    domain d;
    if (get_domain(domain_name, d) < 0) {
        return {
            -1,
            "can not get domain data.",
        };
    }
    shell_command cmd;
    if (rs.domains[domain_name].colo_status == COLO_DOMAIN_PRIMARY) {
        generate_pvm_cmd(d, rs.current_status, cmd);
    } else if (rs.domains[domain_name].colo_status == COLO_DOMAIN_SECONDARY) {
        generate_svm_cmd(d, rs.current_status, cmd);
    }
    
    int pid = run_new_vm(d.name, cmd);
    if (pid < 0) {
        return {
            -1,
            "colo domain " + domain_name + " start failed.",
        };
    }
    rs.domains[domain_name].pid = pid;
    rs.domains[domain_name].status = DOMAIN_COLO_ENABLED;


    return {
        0,
        "peer colod start domain success.",
    };
}


colod_ret_val peer_colod_destroy_domain(std::string domain_name) {
    if (!rs.domains[domain_name].colo_enable) {
        return {
            -1,
            "can not destroy non-colo domain.",
        };
    }
    if (test_domain_status(domain_name) == DOMAIN_SHUT_OFF) {
        return {
            -1,
            "remote domain is shutoff.",
        };
    }
    if (destroy_domain(domain_name) < 0) {
        return {
            -1,
            "can not destroy domain.",
        };
    }
    rs.domains[domain_name].pid = -1;
    rs.domains[domain_name].status = DOMAIN_SHUT_OFF;
    return {
        0,
        "colo domain " + domain_name + " destroy.",
    };
}

colod_ret_val peer_colod_send_qmpcmds(std::string domain_name, COLO_DOMAIN_STATUS cds) {
    if (test_domain_status(domain_name) == DOMAIN_SHUT_OFF) {
        return {
            -1,
            "remote domain is shutoff.",
        };
    }
    domain d;
    if (get_domain(domain_name, d) < 0) {
        return {
            -1,
            "can not get domain data.",
        };
    }
    
    std::vector<std::string> qmp_cmds;
    if (cds == COLO_DOMAIN_PRIMARY) {
        generate_pvm_qmpcmd(d, rs.current_status, rs.domains[domain_name], qmp_cmds);
        if (send_qmp_cmds(qmp_cmds) < 0) {
            return {
                -1,
                "colo domain " + domain_name + " start failed : primary send qmp cmds failed.",
            };
        }
        return {
            0,
            "send colo qmp success.",
        };
    } else if (cds == COLO_DOMAIN_SECONDARY) {
        generate_svm_qmpcmd(d, rs.current_status, rs.domains[domain_name], qmp_cmds);
        if (send_qmp_cmds(qmp_cmds) < 0) {
            return {
                -1,
                "colo domain " + domain_name + " start failed : sencondary send qmp cmds failed.",
            };
        }
        return {
            0,
            "send colo qmp success.",
        };
    }

    
    return {
        -1,
        "colo domain " + domain_name + " peer send qmp failed.",
    };
}