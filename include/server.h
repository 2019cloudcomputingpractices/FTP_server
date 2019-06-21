#ifndef SERVER_H
#define SERVER_H

#include <atomic>

class Server {
  private:
    std::atomic<int> thread_counter;
    int port_;

  public:
    Server(int port): port_(port), thread_counter(1) {}
    void Start();
    void NewConnection(int sock);
};

#endif