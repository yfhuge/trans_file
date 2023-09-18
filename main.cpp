#include "server.h"

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        printf("format:./server IP PORT PROCESSNUM\n");
        return 0;
    }
    Server server(argv[1], atoi(argv[2]), atoi(argv[3]));
    server.EpollListen();
    server.EventLoop();
    return 0;
}