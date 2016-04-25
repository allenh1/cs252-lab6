#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <sys/types.h>
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

const unsigned short _queue_length = 5;

void process_request(int fd);

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
    D fprintf(stderr, "received client connection!\n");
    if (slave < 0) {
      perror("accept");
      _exit(4);
    }

    // Process the request
    pid = fork();
    if (pid == -1) {
      perror("fork");
      _exit(5);
    } else if (pid == 0) {
      // Child process
      process_request(slave);
      return 0;
    } else close(slave);
  } return 0;
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

  int state = 0;

  /**
   * @todo fork bcc & evaluate.
   */

  int pid = -1;
  char * _args[3] = {
    strdup("bcc"),
    strdup("temp.bc"),
    NULL
  };
 
  for (int n = 0; request_len < max_request && (n = read(fd, &next_char,
							 sizeof(next_char)) > 0);) {
    if (strstr(_get_request, "done!")) {
      _get_request[request_len++] = '\n';
      break;
    }
    if (next_char == '\r') continue; // Hacky, yes.
    if (request_len) last_char = _get_request[request_len-1];
    _get_request[request_len++] = next_char;
  } D fprintf(stderr, "I read: \"%s\"\n", _get_request); 
    
  _exit(2);
    
  FILE * _fd;
  if (!(_fd = fopen("out.bc", "w"))) {
    perror("fopen");
  }
  fprintf(_fd, "%s", _get_request);
  fflush(_fd); fclose(_fd);
}











