#include "public.h"

int send_n(int fd, void* buf, int len)
{
    char *p = (char*)buf;
    int total = 0;
    while (total < len)
    {
        int ret = send(fd, p + total, len - total, 0);
        assert(ret != -1);
        total += ret;
    }
    return 0;
}

int recv_n(int fd, void* buf, int len)
{
    char *p = (char*)buf;
    int total = 0;
    while (total < len)
    {
        int ret = recv(fd, p + total, len - total, 0);
        assert(ret != -1);
        total += ret;
    }
    return 0;
}

//做一种仪式，让内核控制信息发生进程间的dup机制
int send_fd(int sfd,int fd)
{
    struct msghdr msg;
    bzero(&msg,sizeof(msg));//把结构体清空
    struct iovec iov[1];//用户态要传递一些内容
    char buf[128]="hello";
    iov[0].iov_base=buf;
    iov[0].iov_len=5;
    msg.msg_iov=iov;
    msg.msg_iovlen=1;
    //构造变长结构体
    int cmsg_len=CMSG_LEN(sizeof(int));
    struct cmsghdr *cmsg=(struct cmsghdr *)malloc(cmsg_len);
    cmsg->cmsg_len=cmsg_len;
    cmsg->cmsg_level=SOL_SOCKET;
    cmsg->cmsg_type=SCM_RIGHTS;
    *(int*)CMSG_DATA(cmsg)=fd;//给变长结构体的最后一个成员赋值
    msg.msg_control=cmsg;
    msg.msg_controllen=cmsg_len;
    int ret;
    ret=sendmsg(sfd,&msg,0);
    assert(ret != -1);
    return 0;
}
//接收描述符
int recv_fd(int sfd,int *fd)
{
    struct msghdr msg;
    bzero(&msg,sizeof(msg));//把结构体清空
    struct iovec iov[1];//用户态要传递一些内容
    char buf[128]={0};
    iov[0].iov_base=buf;
    iov[0].iov_len=5;
    msg.msg_iov=iov;
    msg.msg_iovlen=1;
    //构造变长结构体
    int cmsg_len=CMSG_LEN(sizeof(int));
    struct cmsghdr *cmsg=(struct cmsghdr *)malloc(cmsg_len);
    cmsg->cmsg_len=cmsg_len;
    cmsg->cmsg_level=SOL_SOCKET;
    cmsg->cmsg_type=SCM_RIGHTS;
    msg.msg_control=cmsg;
    msg.msg_controllen=cmsg_len;
    int ret;
    ret=recvmsg(sfd,&msg,0);
    assert(ret != -1);
    *fd=*(int*)CMSG_DATA(cmsg);
    return 0;
}
