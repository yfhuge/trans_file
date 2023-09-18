#pragma once

#include "public.h"
#include "processpool.h"

class Server
{
public:
    static const int MAX_EPOLL_SIZE = 1024;
public:
    Server(std::string ip, int port, int num);
    void EpollListen();
    void EventLoop();
private:
    void EpollAdd(int fd);
    void SendFile(int fd);

    ProcessPool<int> m_q;
    int listenfd;
    int epollfd;
    int m_port;
    std::string m_ip;
    struct sockaddr_in address;
    struct epoll_event Events[MAX_EPOLL_SIZE];
};