#ifndef FTP_H
#define FTP_H
#include <cstdio>
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

#endif