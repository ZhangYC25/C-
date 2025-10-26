#include <sys/socket.h>
#include <sys/epoll.h>
#include <error.h>
#include <netinet/in.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
//tcp server

#define BUFFER_LENGTH	1024
#define PORT 2048

typedef int (*RCALLBACK)(int fd);

//fd, buffer, callback
struct conn_item{
	int fd;

	char rbuffer[BUFFER_LENGTH];
	int rlen;
	char wbuffer[BUFFER_LENGTH];
	int wlen;

	union {
		RCALLBACK accept_callback;
		RCALLBACK recv_callback;
	} recv_t;
	RCALLBACK send_callback;
};

//libevent

struct conn_item connlist[1024] = {0};
int epfd;

//listenfd
int accept_cb(int fd);

//clientfd
int recv_cb(int fd);
int send_cb(int fd);

void set_event(int fd, int event, int flag){
	struct epoll_event ev;
	if (flag) {//1.add;0.mod
		ev.data.fd = fd;
		ev.events = event;
		epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
	} else {
		ev.data.fd = fd;
		ev.events = event;
		epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
	}
}

int accept_cb(int fd){
	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);
	int clientfd = accept(fd, (struct sockaddr*)&clientaddr,&len);
	if(clientfd < 0) return -1;
	
	set_event(clientfd, EPOLLIN, 1);

	connlist[clientfd].fd = clientfd;
	memset(connlist[clientfd].rbuffer, 0, BUFFER_LENGTH);
	memset(connlist[clientfd].wbuffer, 0, BUFFER_LENGTH);
	connlist[clientfd].rlen = 0;
	connlist[clientfd].wlen = 0;
	connlist[clientfd].recv_t.recv_callback = recv_cb;
	connlist[clientfd].send_callback = send_cb;

	return clientfd;
}

int recv_cb(int fd){
	char* buffer = connlist[fd].rbuffer;
	int idx = connlist[fd].rlen;

	int count = recv(fd, buffer+idx, BUFFER_LENGTH-idx, 0);
	if(0 == count){
		printf("disconnect\n");
		epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
		close(fd);
		return -1;
	}
	connlist[fd].rlen += count;

	memcpy(connlist[fd].wbuffer,connlist[fd].rbuffer,connlist[fd].rlen);
	connlist[fd].wlen = connlist[fd].rlen;
	//！！！！！关注读！！！！！
	set_event(fd, EPOLLOUT, 0);
	return count;
}

int send_cb(int fd){
	char* buffer = connlist[fd].wbuffer;
	int idx = connlist[fd].wlen;
	int send_count = send(fd, buffer, idx, 0);
	
	//再次modify
	set_event(fd, EPOLLIN, 0);

	return send_count;
}

int main(int argc, char* argv[]){
	
	//创建socket
	int sockfd = socket(AF_INET,SOCK_STREAM,0);

	//绑定IP地址+端口
	struct sockaddr_in serveraddr;
	memset(&serveraddr,0,sizeof(struct sockaddr_in));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(2048);

	//绑定
	if(-1 == bind(sockfd,(struct sockaddr*)&serveraddr,sizeof(struct sockaddr))){
		perror("bind");
		return -1;
	}

	//监听
	listen(sockfd,10);

	connlist[sockfd].fd = sockfd;
	connlist[sockfd].recv_t.accept_callback = accept_cb;
    //epoll
	//参数没有意义，但是必须要有
	//最开始是指预期的IO大小，现在改成链表了。
	epfd = epoll_create(1);
	set_event(sockfd, EPOLLIN, 1);
	struct epoll_event events[1024] = {0};
	while (1) {
		int nready = epoll_wait(epfd, events, 1024, -1);
		for (int i = 0;i < nready;i++) {
			int connfd = events[i].data.fd;
			if (events[i].events & EPOLLIN) {
				int recv_count = connlist[connfd].recv_t.recv_callback(connfd);
				printf("clientfd: %d, count: %d, buffer: %s\n",connfd,recv_count,connlist[connfd].rbuffer);
			
			} else if(events[i].events & EPOLLOUT){
				int send_count = connlist[connfd].send_callback(connfd);
				printf("send --> buffer: %s\n",connlist[connfd].wbuffer);
			}
		}
	}
	return 0;
}
