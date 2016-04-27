#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <pwd.h>

#ifdef DEBUG
#define D
#else
#define D for(;0;)
#endif

#define true  1
#define false 0

const char * _good_color = "\x1b[35;1m"; // <-- That's racist.
const char *  _err_color = "\x1b[31;1m";
const char *  _std_color = "\x1b[0m";

const unsigned short _queue_length = 5;

const char * OK  = "\x1b[34;1m[ \x1b[32;1mok \x1b[34;1m]\x1b[0m ";
const char * ERR = "\x1b[34;1m[ \x1b[33;1m!! \x1b[34;1m]\x1b[0m ";

void process_request(int fd);
void printOk();
void printErr();

void sigchld_handlr(int signum)
{ for(int pid = -1; pid = waitpid(-1, NULL, WNOHANG) > 0;); }

int main(int argc, char ** argv)
{
  struct sigaction chld;
  chld.sa_handler = sigchld_handlr;
  sigemptyset(&chld.sa_mask);
  chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;

  if (sigaction(SIGCHLD, &chld, NULL) == -1) {
    perror("sigchild");
    _exit(-1);
  }
  
  const unsigned short port = 8888;
  
  struct sockaddr_in serverIP;

  // Clear server socket
  memset(&serverIP, 0, sizeof(serverIP));
  serverIP.sin_family = AF_INET;             // internet mode
  serverIP.sin_addr.s_addr = INADDR_ANY;     // accept from any host
  serverIP.sin_port = htons((u_short) port); // accept on port 8888

  int masterSocket = socket(PF_INET, SOCK_STREAM, 0);
  if (masterSocket < 0) {
    perror("socket");
    _exit(1);
  }

  int error = bind(masterSocket, (struct sockaddr*) &serverIP, sizeof(serverIP));

  if (error) {
    perror("bind");
    _exit(2);
  }

  error = listen(masterSocket, _queue_length);
  if (error) {
    perror("listen");
    _exit(3);
  }

  for (; true;) {
    struct sockaddr_in client_ip; int pid = -1;
    ssize_t alen = sizeof(client_ip);
    int slave = accept(masterSocket, (struct sockaddr*) &client_ip,
		       (socklen_t*) &alen);
    if (slave < 0) {
      perror("accept");
      _exit(4);
    }
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  
    printOk("client connection received");
    process_request(slave);
  } return 0;
}

void printOk(const char * msg) {
  size_t len = strlen(msg) + strlen("[ ok ] ");
  fprintf(stderr, _good_color);
  struct winsize w;
  ioctl(1,TIOCGWINSZ, &w);
  fprintf(stderr, msg);
  for(; w.ws_col - len++; fprintf(stderr, " "));
  fprintf(stderr, OK);
  fprintf(stderr, "\x1b[0m\n");
}

void printErr(const char * msg) {
  size_t len = strlen(msg) + strlen("[ !! ] ");
  fprintf(stderr, _err_color);
  struct winsize w;
  ioctl(1,TIOCGWINSZ, &w);
  fprintf(stderr, msg);
  for(; w.ws_col - len++; fprintf(stderr, " "));
  fprintf(stderr, ERR);
  fprintf(stdout, "\x1b[0m\n");
}

/**
 * @brief Process bcc request.
 *
 * This function evalutes a bcc program and serves,
 * as a response, the output of the bcc program.
 *
 * @param fd The file descriptor of the requesting client.
 */
void process_request(int fd)
{
  const unsigned short max_request = 1024;
  char * _get_request = (char*) calloc(max_request + 1, sizeof(char));
  ssize_t request_len = 0;
  int n;

  unsigned char next_char = '\0';
  unsigned char last_char = '\0';

  int pid = -1;
  
  for (int n = 0; request_len < max_request && (n = read(fd, &next_char,
							 sizeof(next_char)) > 0);) {
    if (strstr(_get_request, "done!")) {
      _get_request[request_len++] = '\n';
      break;
    }
    if (next_char == '\r') continue; // Hacky, yes.
    else {
      if (request_len) last_char = _get_request[request_len-1];
      _get_request[request_len++] = next_char;
    }
  }
  char * copy = _get_request;
  pid = fork();

  if (pid == 0) {
    int fdpipe[2]; int pid2;
    if (pipe(fdpipe) < 0) {
      printErr("failed to construct pipe");
      _exit(8);
    } else if ((pid2 = fork()) < 0) {
      printErr("failed to fork process");
      _exit(9);
    } else if (pid2 == 0) {
      dup2(fdpipe[0], 0);
      dup2(2, fd);// write errors to host
      close(2);
      dup2(1, fd);// write other  to host
      close(1);
      close(fdpipe[0]); close(1);
      close(fdpipe[1]);
      if (execlp("bcc", "bcc", "out.s", NULL)) {
	printErr("bcc failed -- is it installed?");
      } 
    } else {
      close(fdpipe[0]);
      write(fdpipe[1], _get_request, request_len);
      close(fdpipe[1]);
      
      waitpid(pid2, NULL, 0);
      char * buff = (char*) calloc(1024, sizeof(char));
      sprintf(buff, "process [%d] (bcc) has exited.", pid2);
      printOk(buff); free(buff);
      _exit(0);
    }
  } else if (pid > 0) {
    struct winsize w;
    ioctl(1,TIOCGWINSZ, &w);
    waitpid(pid, NULL, 0);
    char * buff = (char*) calloc(1024, sizeof(char));
    sprintf(buff, "process [%d] has exited.", pid);
    printOk(buff); free(buff);
  } free(copy);

  /** generate out.txt **/
  if (write(fd, "\n", 1) < 0) {
    printErr("Client disconnected unexpectedly.");
    return;
  }

  int fdpipe[2];
  if (pipe(fdpipe) < 0) {
    printErr("Failed to construct pipe.");
    close(fd); return;
  } else if ((pid = fork()) < 0) {
    printErr("Failed to fork process");
    close(fd); return;
  } else if (pid == 0) {
    close(fdpipe[0]);

    dup2(fdpipe[1], 1); // stdout to pipe
    dup2(fdpipe[1], 2); // stderr to pipe

    close(fdpipe[1]);

    execlp("./out", "./out", NULL);
    _exit(0);
  } else {
    char buff[2048]; memset(buff, 0, 2048);
    close(fdpipe[1]);

    for(;read(fdpipe[0], buff, sizeof(buff)););
    waitpid(pid, NULL, 0);
    char buff2[64]; memset(buff2, 0, 64);
    sprintf(buff2, "process [%d] has exited", pid);
    printOk(buff2);
    /** Send output to client **/
    D fprintf(stderr, "bufflen: %zd\n", strlen(buff));
    for (char * b = buff; *b && write(fd, b++, 1); b - buff != strlen(buff));
    printOk("closing client connection.");
  } 
}
