#include "server.h"

Server::Server(std::string ip, int port, int num):m_q(num), m_ip(ip), m_port(port)
{
}

void Server::EpollAdd(int fd)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
}

void Server::SendFile(int fd)
{
    int new_fd = open("file", O_RDWR);
    assert(new_fd != -1);
    struct stat file_stat;
    int ret = stat("file", &file_stat);
    assert(ret != -1);
    int file_name_size = strlen("file");
    // 发送文件名的长度
    ret = send(fd, &file_name_size, 4, 0);
    assert(ret != -1);
    // 发送文件名
    ret = send(fd, "file", file_name_size, 0);
    assert(ret != -1);
    // 发送文件的大小
    ret = send(fd, &file_stat.st_size, sizeof(file_stat.st_size), 0);
    assert(ret != -1);
    char buf[1024] = {0};
    int len;
    while (1)
    {
        len = read(new_fd, buf, 1024);
        if (len == 0) break;
        assert(len != -1);

        ret = send(fd, &len, 4, 0);
        assert(ret != -1);
        send_n(fd, buf, len);
    }
}

void Server::EpollListen()
{
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(listenfd != -1);
    int reuse = 1;
    int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse));
    assert(ret != -1);
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(m_port);
    address.sin_addr.s_addr = inet_addr(m_ip.c_str());
    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    epollfd = epoll_create(5);
    assert(epollfd != -1);
    EpollAdd(listenfd);
    m_q.ListenAll(epollfd);
}

void Server::EventLoop()
{
    while (1)
    {
        int num = epoll_wait(epollfd, Events, MAX_EPOLL_SIZE, -1);
        for (int i = 0; i < num; ++ i)
        {
            if (Events[i].data.fd == listenfd)
            {
                // 有新连接到来
                struct sockaddr_in client_addr;
                socklen_t client_len;
                int clientfd = accept(listenfd, (struct sockaddr*)&client_addr, &client_len);
                assert(clientfd != -1);
                printf("now %s %d connect\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                m_q.SendTask(clientfd);
                close(clientfd);
            }
            for (int j = 0; j < m_q.process_num; ++ j)
            {
                if (Events[i].data.fd == m_q.m_processQue[j].fd)
                {
                    m_q.m_processQue[j].busy = false;
                    char flag;
                    int ret = recv(m_q.m_processQue[j].fd, &flag, 1, 0);
                    assert(ret != -1);
                    printf("child %d is not busy\n", m_q.m_processQue[j].pid);
                    break;
                }
            }
        }
    }
}