#include "http_server.h"

int main() {
    int server_socket = create_server_socket();
    
    if (server_socket <= 0) {
        printf("create_server_socket: %s\n", strerror(errno));
        return -1;
    }
    
    int status;
    pid_t pids[SERVER_WORK_COUNT];
    
    for(int i = 0; i < SERVER_WORK_COUNT; ++i) {
        pids[i] = fork();
        if(pids[i] == 0) return handle_request(server_socket);
    }
    
    close(server_socket);
    getchar();
    
    for(int i = 0; i < SERVER_WORK_COUNT; ++i)
        kill(pids[i] , SIGHUP);
    
    for(int i = 0; i < SERVER_WORK_COUNT; ++i)
        waitpid(pids[i] , &status, 0);
    
    printf("all worker was killed\n");
    
    return 1;
}

int create_server_socket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0), client_socket;
	
	if(sock <= 0) return -1;
    
    struct sockaddr_in addr;
    unsigned int addr_len = sizeof(struct sockaddr_in);
    
    memset(&addr, 0, addr_len);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = htons(INADDR_ANY);
    
    if(bind(sock, (struct sockaddr*)&addr, addr_len) != 0) return -1;
    
    if(listen(sock, SERVER_LISTEN_BACKLOG) != 0) return -1;
    
    fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);
    
    return sock;
}

int accept_client_socket(int server_socket, struct sockaddr* addr, int* addr_len) {
    int sock = accept(server_socket, addr, addr_len);
    if(sock <= 0 || fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) == -1) return -1;
    return sock;
}

int epoll_add(int epoll_fd, int sock, unsigned int events) {
    struct epoll_event event;
    event.events = events;
    event.data.fd = sock;
    return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event);
}

int get_request(char* reqeust_buff, int sock) {
    int len = 0;
    int request_len = 0;
    char* buff = reqeust_buff;
    memset(buff, 0, SERVER_REQUEST_MAX_LEN);
    
    while(1) {
        if(request_len > SERVER_REQUEST_MAX_LEN) return -1;
        
        buff = reqeust_buff + request_len;
        len = recv(sock, buff, SERVER_RECV_BUFF_LEN, 0);
        
        if(len <= 0) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;
            else if(errno == EINTR) continue;
            else printf("len = %d, errno = %d\n", len, errno);
            return -1;
        }

        request_len += len;
    }
    
    return request_len;
}

void handle_signal(int signal) {
    if(signal == SIGHUP)
        is_kill = 1;
}

int handle_request(int server_sock) {
    pid_t pid = getpid();
    
    signal(SIGHUP, handle_signal);
    prctl(PR_SET_PDEATHSIG, SIGHUP);
    
    int epoll_fd = epoll_create(SERVER_LISTEN_BACKLOG);
    struct epoll_event events[SERVER_LISTEN_BACKLOG];
    
    if(epoll_add(epoll_fd, server_sock, EPOLLIN) == -1) {
        printf("epoll_add_1: %s\n", strerror(errno));
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in client_addr;
    unsigned int addr_len = sizeof(struct sockaddr_in);
    char* reqeust_buff = (char*)malloc(SERVER_REQUEST_MAX_LEN);
    lua_State* context = create_lua_context();
    
    if(!context) {
        printf("create_lua_context failed\n");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    
    printf("create worker process sucess! the pid is %d\n", pid);
    
    while(!is_kill) {
        int event_count = epoll_wait(epoll_fd, events, SERVER_LISTEN_BACKLOG, 0);
        
        if(event_count == 0)
            continue;
        
        for(unsigned int i = 0; i < event_count; ++i) {
            if(events[i].data.fd == server_sock) {
                int client_socket = accept_client_socket(server_sock, (struct sockaddr*)&client_addr, &addr_len);
                
                printf("accept by pid: %d\n", pid);
                
                if(client_socket <= 0) {
                    printf("accept_client_socket: %s\n", strerror(errno));
                    continue;
                }
                
                printf("handle by pid: %d, client_socket: %d\n", pid, client_socket);
                
                if(epoll_add(epoll_fd, client_socket, EPOLLIN | EPOLLET) == -1) {
                    printf("epoll_add_2: %s, pid: %d\n", strerror(errno), pid);
                    close(client_socket);
                }
            }
            else if(events[i].events & EPOLLIN) {
                int request_len = get_request(reqeust_buff, events[i].data.fd);
                
                if(request_len == -1) {
                    printf("get_request: %s\n", strerror(errno));
                    close(events[i].data.fd);
                    continue;
                }
                
                const char* response = handle_by_lua(context, reqeust_buff);
                int len = get_char_index_of('\0', response);
                send(events[i].data.fd, response, len, 0);
                
                close(events[i].data.fd);
            }
        }
    }
    
    close(server_sock);
    free(reqeust_buff);
    close_lua_context(context);
    
    printf("kill worker process sucess! the pid is %d\n", pid);
    
    exit(EXIT_SUCCESS);
}