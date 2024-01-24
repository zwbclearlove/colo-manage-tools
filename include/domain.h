#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "config.h"
#include "colod.h"
#include "shell_command.h"




// enum OS_ARCH_TYPE {
//     OS_ARCH_X86_64,
//     OS_ARCH_X86,
//     OS_ARCH_ARM64,
// };

typedef struct _domain
{
    std::string name;
    std::string qemu_path;
    int memory_size;
    DOMAIN_STATUS domain_status;
    std::string emulator;
    int vcpu;
    std::string os_arch_type;
    std::string os_machine;
    std::string os_type;
    std::string cpu_type;
    struct _disk {
        std::string driver;
        int disk_size;
        std::string path;
        std::string hidden_path;
        std::string active_path;
    } disk;

    struct _net {
        std::string id;
        std::string br;
        std::string helper;
        std::string mac;
    } net;

    std::vector<int> port_list;
    struct _pri {
        unsigned short telnet_port;
        unsigned short mirror_port;
        unsigned short cmp0_port;
        unsigned short cmp1_port;
        unsigned short cmp_out_port;
    } pri;
    struct _sec {
        unsigned short telnet_port;
        unsigned short nbd_port;
        unsigned short migrate_port;
        unsigned short red0_port;
        unsigned short red1_port;
    } sec;

} domain;



int vm_define(const std::string& vm_def_filepath, domain& d, std::string& err);
int vm_undefine(const std::string& domain_name, std::string& err);
int generate_vm_cmd(const domain& d, shell_command& cmd);
int generate_pvm_cmd(const domain& d, const colo_status& cs, shell_command& cmd);
int generate_svm_cmd(const domain& d, const colo_status& cs, shell_command& cmd);
int generate_pvm_qmpcmd(const domain& d, const colo_status& cs, const colod_domain_status& cds, std::vector<std::string>& qmp_cmds);
int generate_svm_qmpcmd(const domain& d, const colo_status& cs, const colod_domain_status& cds, std::vector<std::string>& qmp_cmds);
int get_domain(const std::string& domain_name, domain& d);
int get_domain_pid(const std::string& domain_name, int& pid);
// int get_domain_status(const std::string& domain_name, std::string& msg);
int colo_enable(const std::string& domain_name);
int colo_disable(const std::string& domain_name);