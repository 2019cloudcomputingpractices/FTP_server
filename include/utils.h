#ifndef FTP_UTILS_H
#define FTP_UTILS_H

/**
 * Creates socket on specified port and starts listening to this socket
 * @param port Listen on this port
 * @return int File descriptor for new socket
 */
int CreateSocket(int port);

/**
 * Accept connection from client
 * @param socket Server listens this
 * @return int File descriptor to accepted connection
 */
int AcceptConnection(int sock);

/**
 * Get ip where client connected to
 * @param sock Commander socket connection
 * @param ip Result ip array (length must be 4 or greater)
 * result IP array e.g. {127,0,0,1}
 */
void GetIp(int sock, int *ip);

/**
 * thread safe printf wrapper
 */
int Print(const char * fmt, ... );

#endif