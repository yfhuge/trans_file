#include "../public.h"

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("format:./client IP PORT\n");
        return 0;
    }
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(socket_fd != -1);
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(atoi(argv[2]));
    address.sin_addr.s_addr = inet_addr(argv[1]);
    int ret = connect(socket_fd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    char file_name[128] = {0};
    int file_name_size;
    // 接收文件名的长度
    ret = recv(socket_fd, &file_name_size, 4, 0);   
    assert(ret != -1);
    // 接收文件名
    ret = recv(socket_fd, file_name, file_name_size, 0);
    assert(ret != -1);
    int fd = open(file_name, O_RDWR | O_CREAT, 0666);
    assert(fd != -1);

    // 1. read + write
    off_t file_size, down_size = 0, lase_size = 0;
    // 接收文件的长度
    int len = recv(socket_fd, &file_size, sizeof(file_size), 0);
    assert(len != -1);
    char buf[1024] = {0};
    while (down_size < file_size)
    {
        ret = recv(socket_fd, &len, 4, 0);
        assert(ret != -1);
        recv_n(socket_fd, buf, len);
        ret = write(fd, buf, len);
        assert(ret != -1);
        down_size += len;
        if (down_size - lase_size >= file_size * 0.0000001)
        {
            printf("%6.2f%%\r", (double)down_size  / file_size * 100);
            fflush(stdout);
            lase_size = down_size;
        }
    }
    printf("100.00%%\n");
    close(fd);
    close(socket_fd);
    return 0;
}