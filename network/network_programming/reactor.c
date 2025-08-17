#include <sys/socket.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <error.h>
#include <netinet/in.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>
//tcp server

#define BUFFER_LENGTH	128

struct conn_item{
	int fd;
	char buffer[BUFFER_LENGTH];
	int idx;
};
struct conn_item connlist[1024] = {0};

void* client_thread(void* arg){
	int clientfd = *(int*)arg;
	while (1) {
		char buffer[128] = {0};
		//接受，返回接受长度；对方断开时，返回 0。
		int count = recv(clientfd,buffer,128,0);

		//注意这里要关闭连接！
		if(0 == count) {
			break;
		}
		send(clientfd,buffer,count,0);
		printf("clientfd: %d, count: %d, buffer: %s",
				clientfd,count,buffer);
	}
	close(clientfd);
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

    //epoll
	//参数没有意义，但是必须要有
	//最开始是指预期的IO大小，现在改成链表了。
	int epfd = epoll_create(1);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = sockfd;

	epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

	struct epoll_event events[1024] = {0};
	while (1) {
		int nready = epoll_wait(epfd, events, 1024, -1);
		for (int i = 0;i < nready;i++) {
			int connfd = events[i].data.fd;
			if (sockfd == connfd) {
				struct sockaddr_in clientaddr;
				socklen_t len = sizeof(clientaddr);
				int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
				printf("nready:%d,accept: %d\n",nready,clientfd);

				ev.events = EPOLLIN;
				ev.data.fd = clientfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);

				connlist[clientfd].fd = clientfd;
				memset(connlist[clientfd].buffer, 0, BUFFER_LENGTH);
				connlist[clientfd].idx = 0;

				printf("clientfd:%d\n",clientfd);
			}
			else if (events[i].events &EPOLLIN) {

				char* buffer = connlist[connfd].buffer;
				int idx = connlist[connfd].idx;
				int count = recv(connfd,buffer+idx,BUFFER_LENGTH-idx,0);
				

				if(0 == count) {
					printf("disconnect\n");
					epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
					close(connfd);
					continue;
				}
				connlist[connfd].idx += count;
				//发送数据
				send(connfd,buffer,count,0);
			}
		}
	}
	
	//close函数是系统调用，不是socket里面的东西
	getchar();
	//close(clientfd);
	//return 0;
}
