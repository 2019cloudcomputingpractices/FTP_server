#ifndef SERVER_H
#define SERVER_H

#include <atomic>
#include <queue>
#include <vector>
#include "ftp.h"
#define PORT_RANGE_MIN 6000
#define PORT_RANGE_MAX 60000
#define WORK_MODE
#define WORKERS 4
#define MAX_CONN 10000

class Server {
  private:
    std::atomic<int> thread_counter;
    int port_;
    int port_range_min;
    int port_range_max;
    std::atomic<int> port_pointer;
    std::atomic<int> conns_owner;
    std::queue<int> conns_queue;
    std::vector<int> conns_pool;
    std::atomic<int> conns_pool_begin;
    std::atomic<int> conns_pool_end;
  public:
    Server(int port): port_(port),conns_owner(-1), conns_pool_begin(0), conns_pool_end(0),thread_counter(1), port_pointer(PORT_RANGE_MIN), port_range_min(PORT_RANGE_MIN), port_range_max(PORT_RANGE_MAX) {
      this->conns_pool.resize(MAX_CONN + 1, -1);
    }
    void Start();
    void NewConnection(int sock);
    void GetPort(Port* port);
    void PushConnection(int sock);
    int PopConnection();
    bool ConnPoolEmpty();
    bool ConnPoolFull();
    int GetConn();
    void PutConn(int sock);
};

void _worker(Server* srv);

#endif