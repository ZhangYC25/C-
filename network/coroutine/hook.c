/*
This code is quite shoddy, but it’s sufficient for everyone to 
understand how coroutines work. Going forward, I will continue 
to refine this basic coroutine code and turn it into a truly 
usable coroutine implementation.
*/


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <poll.h>
#include <dlfcn.h>
#include <ucontext.h>
#include <sys/epoll.h>

#define MAX_FDS 1024
#define MAX_EVENTS 1024
#define BUFFER_LENGTH 1024
static ucontext_t g_main_ctx;           // 主上下文
static ucontext_t g_co_ctx[MAX_FDS];    // 每个 fd 对应一个协程上下文
static void* g_stack[MAX_FDS] = {0};    // 记录栈指针，用于释放
static int g_inited = 0;

static int epollfd = -1;
static int sockfd = -1;

struct conn_item{
        int fd;

        char rbuffer[BUFFER_LENGTH];
        int rlen;
        char wbuffer[BUFFER_LENGTH];
        int wlen;
};

//libevent

struct conn_item connlist[1024] = {0};

// Hook 函数指针 
typedef ssize_t (*read_t)(int fd, void* buf, size_t count);
read_t read_f = NULL;

typedef ssize_t (*write_t)(int fd, const void* buf, size_t count);
write_t write_f = NULL;

//安全打印
void safe_print(const char* msg) {
    write_f(1, msg, strlen(msg));
}

void safe_print_int(int n) {
    char buf[16] = {0};
    int len = 0;
    if (n == 0) buf[len++] = '0';
    while (n) {
        buf[len++] = '0' + n % 10;
        n /= 10;
    }
    for (int i = len - 1; i >= 0; i--) {
        write_f(1, &buf[i], 1);
    }
}

//Hook 的 read
ssize_t read(int fd, void* buf, size_t count) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;

    // 检查是否可读
    if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) < 0) {
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);  // 第一次添加
    }

    struct epoll_event ready_ev;
    // 非阻塞检查,这里其实不应该这样做，也有严重的逻辑错误
    int ready = epoll_wait(epollfd, &ready_ev, 1, 0);  

    if (ready <= 0 || ready_ev.data.fd != fd) {
        safe_print("read: fd="); safe_print_int(fd); safe_print(" not ready, yielding...\n");
        swapcontext(&g_co_ctx[fd], &g_main_ctx);  // 挂起协程
    }

    // 真正读数据
    ssize_t ref = read_f(fd, buf, count);
    if (ref > 0) {
        safe_print("read: fd="); safe_print_int(fd); safe_print(" got data, bytes="); safe_print_int(ref); safe_print("\n");
    }
    return ref;
}

ssize_t write(int fd, const void* buf, size_t count) {
    safe_print("write: fd="); safe_print_int(fd); safe_print(" len="); safe_print_int(count); safe_print("\n");
    return write_f(fd, buf, count);
}

void init_hook(void) {
    if (!g_inited) {
        read_f = dlsym(RTLD_NEXT, "read");
        write_f = dlsym(RTLD_NEXT, "write");
        if (!read_f || !write_f) {
            safe_print("dlsym failed!\n");
            exit(1);
        }
        g_inited = 1;
    }
}

//协程任务
void client_task(int clientfd) {
    char* buffer = connlist[clientfd].rbuffer;
    int rlen = connlist[clientfd].rlen;

    safe_print("client_task: fd="); safe_print_int(clientfd); safe_print(" started\n");

    while (1) {
        int n = read(clientfd, buffer+rlen, sizeof(buffer) - 1 - rlen);
        connlist[clientfd].rlen += n;

        if (n <= 0) break;
        buffer[n] = '\0';
        write(clientfd, buffer, connlist[clientfd].rlen);
        connlist[clientfd].rlen = 0;
        safe_print("echo: '"); safe_print(buffer); safe_print("'\n");
    }

    close(clientfd);
    safe_print("client "); safe_print_int(clientfd); safe_print(" closed\n");

    // 清理栈
    free(g_stack[clientfd]);
    g_stack[clientfd] = NULL;
}

int main() {
    init_hook();

    // 创建监听 socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return -1; }

    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in servaddr = {0};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(2048);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind");
        return -1;
    }

    listen(sockfd, 10);

    // 创建 epoll
    epollfd = epoll_create(1);

    // 将 listenfd 加入 epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &ev);

    connlist[sockfd].fd = sockfd;
    memset(connlist[sockfd].rbuffer, 0, BUFFER_LENGTH);
    connlist[sockfd].rlen = 0;
    memset(connlist[sockfd].wbuffer, 0, BUFFER_LENGTH);
    connlist[sockfd].wlen = 0;
    // 获取主上下文
    getcontext(&g_main_ctx);

    struct epoll_event events[MAX_EVENTS];
    safe_print("Server started, listening on port 2048...\n");

    //事件循环
    while (1) {
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, 10);  // 阻塞等待事件

        for (int i = 0; i < nfds; i++) {
            int connfd = events[i].data.fd;

            if (connfd == sockfd) {
                // 新连接
                struct sockaddr_in clientaddr;
                socklen_t len = sizeof(clientaddr);
                int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
                if (clientfd < 0) continue;

                if (clientfd >= MAX_FDS) {
                    safe_print("clientfd too large: "); safe_print_int(clientfd); safe_print("\n");
                    close(clientfd);
                    continue;
                }
                connlist[clientfd].fd = clientfd;
                memset(connlist[clientfd].rbuffer, 0, BUFFER_LENGTH);
                connlist[clientfd].rlen = 0;
                memset(connlist[clientfd].wbuffer, 0, BUFFER_LENGTH);
                connlist[clientfd].wlen = 0;
                
                safe_print("New client: "); safe_print_int(clientfd); safe_print("\n");

                // 创建协程上下文
                ucontext_t* ctx = &g_co_ctx[clientfd];
                g_stack[clientfd] = malloc(8192);
                if (!g_stack) { 
                    close(clientfd); 
                    return -1; 
                }

                getcontext(ctx);
                ctx->uc_stack.ss_sp = g_stack[clientfd];
                ctx->uc_stack.ss_size = 8192;
                ctx->uc_link = &g_main_ctx;

                // 创建协程
                makecontext(ctx, (void(*)())client_task, 1, clientfd);

                // 立即启动协程
                swapcontext(&g_main_ctx, ctx);
                // 协程 yield 后会回到这里
            }
            else {
                // 普通 clientfd 可读，唤醒协程
                if (connfd >= 0 && connfd < MAX_FDS && g_stack[connfd]) {
                    safe_print("Waking up coroutine for fd="); safe_print_int(connfd); safe_print("\n");
                    swapcontext(&g_main_ctx, &g_co_ctx[connfd]);
                    // 协程执行完 read 后会再次 yield 回来
                }
            }
        }
    }

    close(sockfd);
    close(epollfd);
    return 0;
}

