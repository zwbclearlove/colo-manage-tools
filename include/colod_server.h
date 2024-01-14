#pragma once

#include "colod.h"
#include "buttonrpc.hpp"
#include "config.h"

class ColodServer {
public:
    ColodServer();
    ColodServer(int pid, int port);
    ~ColodServer();

    void server_init();
    void run();
private:
    int pid;
    int port;
    buttonrpc brpc;
    
};