#include "domain.h"
#include "yaml-cpp/yaml.h"
#include <iostream>
#include <fstream>
#include "config_parser.h"
#include "runvm.h"



int vm_config_parse(const YAML::Node& config, domain& new_domain, std::string& err) {
    new_domain.name = config["name"].as<std::string>();

    if (!config["qemu"].IsDefined()) {
        err = "cannot find qemu emulator.";
        return -1;
    }
    new_domain.qemu_path = config["qemu"].as<std::string>();
    
    if (!config["memory"].IsDefined() || !config["memory"].IsScalar()) {
        err = "cannot parse vm memory size.";
        return -1;
    }
    new_domain.memory_size = config["memory"].as<int>();

    if (!config["vcpu"].IsDefined() || !config["vcpu"].IsScalar()) {
        err = "cannot parse vcpu size.";
        return -1;
    }
    new_domain.vcpu = config["vcpu"].as<int>();

    if (!config["os"].IsDefined()) {
        err = "cannot find os definition.";
        return -1;
    }
    if (!config["os"]["arch"].IsDefined()) {
        err = "cannot find os architecture.";
        return -1;
    }
    new_domain.os_arch_type = config["os"]["arch"].as<std::string>();

    if (!config["os"]["machine"].IsDefined()) {
        err = "cannot find os machine.";
        return -1;
    }
    new_domain.os_machine = config["os"]["machine"].as<std::string>();

    if (!config["os"]["type"].IsDefined()) {
        err = "cannot find os type.";
        return -1;
    }
    new_domain.os_type = config["os"]["type"].as<std::string>();

    if (!config["cpu"].IsDefined()) {
        err = "cannot find cpu def.";
        return -1;
    }
    new_domain.cpu_type = config["cpu"].as<std::string>();
    
    if (!config["disk"].IsDefined()) {
        err = "cannot find disk definition.";
        return -1;
    }
    if (!config["disk"]["driver"].IsDefined()) {
        err = "cannot find disk driver.";
        return -1;
    }
    new_domain.disk.driver = config["disk"]["driver"].as<std::string>();

    if (!config["disk"]["size"].IsDefined() || !config["disk"]["size"].IsScalar()) {
        err = "cannot find disk size.";
        return -1;
    }
    new_domain.disk.disk_size = config["disk"]["size"].as<int>();

    if (!config["disk"]["path"].IsDefined()) {
        err = "cannot find disk path.";
        return -1;
    }
    new_domain.disk.path = config["disk"]["path"].as<std::string>();

    if (!config["disk"]["hidden_path"].IsDefined()) {
        err = "cannot find disk hidden_path.";
        return -1;
    }
    new_domain.disk.hidden_path = config["disk"]["hidden_path"].as<std::string>();

    if (!config["disk"]["active_path"].IsDefined()) {
        err = "cannot find disk active_path.";
        return -1;
    }
    new_domain.disk.active_path = config["disk"]["active_path"].as<std::string>();

    if (!config["net"].IsDefined()) {
        err = "cannot find net definition.";
        return -1;
    }
    if (!config["net"]["id"].IsDefined()) {
        err = "cannot find net id.";
        return -1;
    }
    new_domain.net.id = config["net"]["id"].as<std::string>();

    if (!config["net"]["br"].IsDefined()) {
        err = "cannot find net br.";
        return -1;
    }
    new_domain.net.br = config["net"]["br"].as<std::string>();

    if (!config["net"]["helper"].IsDefined()) {
        err = "cannot find net helper.";
        return -1;
    }
    new_domain.net.helper = config["net"]["helper"].as<std::string>();

    if (!config["net"]["mac"].IsDefined()) {
        err = "cannot find net mac.";
        return -1;
    }
    new_domain.net.mac = config["net"]["mac"].as<std::string>();

    if (!config["colo"].IsDefined()) {
        err = "cannot find colo definition.";
        return -1;
    }
    // if (!config["colo"]["ports"].IsDefined() || !config["colo"]["ports"].IsSequence()) {
    //     err = "cannot find colo definition.";
    //     return -1;
    // }
    // for (int i = 0; i < config["colo"]["ports"].size(); i++) {
    //     new_domain.port_list.emplace_back(config["colo"]["ports"][i].as<int>());
    // }
    return 0;
}

#define ADD_ARG(arg) shell_command_add_arg(cmd, (arg))
#define ADD_ARGS(arg1, arg2) shell_command_add_args(cmd, (arg1), (arg2))

int generate_vm_cmd(const domain& d, shell_command& cmd) {
    cmd.binaryPath = d.qemu_path;
    ADD_ARG(cmd.binaryPath);
    ADD_ARG("-enable-kvm");
    ADD_ARG("-machine");
    ADD_ARG(d.os_machine + ",accel=kvm,usb=off,dump-guest-core=off,kernel_irqchip=on");
    ADD_ARGS("-m", std::to_string(d.memory_size));
    ADD_ARGS("-smp", std::to_string(d.vcpu));
    ADD_ARGS("-overcommit", "mem-lock=off");
    ADD_ARGS("-chardev", "socket,id=qmp,port=4444,host=localhost,server=on,wait=off");   
    ADD_ARGS("-mon", "chardev=qmp,mode=control,pretty=on");
    ADD_ARGS("-vnc", ":7");
    ADD_ARGS("-rtc", "base=utc");
    ADD_ARGS("-smbios", "type=1");
    ADD_ARG("-no-user-config");
    ADD_ARG("-nodefaults");
    ADD_ARGS("-boot", "menu=on,strict=on");
    ADD_ARGS("-cpu", d.cpu_type);
    ADD_ARGS("-device", "cirrus-vga,id=video0,bus=pci.0,addr=0x2");
    ADD_ARGS("-device", "piix3-usb-uhci,id=usb,bus=pci.0,addr=0x1.0x2");
    ADD_ARGS("-device", "usb-tablet,id=input0,bus=usb.0,port=1");
    ADD_ARGS("-netdev", "tap,id=" + d.net.id + ",vhost=off,br=" 
            + d.net.br + ",helper=" + d.net.helper);
    ADD_ARGS("-device", "virtio-net-pci,id=e0,netdev=" + d.net.id 
            + ",csum=false,mrg_rxbuf=false,gso=false,mac=" + d.net.mac);
    ADD_ARGS("-drive", "if=virtio,id=colo-disk0,driver=quorum,read-pattern=fifo,vote-threshold=1,children.0.file.filename=" 
            + d.disk.path + ",children.0.driver=" + d.disk.driver);    
    return 0;
}

int generate_pvm_cmd(const domain& d, const colo_status& cs, shell_command& cmd) {
    std::string host_ip = cs.local_status == COLO_NODE_PRIMARY ? cs.host_ip : cs.peer_ip;
    std::string sec_ip = cs.local_status == COLO_NODE_PRIMARY ? cs.peer_ip : cs.host_ip;
    cmd.binaryPath = d.qemu_path;
    ADD_ARG(cmd.binaryPath);
    ADD_ARG("-enable-kvm");
    ADD_ARG("-machine");
    ADD_ARG(d.os_machine + ",accel=kvm,usb=off,dump-guest-core=off,kernel_irqchip=on");
    ADD_ARGS("-m", std::to_string(d.memory_size));
    ADD_ARGS("-smp", std::to_string(d.vcpu));
    ADD_ARGS("-overcommit", "mem-lock=off");
    ADD_ARGS("-chardev", "socket,id=qmp,port=4444,host=localhost,server");   
    ADD_ARGS("-mon", "chardev=qmp,mode=control,pretty=on");
    ADD_ARGS("-vnc", ":7");
    ADD_ARGS("-rtc", "base=utc");
    ADD_ARGS("-smbios", "type=1");
    ADD_ARG("-no-user-config");
    ADD_ARG("-nodefaults");
    ADD_ARGS("-boot", "menu=on,strict=on");
    ADD_ARGS("-cpu", d.cpu_type);
    ADD_ARGS("-device", "cirrus-vga,id=video0,bus=pci.0,addr=0x2");
    ADD_ARGS("-device", "piix3-usb-uhci,id=usb,bus=pci.0,addr=0x1.0x2");
    ADD_ARGS("-device", "usb-tablet,id=input0,bus=usb.0,port=1");
    ADD_ARGS("-netdev", "tap,id=" + d.net.id + ",vhost=off,br=" 
            + d.net.br + ",helper=" + d.net.helper);
    ADD_ARGS("-device", "virtio-net-pci,id=e0,netdev=" + d.net.id 
            + ",csum=false,mrg_rxbuf=false,gso=false,mac=" + d.net.mac);
    ADD_ARGS("-chardev", "socket,id=mirror0,host=" + host_ip + ",port=9003,server=on,wait=off");
    ADD_ARGS("-chardev", "socket,id=compare1,host=" + host_ip + ",port=9004,server=on,wait=on");
    ADD_ARGS("-chardev", "socket,id=compare0,host=" + host_ip + ",port=9001,server=on,wait=off");
    ADD_ARGS("-chardev", "socket,id=compare0-0,host=" + host_ip + ",port=9001,reconnect=1");
    ADD_ARGS("-chardev", "socket,id=compare_out,host=" + host_ip + ",port=9005,server=on,wait=off");
    ADD_ARGS("-chardev", "socket,id=compare_out0,host=" + host_ip + ",port=9005,reconnect=1");
    ADD_ARGS("-object", "filter-mirror,id=m0,netdev=" + d.net.id + ",queue=tx,outdev=mirror0");
    ADD_ARGS("-object", "filter-redirector,netdev=" + d.net.id + ",id=redire0,queue=rx,indev=compare_out");
    ADD_ARGS("-object", "filter-redirector,netdev=" + d.net.id + ",id=redire1,queue=rx,outdev=compare0");
    ADD_ARGS("-object", "iothread,id=iothread1");
    ADD_ARGS("-object", "colo-compare,id=comp0,primary_in=compare0-0,secondary_in=compare1,outdev=compare_out0,iothread=iothread1");
    ADD_ARGS("-drive", "if=virtio,id=colo-disk0,driver=quorum,read-pattern=fifo,vote-threshold=1,children.0.file.filename=" 
            + d.disk.path + ",children.0.driver=" + d.disk.driver);   
    ADD_ARG("-S");
    return 0;
}

int generate_svm_cmd(const domain& d, const colo_status& cs, shell_command& cmd) {
    std::string host_ip = cs.local_status == COLO_NODE_SECONDARY ? cs.host_ip : cs.peer_ip;
    std::string pri_ip = cs.local_status == COLO_NODE_PRIMARY ? cs.host_ip : cs.peer_ip;
    cmd.binaryPath = d.qemu_path;
    ADD_ARG(cmd.binaryPath);
    ADD_ARG("-enable-kvm");
    ADD_ARG("-machine");
    ADD_ARG(d.os_machine + ",accel=kvm,usb=off,dump-guest-core=off,kernel_irqchip=on");
    ADD_ARGS("-m", std::to_string(d.memory_size));
    ADD_ARGS("-smp", std::to_string(d.vcpu));
    ADD_ARGS("-overcommit", "mem-lock=off");
    ADD_ARGS("-chardev", "socket,id=qmp,port=4444,host=localhost,server");   
    ADD_ARGS("-mon", "chardev=qmp,mode=control,pretty=on");
    ADD_ARGS("-vnc", ":7");
    ADD_ARGS("-rtc", "base=utc");
    ADD_ARGS("-smbios", "type=1");
    ADD_ARG("-no-user-config");
    ADD_ARG("-nodefaults");
    ADD_ARGS("-boot", "menu=on,strict=on");
    ADD_ARGS("-cpu", d.cpu_type);
    ADD_ARGS("-device", "cirrus-vga,id=video0,bus=pci.0,addr=0x2");
    ADD_ARGS("-device", "piix3-usb-uhci,id=usb,bus=pci.0,addr=0x1.0x2");
    ADD_ARGS("-device", "usb-tablet,id=input0,bus=usb.0,port=1");
    ADD_ARGS("-netdev", "tap,id=" + d.net.id + ",vhost=off,br=" 
            + d.net.br + ",helper=" + d.net.helper);
    ADD_ARGS("-device", "virtio-net-pci,id=e0,netdev=" + d.net.id 
            + ",csum=false,mrg_rxbuf=false,gso=false,mac=" + d.net.mac);
    ADD_ARGS("-chardev", "socket,id=red0,host=" + pri_ip + ",port=9003,reconnect=1");
    ADD_ARGS("-chardev", "socket,id=red1,host=" + pri_ip + ",port=9004,reconnect=1");
    ADD_ARGS("-object", "filter-redirector,id=f1,netdev=" + d.net.id + ",queue=tx,indev=red0,vnet_hdr_support");
    ADD_ARGS("-object", "filter-redirector,id=f2,netdev=" + d.net.id + ",queue=rx,outdev=red1,vnet_hdr_support");
    ADD_ARGS("-object", "filter-rewriter,id=rew0,netdev=" + d.net.id + ",queue=all,vnet_hdr_support");        

    ADD_ARGS("-drive", "if=none,id=parent0,file.filename=" + d.disk.path);
    ADD_ARGS("-drive", "if=none,id=childs0,driver=replication,mode=secondary,file.driver=" + d.disk.driver 
            + ",top-id=colo-disk0,file.file.filename=" + d.disk.active_path + ",file.backing.driver=" 
            + d.disk.driver + ",file.backing.file.filename=" + d.disk.hidden_path + ",file.backing.backing=parent0");   
    ADD_ARGS("-drive", "if=virtio,id=colo-disk0,driver=quorum,read-pattern=fifo,vote-threshold=1,children.0=childs0");
    ADD_ARGS("-incoming", "tcp:0:8888");        
    return 0;
}

int save_new_vm_file(const YAML::Node& vmdef, domain& d, std::string& err) {
    YAML::Node dom;
    std::string save_path = DEFAULT_SAVE_PATH + vmdef["name"].as<std::string>() + ".yaml";
    std::ofstream fout2(save_path);
    fout2 << vmdef;
    fout2.close();
    return 0;
}

int vm_define(const std::string& vm_def_filepath, domain& d, std::string& err) {
    YAML::Node defconfig = YAML::LoadFile(vm_def_filepath);
    if (!defconfig["name"]) {
        err = "cannot parse vm name.";
        return -1;
    }
    std::string name = defconfig["name"].as<std::string>();
    if (name.compare("save") == 0) {
        err = "cannot define vm with name 'save'.";
        return -1;
    }
    
    if (vm_config_parse(defconfig, d, err) < 0) {
        
        return -1;
    }
    
    if (save_new_vm_file(defconfig, d, err) < 0) {
        return -1;
    }
    
    return 0;
}

int vm_undefine(const std::string& domain_name, std::string& err) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    YAML::Node domains;
    bool rm = false;
    bool colo_enable = false;
    for (int i = 0; i < sf["domains"].size(); i++) {
        if (domain_name.compare(sf["domains"][i]["name"].as<std::string>()) == 0) {
            rm = true;
            if (sf["domains"][i]["colo_enable"].as<bool>()) {
                colo_enable = true;
            }
        } else {
            domains.push_back(sf["domains"][i]);
        }
    }
    if (colo_enable) {
        // std::cout << "remove colo file, domain name : " << domain_name << "." << std::endl;
    }
    if (rm) {
        sf["domains"] = domains;
        std::ofstream fout(DEFAULT_SAVE_FILE);
        fout << sf;
        fout.close();
        //std::cout << domain_name << " has been undefined." << std::endl;
    } else {
        err = "cannot find domain: " + domain_name + ".";
    }

    
    // todo: rm domain.yaml in save path 
    return 0;
}

int get_domain(const std::string& domain_name, domain& d) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    bool find = false;
    for (int i = 0; i < sf["domains"].size(); i++) {
        if (domain_name.compare(sf["domains"][i]["name"].as<std::string>()) == 0) {
            find = true;
            YAML::Node vmdef = YAML::LoadFile(sf["domains"][i]["path"].as<std::string>());
            std::string err;
            if (vm_config_parse(vmdef, d, err) < 0) {
                std::cerr << "cannot parse vm definition file." << std::endl;
                return -1;
            }
        }
    }
    if (!find) {
        return -1;
    }
    return 0;
}

int get_domain_pid(const std::string& domain_name, int& pid) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    for (int i = 0; i < sf["domains"].size(); i++) {
        if (domain_name.compare(sf["domains"][i]["name"].as<std::string>()) == 0) {
            int p = sf["domains"][i]["pid"].as<int>();
            if (p == -1) {
                std::cerr << "domain is not running." << std::endl;
                return -1;
            } else {
                pid = p;
                return 0;
            }
            
        }
    }
    return -1;
}

int get_domain_config_file_path(const std::string& domain_name, std::string& path) {
    YAML::Node sf = YAML::LoadFile(DEFAULT_SAVE_FILE);
    for (int i = 0; i < sf["domains"].size(); i++) {
        if (domain_name.compare(sf["domains"][i]["name"].as<std::string>()) == 0) {
            path = sf["domains"][i]["path"].as<std::string>();
            return 0;
        }
    }
    return -1;
}

// int get_domain_status(const std::string& domain_name, string& msg) {
//     domain d;
//     if (get_domain(domain_name, d) < 0) {
//         msg = "can not get domain data.";
//         return -1;
//     }
//     msg += "-------------------------------------------\n";
//     msg += "       Name: " + d.name + "\n";
//     msg += "    OS Type: " + d.os_type + "\n";
//     msg += "     Status: " + domain_status_to_str_map[rs.domains[domain_name].status] + "\n";
//     msg += "     CPU(s): " + d.vcpu + "\n";
//     msg += "     Memory: " + d.memory_size + " MB" + "\n";
//     msg += "  Disk Size: " + d.disk.disk_size + " MB" + "\n";
//     msg += "Colo Status: " + "none" + "\n";
//     msg += "-------------------------------------------\n";
//     return 0;
// }


int colo_enable(const std::string& domain_name) {
    domain d;
    if (get_domain(domain_name, d) < 0) {
        std::cout << "can not get domain data." << std::endl;
        return -1;
    }
    // transfer config file
    std::string src_config_file;
    std::string dst_config_file;
    if (get_domain_config_file_path(domain_name, src_config_file) < 0) {
        std::cout << "can not find domain config file." << std::endl;
        return -1;
    }
    colo_status cs;
    if (get_connect_status(cs) < 0) {
        std::cout << "can not get connect status." << std::endl;
        return -1;
    }
    dst_config_file = cs.peer_user + "@" + cs.peer_ip + ":" + cs.peer_file_path;

    std::cout << "scp " << src_config_file << " " << dst_config_file << std::endl;
    if (transfer_file(src_config_file, dst_config_file) < 0) {
        std::cout << "transfer domain config file failed." << std::endl;
    }
    std::cout << "transfer domain config file success." << std::endl;
    // transfer disk image 
    // std::string src_disk_file = d.disk.path;
    // std::string dst_disk_file = cs.peer_user + "@" + cs.peer_ip + ":" + src_disk_file;
    // std::cout << "scp " << src_disk_file << " " << dst_disk_file << std::endl;
    // if (transfer_file(src_disk_file, dst_disk_file) < 0) {
    //     std::cout << "transfer domain disk file failed." << std::endl;
    // }
    // std::cout << "transfer domain disk file success." << std::endl;
    
    std::cout << "build colo env success." << std::endl;
    return 0;
}

int colo_disable(const std::string& domain_name) {
    domain d;
    if (set_domain_colo_disable(domain_name) < 0) {
        std::cout << "can not set config file." << std::endl;
        return -1;
    }
    return 0;
}