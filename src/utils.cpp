#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cstdarg>
#include <mutex>
#include <unistd.h>
#include <ctime>
#include "utils.h"

std::mutex coutMutex;
/**
 * Creates socket on specified port and starts listening to this socket
 * @param port Listen on this port
 * @return int File descriptor for new socket
 */
int CreateSocket(int port)
{
  int sock;
  int reuse = 1;

  /* Server addess */
  sockaddr_in server_address = (sockaddr_in){  
     AF_INET,
     htons(port),
     (in_addr){INADDR_ANY}
  };

  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    std::lock_guard<std::mutex> lock(coutMutex);
    fprintf(stderr, "Cannot open socket");  
    exit(-1);
  }

  /* Address can be reused instantly after program exits */
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);

  /* Bind socket to server address */
  if(bind(sock,(sockaddr*) &server_address, sizeof(server_address)) < 0) {
    std::lock_guard<std::mutex> lock(coutMutex);
    fprintf(stderr, "Cannot bind socket to address, port: %d", port);
    close(sock);
    //exit(-1);
    return -1;
  }

  listen(sock,1024);
  //listen(sock, 0);
  return sock;
}

/**
 * Accept connection from client
 * @param socket Server listens this
 * @return int File descriptor to accepted connection
 */
int AcceptConnection(int sock)
{
  socklen_t addrlen = 0;
  sockaddr_in client_address;
  addrlen = sizeof(client_address);
  return accept(sock,(sockaddr*) &client_address,&addrlen);
}

/**
 * Get ip where client connected to
 * @param sock Commander socket connection
 * @param ip Result ip array (length must be 4 or greater)
 * result IP array e.g. {127,0,0,1}
 */
void GetIp(int sock, int *ip)
{
  socklen_t addr_size = sizeof(sockaddr_in);
  sockaddr_in addr;
  getsockname(sock, (sockaddr *)&addr, &addr_size);
  char* host = inet_ntoa(addr.sin_addr);
  sscanf(host,"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3]);
}

/**
 * thread safe printf wrapper
 */

int Print(const char * fmt, ... ) {
    int res = 0;
    va_list args;
    va_start(args, fmt);
    {
        std::lock_guard<std::mutex> lock(coutMutex);
        // print prefix
        time_t now = time(0);
        tm *ltm = localtime(&now);
        printf("%4d/%02d/%02d %02d:%02d:%02d ", ltm->tm_year, ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, \
        ltm->tm_min, ltm->tm_sec);
        res = vprintf(fmt, args);
    }
    va_end(args);
    return res;
}