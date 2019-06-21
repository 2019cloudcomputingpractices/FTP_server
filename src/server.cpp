#include "server.h"
#include "utils.h"
#include "connection.h"
#include <mutex>
#include <thread>

void Server::Start() {
    int sock = CreateSocket(port_);
    for (;;) {
        int newsock = AcceptConnection(sock);
        std::thread t(&Server::NewConnection, this, newsock);
        t.detach();
    }
}

void Server::NewConnection(int sock) {
    Print("number of active threads: %d\n",++thread_counter);

    Connection connection(sock);
    connection.Run();

    Print("number of active threads: %d\n",--thread_counter);
}