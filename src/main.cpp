#include "server.h"
#include "utils.h"
#include <cstring>
#include <cstdlib>
#include <csignal>
#define PORT 8021
#define USAGE \
"usage: ftp_server [-p <port>]\n\
p\tspecify the port on which the server listen\n\
"

void signalHandler( int signum )
{
    Print("Interrupt signal (%d) received.\n", signum);
 
    // 清理并关闭
    // 终止程序  
 
   exit(signum);  
 
}
 

int main(int argc, char* argv[]) {
    // 注册信号 SIGINT 和信号处理程序
    signal(SIGINT, signalHandler);
    int port = PORT;
    if (argc > 2) {
        if (strcmp(argv[1], "-p") == 0) {
            int arg_port = atoi(argv[2]);
            if (arg_port > 20 && arg_port < 65536) {
                port = arg_port;
            }
        } else {
            Print("%s", USAGE);
        }
    }
    Server server(port);
    server.Start();
}