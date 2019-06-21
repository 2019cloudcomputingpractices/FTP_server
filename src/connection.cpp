#include "connection.h"
#include "utils.h"
#include <unistd.h> 
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
std::string PermissionString(int permission);
ssize_t SendFile(int out_fd, int in_fd, off_t * offset, size_t count);

/**
 * Generate random port for passive mode
 */
void GenPort(Port *port)
{
  srand(time(NULL));
  port->p1 = 128 + (rand() % 64);
  port->p2 = rand() % 0xff;
}

void Connection::Run() {
	SendMessage("220 Welcome to ftp service built by dafeng & shukui. \n");
	char buffer[kBufferSize] = {0};

	while (status_ != kClosed) {
    int bytes_read = read(sock_, buffer, kBufferSize);
    if (bytes_read == 0) {
      break;
    }
		if(bytes_read > kBufferSize) {
			Print("Server read error :(");
			break;
		}
    buffer[kBufferSize - 1] = '\0';
    Print("User %s sent command: %s\n",(username_ == 0)?"unknown":username_, buffer);
    Command cmd(buffer);         
    /* Ignore non-ascii char. Ignores telnet command */
    if(buffer[0]<=127 && buffer[0]>=0) {
      Response(cmd);
    }
    memset(buffer, 0, sizeof buffer);	
	}

	close(sock_);
	Print("Closing... :)\n");
}

void Connection::Response(const Command& cmd) {
	const char* arg = cmd.arg;
  switch(LookUpCommand(cmd.command)) {
    case ABOR: FtpAbor(arg); break;
		case CWD:  FtpCwd (arg); break;
		case DELE: FtpDele(arg); break;
		case LIST: FtpList(arg); break;
		case MDTM: FtpMdtm(arg); break;
		case MKD:  FtpMkd (arg); break;
		case NLST: FtpNlst(arg); break;
    case NOOP: FtpNoop(arg); break;
		case PASS: FtpPass(arg); break;
    case PASV: FtpPasv(arg); break;
		case PORT: FtpPort(arg); break;
		case PWD:  FtpPwd (arg); break;
		case QUIT: FtpQuit(arg); break;
		case RETR: FtpRetr(arg); break;
		case RMD:  FtpRmd (arg); break;
		case RNFR: FtpRnfr(arg); break;
		case RNTO: FtpRnto(arg); break;	
		case SITE: FtpSite(arg); break;
 		case SIZE: FtpSize(arg); break;
    case STOR: FtpStor(arg); break;
  	case TYPE: FtpType(arg); break;
  	case USER: FtpUser(arg); break;		
		default: SendMessage("500 Unknown command\n");
	}
}

void Connection::SendMessage(const char* message) {
  write(sock_, message, strlen(message));
}

/** 
 * ABOR command 
 * Incomplete
 */
void Connection::FtpAbor(const char* arg) {
  if (!logged_in_) {
    SendMessage("530 Please login with USER and PASS.\n");
    return;
  }
  SendMessage("226 Closing data connection.\n");
}

/** CWD command */
void Connection::FtpCwd(const char* arg) {
  if(logged_in_) {
    if(chdir(arg) == 0){
      SendMessage("250 Directory successfully changed.\n");
    } else {
      SendMessage("550 Failed to change directory.\n");
    }
  } else {
		SendMessage("530 Please login with USER and PASS.\n");
	}
}

/** DELE command */
void Connection::FtpDele(const char* arg) {
  if (!logged_in_) {
    SendMessage("530 Please login with USER and PASS.\n");
    return;
  }
  if(unlink(arg) == -1) {
    SendMessage("550 File unavailable.\n");
  } else {
    SendMessage("250 Requested file action okay, completed.\n");
  }
}

/** LIST command */
void Connection::FtpList(const char* arg) {
  if(logged_in_){
		char current_dir[kBufferSize] = {0};
		char original_dir[kBufferSize] = {0}; 
    /* Later we want to go to the original path */
    getcwd(original_dir, kBufferSize);
		
    /* Just chdir to specified path */
    if(strlen(arg) > 0 && arg[0]!='-'){
      chdir(arg);
    }
    getcwd(current_dir, kBufferSize);
    DIR *dir = opendir(current_dir);
    if(!dir){
      SendMessage("550 Failed to open directory.\n");
    }else{
      if(mode_ == kServer){
        int data_connection = AcceptConnection(sock_pasv_);
        SendMessage("150 Here comes the directory listing.\n");
				dirent *entry;
        while(entry = readdir(dir)) {
					struct stat statbuf;
          if(stat(entry->d_name, &statbuf) == -1) {
            Print("FTP: Error reading file stats...\n");
          } else{
            /* Convert time_t to tm struct */
            time_t rawtime = statbuf.st_mtime;
            struct tm *time = localtime(&rawtime);
						char timebuff[80] = {0};
            strftime(timebuff, 80, "%b %d %H:%M", time);
						std::string permission = PermissionString(statbuf.st_mode & ALLPERMS);
            dprintf(data_connection,
                "%c%s %5lu %4d %4d %8ld %s %s\r\n", 
                (entry->d_type==DT_DIR)?'d':'-',
                permission.c_str(),
								statbuf.st_nlink,
                statbuf.st_uid, 
                statbuf.st_gid,
                statbuf.st_size,
                timebuff,
                entry->d_name);
          }
        }        
        SendMessage("226 Directory send OK.\n");
        mode_ = kNormal;
        close(data_connection);
        close(sock_pasv_);
				sock_pasv_ = -1;
      } else if(mode_ == kClient) {
        SendMessage("502 PORT mode not implemented.\n");
      } else {
        SendMessage("425 Use PASV or PORT first.\n");
      }
    }
    closedir(dir);
    chdir(original_dir);
  } else {
    SendMessage("530 Please login with USER and PASS.\n");
  }
  mode_ = kNormal;
}

/** MDTM command */
void Connection::FtpMdtm(const char* arg) {
  SendMessage("502 MDTM command not implemented yet\n");
}

/** MKD command */
void Connection::FtpMkd(const char* arg) {
  if(logged_in_){
    char current_dir[kBufferSize] = {0};
    getcwd(current_dir, kBufferSize);

    /* TODO: check if directory already exists with chdir? */
 
    if(arg[0] == '/') { /* Absolute path */
      if(mkdir(arg, S_IRWXU) == 0) {		
				char result[kBufferSize + 20] = {0};
				sprintf(result, "257 \"%s\" new directory created.\n", arg);
        SendMessage(result);
      } else {
        SendMessage("550 Failed to create directory. Check path or permissions.\n");
      }
    } else { /* Relative path */
      if(mkdir(arg, S_IRWXU) == 0) {
				char result[kBufferSize + 20] = {0};
        sprintf(result,"257 \"%s/%s\" new directory created.\n", current_dir, arg);
        SendMessage(result);
      }else{
        SendMessage("550 Failed to create directory. Check path or permissions.\n");
      }
    }
  } else {
    SendMessage("530 Please login with USER and PASS.\n");
  }
}

/** NLST command*/
void Connection::FtpNlst(const char* arg) {
  SendMessage("502 NLST command not implemented yet\n");
}

/** Noop command*/
void Connection::FtpNoop(const char* arg) {
	SendMessage("200 Nice to NOOP you!\n");
}

/** PASS command */
void Connection::FtpPass(const char* arg) {
  if(strlen(username_) > 0){
    logged_in_ = true;
    SendMessage("230 Login successful\n");
  } else {
    SendMessage("500 Invalid username or password\n");
  }
}

/** PASV command */
void Connection::FtpPasv(const char* arg) {
	if (logged_in_) {
    int ip[4];
		GetIp(sock_,ip);
    Port port; 
    GenPort(&port);

    /* Close previous passive socket */
    if (sock_pasv_ > 0) {
			close(sock_pasv_);
		}
    /* Start listening here, but don't accept the connection */
    sock_pasv_ = CreateSocket(256 * port.p1 + port.p2);
		char buff[255];
    sprintf(buff, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\n",
						ip[0], ip[1], ip[2], ip[3], port.p1, port.p2);
    SendMessage(buff);
		Print("%s\n", buff);
    mode_ = kServer;
	}
	else {
		SendMessage("530 Please login with USER and PASS.\n");
	}
}

/** PORT command */
void Connection::FtpPort(const char* arg) {
  SendMessage("502 PORT command not implemented yet\n");
}

/** PWD command */
void Connection::FtpPwd(const char* arg) {
  if(logged_in_) {
    char current_dir[kBufferSize];
    char result[kBufferSize];
    memset(result, 0, kBufferSize);
    if (getcwd(current_dir, kBufferSize) != NULL){
			sprintf(result, "257 \"%s\"\n", current_dir);
      SendMessage(result);
    } else {
      SendMessage("550 Failed to get pwd.\n");
    }
  } else {
		SendMessage("530 Please login with USER and PASS.\n");
	}
}

/** QUIT command */
void Connection::FtpQuit(const char* arg) {
  SendMessage("221 Goodbye, friend. Have a good night.\n");
	status_ = kClosed;
}

/** RETR command */
void Connection::FtpRetr(const char* arg) {
  if (!logged_in_) {
    SendMessage("530 Please login with USER and PASS.\n");
    return;
  }	

  if (mode_ == kServer) { /* Passive mode */
    int fd;
    if(access(arg,R_OK) == 0 && (fd = open(arg,O_RDONLY))) {
      struct stat stat_buf;
      fstat(fd, &stat_buf);				
      SendMessage("150 Opening BINARY mode data connection.\n");		
      int data_connection = AcceptConnection(sock_pasv_);
      close(sock_pasv_);
      off_t offset = 0;
      int sent_total = SendFile(data_connection, fd, &offset, stat_buf.st_size);
      if(sent_total == stat_buf.st_size) {
        SendMessage("226 File send OK.\n");
      } else if (sent_total == 0) {
        SendMessage("550 Failed to read file.\n");
      } else {
        SendMessage("550 Failed to send file.\n");
      }
      close(fd);
      close(data_connection);
    } else {
      SendMessage("550 Failed to get file\n");
    }
  } else if (mode_ == kClient) {
    SendMessage("550 Please use PASV instead of PORT.\n");
  } else {
    SendMessage("425 Use PASV or PORT first.\n");
  }
}

/** RMD Command*/
void Connection::FtpRmd(const char* arg) {
  if (!logged_in_) {
    SendMessage("530 Please login with USER and PASS.\n");
    return;
  }
  if(rmdir(arg) == 0) {
    SendMessage("250 Requested file action okay, completed.\n");
  } else {
    SendMessage("550 Cannot delete directory.\n");
  }
}

/** RNFR command */
void Connection::FtpRnfr(const char* arg) {
  SendMessage("502 RNFR command not implemented yet\n");
}

/** RNTO command */
void Connection::FtpRnto(const char* arg) {
  SendMessage("502 RNTO command not implemented yet\n");
}

/** SITE command */
void Connection::FtpSite(const char* arg) {
  SendMessage("502 SITE command not implemented yet\n");
}

/** SIZE command*/
void Connection::FtpSize(const char* arg) {
  if (!logged_in_) {
    SendMessage("530 Please login with USER and PASS.\n");
    return;
  }
  struct stat statbuf;
  char filesize[128] = {0};
  if(stat(arg, &statbuf) == 0) { /* Success */
    sprintf(filesize, "213 %ld\n", statbuf.st_size);
    SendMessage(filesize);
  } else {
    SendMessage("550 Could not get file size.\n");
  }
}

/** STOR command */
void Connection::FtpStor(const char* arg) {
  if (!logged_in_) {
    SendMessage("530 Please login with USER and PASS.\n");
    return;
  } 
  if (mode_ == kNormal) {
    SendMessage("425 Use PASV or PORT first.\n");
    return;
  }
  FILE *file = fopen(arg,"w");
  if (file == NULL) {
    SendMessage("551 Cannot open file.\n");
    return;
  }
  int fd = fileno(file);
  if (mode_ == kServer) { /* Passive mode */
    int data_connection = AcceptConnection(sock_pasv_);
    close(sock_pasv_);
    int pipefd[2];
    if (pipe(pipefd) == -1) {
      SendMessage("451 Local error: cannot open pipe.\n");
      close(fd);
      close(data_connection);
      return;
    }
    SendMessage("125 Data connection already open; transfer starting.\n");
    int res = 1;
    const int buff_size = 8192;
    while ((res = splice(data_connection, 0, pipefd[1], NULL, buff_size, SPLICE_F_MORE | SPLICE_F_MOVE)) > 0) {
      splice(pipefd[0], NULL, fd, 0, buff_size, SPLICE_F_MORE | SPLICE_F_MOVE);
    }
    /* Internal error */
    if(res == -1){
      SendMessage("451 File send failed.\n");
    }else{
      SendMessage("226 File send OK.\n");
    }
    close(data_connection);
  } else { 
    SendMessage("550 Please use PASV instead of PORT.\n");
  }
  mode_ = kNormal;
  close(fd);
}

/** 
 * TYPE command.
 * BINARY only at the moment.
 */
void Connection::FtpType(const char* arg) {
  if (!logged_in_) {
    SendMessage("530 Please login with USER and PASS.\n");
    return;
  }
  if (arg[0] == 'I') {
    SendMessage("200 Switching to Binary mode.\n");
  } else if (arg[0] == 'A') {
    SendMessage("200 Switching to ASCII mode.\n");
  } else {
    SendMessage("504 Command not implemented for that parameter.\n");
  }
}

/** USER command */
void Connection::FtpUser(const char* arg) {
  if(strlen(arg) < 30){
    strcpy(username_, arg);
    SendMessage("331 User name okay, need password\n");
  }else{
    SendMessage("530 Invalid username\n");
  }
}

ssize_t SendFile(int out_fd, int in_fd, off_t * offset, size_t count)
{
    const int BUF_SIZE = 8192;
    off_t orig;
    char buf[BUF_SIZE];
    size_t toRead, numRead, numSent, totSent;

    if (offset != NULL) {
        /* Save current file offset and set offset to value in '*offset' */
        orig = lseek(in_fd, 0, SEEK_CUR);
        if (orig == -1)
            return -1;
        if (lseek(in_fd, *offset, SEEK_SET) == -1)
            return -1;
    }

    totSent = 0;
    while (count > 0) {
        toRead = count < BUF_SIZE ? count : BUF_SIZE;
        numRead = read(in_fd, buf, toRead);
        if (numRead == -1)
            return -1;
        if (numRead == 0)
            break;                      /* EOF */

        numSent = write(out_fd, buf, numRead);
        if (numSent == -1)
            return -1;
        if (numSent == 0) {               /* Should never happen */
            perror("sendfile: write() transferred 0 bytes");
            exit(-1);
        }

        count -= numSent;
        totSent += numSent;
    }

    if (offset != NULL) {
        /* Return updated file offset in '*offset', and reset the file offset
           to the value it had when we were called. */
        *offset = lseek(in_fd, 0, SEEK_CUR);
        if (*offset == -1)
            return -1;
        if (lseek(in_fd, orig, SEEK_SET) == -1)
            return -1;
    }
    return totSent;
}

/* String mappings for cmdlist */
static const char *command_str[] = {
  "ABOR", "CWD", "DELE", "LIST", "MDTM", "MKD", "NLST", "PASS", "PASV",
  "PORT", "PWD", "QUIT", "RETR", "RMD", "RNFR", "RNTO", "SITE", "SIZE",
  "STOR", "TYPE", "USER", "NOOP"
};

CommandType LookUpCommand(const char* cmd) {
  static std::map<std::string, CommandType> mapping;
  int count = sizeof(command_str) / sizeof(char*);
  if (mapping.empty()) {
    for (int i = 0; i < count; i++) {
      mapping[command_str[i]] = static_cast<CommandType>(i);
    }
  }
  return mapping.count(cmd) ? mapping[cmd] : Unknown;
}

/** 
 * Converts permissions to string. e.g. rwxrwxrwx 
 * @param perm Permissions mask
 * @param str Pointer to string representation of permissions, should be empty
 */
std::string PermissionString(int permission) {
  int curperm = 0;
  int flag = 0;
  int read = 0;
	int write = 0;
	int exec = 0;
  
  /* Flags buffer */
  char fbuff[3];
	std::string result;
  for(int i = 6; i >= 0; i -= 3){
    /* Explode permissions of user, group, others; starting with users */
    curperm = ((permission & ALLPERMS) >> i ) & 0x7;
    
    memset(fbuff,0,3);
    /* Check rwx flags for each*/
    read = (curperm >> 2) & 0x1;
    write = (curperm >> 1) & 0x1;
    exec = (curperm >> 0) & 0x1;
    sprintf(fbuff,"%c%c%c",read?'r':'-' ,write?'w':'-', exec?'x':'-');
    result += fbuff;
  }
	return result;
}

