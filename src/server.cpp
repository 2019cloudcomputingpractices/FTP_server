#include "server.h"
#include "utils.h"
#include "connection.h"
#include <mutex>
#include <thread>
#include <unistd.h>

void _work_multithread(Server* srv, int sock) {
    for (;;) {
        int newsock = AcceptConnection(sock);
        std::thread t(&Server::NewConnection, srv, newsock);
        t.detach();
    }
}

void _work_multiprocess(Server* srv, int sock) {
    pid_t fpid;
    for (;;) {
        int newsock = AcceptConnection(sock);
        fpid = fork();
        if (fpid < 0) {
            Print("error in fork");
        } else if (fpid == 0) {
            // 子进程
			srv->NewConnection(newsock);
        } else {
			// 父进程
			continue;
		}
    }
}

void _worker(Server* srv) {
    Print("Worker ready...\n");
    //int sock = srv->PopConnection();
    int sock = srv->GetConn();
    for (;;) {
        while(sock < 0) {
            //sock = srv->PopConnection();
            sock = srv->GetConn();
        }
        std::thread t(&Server::NewConnection, srv, sock);
        t.detach();
        sock = -1;
    }
}

void Server::Start() {
    std::thread t(&_worker, this);
    t.detach();
    int sock = CreateSocket(port_);
    Print("Server listen on %d...\n", port_);
    //_work_multithread(this, sock);
	//_work_multiprocess(this, sock);
    for (;;) {
        int newsock = AcceptConnection(sock);
        //this->PushConnection(newsock);
        this->PutConn(newsock);
    }
}

void Server::NewConnection(int sock) {
    Print("New connection established with sock %d!\n", sock);
    Print("number of active threads: %d\n",++thread_counter);

    Connection connection(sock, this);
    connection.Run();
    Print("Connection closed with sock %d\n!", sock);
    Print("number of active threads: %d\n",--thread_counter);
}

void Server::GetPort(Port* port) {
	int range = port_range_max - port_range_min;
	int port_value = ((port_pointer++ - port_range_min) % range) + port_range_min;
	port->p2 = port_value % 256;
  	port->p1 = port_value / 256;
	Print("GetPort: port-%d\n", port_value);
}

void Server::PushConnection(int sock) {
    for (;;) {
        if (this->conns_owner != -1) {
            continue;
        }
        this->conns_owner = 0;
        this->conns_queue.push(sock);
        this->conns_owner = -1;
        break;
    }
}

int Server::PopConnection() {
    int sock = -1;
    int max_try = 10;
    for (int i = 0;i < max_try;i++) {
        if (this->conns_owner != -1) {
            continue;
        }
        this->conns_owner = 0;
        if (this->conns_queue.empty()) {
            break;
        }
        sock = this->conns_queue.front();
        this->conns_queue.pop();
        this->conns_owner = -1;
        break;
    }
    return sock;
}

bool Server::ConnPoolEmpty() {
    return conns_pool_begin == conns_pool_end;
}

bool Server::ConnPoolFull() {
    return (conns_pool_end + 1) % (MAX_CONN + 1) == conns_pool_begin;
}

int Server::GetConn() {
    if (this->ConnPoolEmpty()) {
        //Print("[WorkerGetConnSock(-1)]\n");
        return -1;
    }
    int sock = this->conns_pool[conns_pool_begin];
    conns_pool_begin = (1+ conns_pool_begin) % (MAX_CONN + 1);
    Print("[WorkerGetConnSock(%d)]\n", sock);
    return sock;
}

void Server::PutConn(int sock) {
    for (;;) {
        if (this->ConnPoolFull()) {
            continue;
        }
        int new_pos = (conns_pool_end + 1) % (MAX_CONN + 1);
        conns_pool[conns_pool_end] = sock;
        conns_pool_end = new_pos;
        Print("[ServerPutConnSock(%d)]\n", sock);
        break;
    }
}