#include "server.h"
#include "utils.h"
#include <cstring>
#include <cstdlib>
#define PORT 8021
#define USAGE \
"usage: ftp_server [-p <port>]\n\
p\tspecify the port on which the server listen\n\
"

int main(int argc, char* argv[]) {
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