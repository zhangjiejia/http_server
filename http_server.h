#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "lib/lua.h"
#include "lib/string.h"

#define	SERVER_PORT	            80
#define SERVER_LISTEN_BACKLOG   2
#define SERVER_RECV_BUFF_LEN    128
#define SERVER_REQUEST_MAX_LEN  4096
#define	SERVER_WORK_COUNT       2

int is_kill = 0;

void handle_signal(int signal);

int create_server_socket();

int accept_client_socket(int server_sock, struct sockaddr* addr, int* addr_len);

int epoll_add(int epoll_fd, int sock, unsigned int events);

int get_request(char* reqeust_buff, int sock);

int handle_request(int server_sock);