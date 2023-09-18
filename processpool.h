#include "public.h"

struct ProcessChild
{
public:
    int pid;
    int fd;
    bool busy;
};

template<typename T>
class ProcessPool
{
public:
    ProcessPool(int num, int max_request = 8000);
    ~ProcessPool();
    bool Append(T *request);
    void SendTask(int fd);
    void ListenAll(int epollfd);
public:
    void run(int fd);

    int process_num;    // 进程池的进程数量
    int request_num;    // 最大请求数量
    std::mutex m_mutex;
    std::condition_variable m_cond;
    ProcessChild* m_processQue;
    std::list<T*> m_que;
};

template<typename T>
ProcessPool<T>::ProcessPool(int num, int max_request) : process_num(num), request_num(max_request)
{
    int pid;
    int pipefd[2];
    m_processQue = new ProcessChild[num];
    assert(m_processQue != nullptr);
    for (int i = 0; i < process_num; ++ i)
    {
        int ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, pipefd);
        assert(ret != -1);
        pid = fork();
        if (pid == 0)
        {
            // 子进程
            close(pipefd[1]);
            run(pipefd[0]);
        }
        else 
        {
            // 父进程
            close(pipefd[0]);
            struct ProcessChild child;
            child.pid = pid;
            child.fd = pipefd[1];
            child.busy = false;
            m_processQue[i] = child;
        }
    }
}

template<typename T>
ProcessPool<T>::~ProcessPool()
{
    for (int i = 0; i < process_num; ++ i)
    {
        kill(m_processQue[i].pid, SIGTERM);
    }
    delete[] m_processQue;
}

template<typename T>
bool ProcessPool<T>::Append(T *request)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_que.size() >= request)
    {
        return false;
    }
    m_que.push_back(request);
    m_cond.notify_all();
    return true;
}

template<typename T>
void ProcessPool<T>::run(int sockfd)
{
    int fd, get_fd;
    while (1)
    {
        // 接收客户端的fd
        int ret = recv_fd(sockfd, &fd);
        assert(ret != -1);
        
        int new_fd = open("file", O_RDWR);
        assert(new_fd != -1);
        struct stat file_stat;
        ret = stat("file", &file_stat);
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

        char flag = 1;
        ret = send(sockfd, &flag, 1, 0);
        assert(ret != -1);
    }
}

template<typename T>
void ProcessPool<T>::SendTask(int fd)
{
    for (int i = 0; i < process_num; ++ i)
        if (!m_processQue[i].busy)
        {
            printf("child %d is busy\n", m_processQue[i].pid);
            m_processQue[i].busy = true;
            send_fd(m_processQue[i].fd, fd);
            break;
        }
}

template<typename T>
void ProcessPool<T>::ListenAll(int epollfd)
{
    for (int i = 0; i < process_num; ++ i)
    {
        struct epoll_event event;
        event.data.fd = m_processQue[i].fd;
        event.events = EPOLLIN;
        int ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, m_processQue[i].fd, &event);
        assert(ret != -1);
    }
}