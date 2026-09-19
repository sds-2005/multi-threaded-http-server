#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "ThreadPool.h"

using namespace std;

class TcpServer {
public:
    TcpServer(string ip_address, int port, size_t num_threads);
    ~TcpServer();
    void startListen();

private:
    string m_ip_address;
    int m_port;
    int m_socket;
    struct sockaddr_in m_socketAddress;
    unsigned int m_socketAddress_len;
    size_t m_num_threads;
    ThreadPool m_threadPool;

    int startServer();
    void closeServer();
    void handleClient(int client_socket);
};

#endif
