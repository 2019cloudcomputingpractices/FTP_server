#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <map>
#include <cstring>

static const int kCommandArgSize = 1024;
static const int kBufferSize = kCommandArgSize + 10;


/* Commands enumeration */
typedef enum {
  ABOR, CWD, DELE, LIST, MDTM, MKD, NLST, PASS, PASV,
  PORT, PWD, QUIT, RETR, RMD, RNFR, RNTO, SITE, SIZE,
  STOR, TYPE, USER, NOOP, Unknown
} CommandType;

CommandType LookUpCommand(const char* cmd);

typedef struct Port {
  int p1;
  int p2;
} Port;

struct Command {
  char command[5];
  char arg[kCommandArgSize];
  explicit Command(const char* buf) {
    sscanf(buf, "%s %s", command, arg);
  }
};

class Connection {
 public:
  Connection(int sock): sock_(sock), status_(kRunning), mode_(kNormal), sock_pasv_(-1) {
    memset(username_, 0, sizeof username_);
  }
  void Run();

 private:
  int sock_;
  int sock_pasv_;

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