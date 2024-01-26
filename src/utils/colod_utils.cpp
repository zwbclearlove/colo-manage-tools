#include "colod.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

int get_pid_by_name(char * process_name)
{
    char line[50] = {'\0'};
    char cmd[50] = {'\0'};
    sprintf(cmd, "pidof %s", process_name);
    FILE * command = popen(cmd,"r");
    fgets(line, 50, command); 
    pid_t pid = strtoul(line, NULL, 10);
    pclose(command);
    return pid;
}

int colod_pretest() {
    char name[10] = "colod";
    pid_t pid = get_pid_by_name(name);
    if (pid == 0) {
        printf("please start colod.\n");
        return -1;
    }
    printf("colod is running with pid : %d\n", pid);
    return 0;
}

int colod_restart() {
    return 0;
}

int colod_test_exist() {
    char name[10] = "colod";
    pid_t pid = get_pid_by_name(name);
    if (pid != 0) {
        
        printf("colod is already running with pid : %d\n", pid);
        return -1;
    }
    printf("start colod.\n");
    return 0;
}

qmp_socket::qmp_socket() {
    this->sockaddr = "127.0.0.1";
    this->port = 4444;
}

qmp_socket::qmp_socket(std::string sockaddr, int port) {
    this->sockaddr = sockaddr;
    this->port = port;
    this->sockfd = -1;
}

qmp_socket::~qmp_socket() {
    // if (this->sockfd > 0) {
    //     close(this->sockfd);
    // }
}

int qmp_socket::qmp_connect() {
    if (this->sockfd > 0) {
        return -1;
    }
    int ret = socket(AF_INET, SOCK_STREAM, 0);
    if (ret < 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
        return -1;
    }
    this->sockfd = ret;
    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(this->port);
    servaddr.sin_addr.s_addr = inet_addr(this->sockaddr.c_str());
    ret = connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if (ret < 0) {
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        return -1;
    }
    return 0;
}

int qmp_socket::qmp_send(const std::string& msg) {
    int ret;
    ret = send(this->sockfd, msg.c_str(), msg.length(), 0);    
    if (ret < 0) {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return -1;
    }
    return ret;
}

int qmp_socket::qmp_recv(std::string& msg) {
    char recv_msg[512];
    memset(recv_msg, 0, sizeof(recv_msg));
    int ret;
    ret = recv(this->sockfd, recv_msg, 512, 0);
    if (ret < 0) {
        printf("recv msg error: %s(errno: %d)\n", strerror(errno), errno);
    }
    msg = std::string(recv_msg, ret);
    return ret;
}

void qmp_socket::qmp_close() {
    
    shutdown(this->sockfd, SHUT_RDWR);
    close(this->sockfd);
}
 