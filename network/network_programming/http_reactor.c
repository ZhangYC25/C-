#include<error.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>

#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>

#define ENABLE_HTTP_RESPONSE 1

#define BUFFER_LENGTH 1024
typedef int (*RCALLBACK)(int fd);
//listenfd
//EPOLLIN-->
int accept_cb(int fd);

//clientfd
int recv_cb(int fd);
int send_cb(int fd);

int set_evenr(int fd, int event, int flag);
int epfd = 0;

// fd,buff,callback
struct conn_item{
    int fd;
    char rbuffer[BUFFER_LENGTH];
    int rlen;

    char wbuffer[BUFFER_LENGTH];
    int wlen;

    char resource[BUFFER_LENGTH];

    union {
    RCALLBACK accept_callback;
    RCALLBACK recv_callback;
    } recv_t;
    RCALLBACK send_callback;
};
struct conn_item connlist[1024] = {0};

#if ENABLE_HTTP_RESPONSE

#define ROOT_DIR "/root/c-/network/network_programming"
typedef struct conn_item connection_t;

int http_request(connection_t *conn){

    //conn -> rbuffer;
    //conn -> wlen;
    return 0;
}

int http_response(connection_t *conn){
#if 0
    conn->wlen = sprintf(conn->wbuffer,
    "HTTP/1.1 200 OK\r\n"
    "Accept-Ranges: bytes\r\n"
    "Content-Length: 82\r\n"
    "Content-Type: Text/html\r\n"
    "Date: Sat, 06 Aug 2022 13:16:46 GMT\r\n\r\n"
    "<html><head><title>0voice.king</title></head><body><h1>King</h1></body></html>\r\n\r\n");
#else
    int filefd = open("index.html",O_RDONLY);
    struct stat stat_buf;
    fstat(filefd,&stat_buf);

    conn->wlen = sprintf(conn->wbuffer,
        "HTTP/1.1 200 OK\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: %ld\r\n"
        "Content-Type: Text/html\r\n"
        "Date: Sat, 06 Aug 2023 13:16:46 GMT\r\n\r\n",stat_buf.st_size);
    int count = read(filefd,conn->wbuffer + conn->wlen, BUFFER_LENGTH-conn->wlen);
    conn->wlen += count;
#endif
    return conn->wlen;
}
#endif

int set_event(int fd, int event, int flag){
    if(flag){ //1 add;0 modify
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = event;
        epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
    }else{
        struct epoll_event ev;
        ev.data.fd = fd;
        ev.events = event;
        epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);
    }
}

int accept_cb(int fd){
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);
    int clientfd = accept(fd,(struct sockaddr*)&clientaddr,&len);
    if(clientfd<0){
        return -1;
    }

    set_event(clientfd,EPOLLIN,1);

    connlist[clientfd].fd = clientfd;
    memset(connlist[clientfd].rbuffer,0,BUFFER_LENGTH);
    
    connlist[clientfd].rlen = 0;
    memset(connlist[clientfd].wbuffer,0,BUFFER_LENGTH);
    
    connlist[clientfd].wlen = 0;
    
    connlist[clientfd].recv_t.recv_callback = recv_cb;
    connlist[clientfd].send_callback = send_cb;
    return clientfd;
}

int recv_cb(int fd){
    char* buffer = connlist[fd].rbuffer;
    int idx = connlist[fd].rlen;
    int count = recv(fd,buffer+idx,BUFFER_LENGTH-idx,0);
    if(count == 0){
        epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
        close(fd);
        //continue;
        return -1;
    }
    connlist[fd].rlen += count;

    //!!!!!!
#if 0
    memcpy(connlist[fd].wbuffer,connlist[fd].rbuffer,connlist[fd].rlen);
    connlist[fd].wlen = connlist[fd].rlen;
    connlist[fd].rlen -= connlist[fd].rlen;
#else
    http_request(&connlist[fd]);
    http_response(&connlist[fd]);
#endif
    set_event(fd,EPOLLOUT,0);
    return count;
}

int send_cb(int fd){
    char* buffer = connlist[fd].wbuffer;
    int idx = connlist[fd].wlen;
    int count = send(fd,buffer,idx,0);
    //
    set_event(fd,EPOLLIN,0);
    return count;
}

int init_server(unsigned short port){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(struct sockaddr_in));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(port);

    if(-1 == bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr))){
        perror("bind");
        return -1;
    }

    listen(sockfd,10);
    return sockfd;
}
//tcp
int main(){
    int port_count = 10;
    int i = 0;
    unsigned short port = 2048;
    epfd = epoll_create(1);
    for(i;i<port_count;i++){
        int sockfd = init_server(port + i);
        connlist[sockfd].fd = sockfd;
        connlist[sockfd].recv_t.accept_callback = accept_cb;
        set_event(sockfd,EPOLLIN,1);
    }
 //epoll
 //全局变量
    struct epoll_event events[1024]={0};
    while (1)
    {
        int nready = epoll_wait(epfd,events,1024,-1);
        int i = 0;
        for(i;i<nready;i++){
            int commfd = events[i].data.fd;
            if(events[i].events & EPOLLIN){
                int count = connlist[commfd].recv_t.recv_callback(commfd);
            }else if(events[i].events & EPOLLOUT){
                int count = connlist[commfd].send_callback(commfd);
            }
        }
    }
    
    getchar();
    //close(clientfd);
}