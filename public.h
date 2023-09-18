#pragma once

#define _GUN_SOURCE
#include <unistd.h>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <mutex>
#include <condition_variable>
#include <list>
#include <memory.h>
#include <signal.h>
#include <sys/epoll.h>
#include <assert.h>
#include <sys/stat.h>
#include <mutex>
#include <condition_variable>

int send_n(int fd, void* buf, int len);
int recv_n(int fd, void* buf, int len);

int send_fd(int sfd,int fd);
int recv_fd(int sfd,int *fd);