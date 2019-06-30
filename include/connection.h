#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <map>
#include <cstring>
#include "server.h"

class Connection {
 public:
  Connection(int sock, Server* srv): sock_(sock), srv(srv), status_(kRunning), mode_(kNormal), sock_pasv_(-1) {
    memset(username_, 0, sizeof username_);
  }
  void Run();

 private:
  int sock_;
  int sock_pasv_;

  Server* srv;

  bool logged_in_;
  enum Mode {kNormal, kServer, kClient} mode_;
  enum Status {kRunning, kClosed} status_;
  char username_[32];

  void Response(const Command &cmd);
  void SendMessage(const char* message);

  void FtpAbor(const char* arg);
  void FtpCwd (const char* arg);
  void FtpDele(const char* arg);
  void FtpList(const char* arg);
  void FtpMdtm(const char* arg);
  void FtpMkd (const char* arg);
  void FtpNlst(const char* arg);
  void FtpNoop(const char* arg);
  void FtpPass(const char* arg);
  void FtpPasv(const char* arg);
  void FtpPort(const char* arg);
  void FtpPwd (const char* arg);
  void FtpQuit(const char* arg);
  void FtpRetr(const char* arg);
  void FtpRmd (const char* arg);
  void FtpRnfr(const char* arg);
  void FtpRnto(const char* arg);
  void FtpSite(const char* arg);
  void FtpSize(const char* arg);
  void FtpStor(const char* arg);
  void FtpType(const char* arg);
  void FtpUser(const char* arg);
};


/**
 * Generate random port for passive mode
 */
void Gen_Port(Port *port);

#endif