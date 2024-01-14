#pragma once

// colo_manage_tools interface
int colod_connect_peer();
int colod_connect_status();
int colod_define();
int colod_undefine();
int colod_list();
int colod_start();
int colod_destroy();
int colod_colo_enable();
int colod_colo_disable();
int colod_vm_status();
int colod_set_params();
int colod_do_failover();


// peer colod insterface
int colod_connect_test();
int colod_domain_test();
