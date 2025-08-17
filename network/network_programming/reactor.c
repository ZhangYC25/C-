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
#if 0
	//定义客户端
	struct sockaddr_in clientaddr;
	socklen_t len = sizeof(clientaddr);
	printf("accept!\n");
	int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
#if 0 //if 中的 if

#else
	while (1) {
	char buffer[128] = {0};
	//接受，返回接受长度；对方断开时，返回 0。
	int count = recv(clientfd,buffer,128,0);
	if(0 == count) break;

	//发送数据
	send(clientfd,buffer,count,0);
	/*输出 3，4
	stdin:0 stdout:1 stderr:2
	文件描述符
	*/
	printf("sockfd: %d, clientfd: %d, count: %d, buffer: %s",sockfd,clientfd,count,buffer);
	}
#endif //58 if 结束

#elif 0
	while (1) {
		struct sockaddr_in clientaddr;
		socklen_t len = sizeof(clientaddr);
		//printf("accept!\n");
		int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
		
		//接受之后，创建新线程，让线程去做。
		pthread_t thid;
		//线程id，线程属性，入口函数，入口函数参数
		pthread_create(&thid, NULL, client_thread, &clientfd);
		}
#elif 0 //select()
	/*
	maxfd:用于内部循环遍历
	rset:可读集合，是一个数组bit，用于可读判断
	只要有时间，就会返回
	select(maxfd,rset,wset,eset,timeout)
	*/
	fd_set rdfs, rset;
	//bit位全部清空
	FD_ZERO(&rdfs);
	//把某位 置 1
	FD_SET(sockfd,&rdfs);

	int maxfd = sockfd;
	while (1) {
		rset = rdfs;
		//for(i=..;i < ..;i++)
		int nready = select(maxfd+1, &rset, NULL, NULL, NULL);
		//判断是否置 1
		if (FD_ISSET(sockfd, &rset)) {
			struct sockaddr_in clientaddr;
			socklen_t len = sizeof(clientaddr);
			int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
			printf("nready:%d,accept: %d\n",nready,clientfd);
			
			FD_SET(clientfd,&rdfs);
			if (maxfd < clientfd) {
				maxfd = clientfd;
			}
		}
		for(int i = sockfd+1;i <= maxfd;i++){
			if (FD_ISSET(i,&rset)){
				char buffer[128] = {0};
				int count = recv(i,buffer,128,0);
				if(0 == count) {
					printf("disconnect\n");
					FD_CLR(i,&rdfs);
					close(i);
					break;
				}
				//发送数据
				send(i,buffer,count,0);
				printf("clientfd: %d, count: %d, buffer: %s",i,count,buffer);
			}
		}
	}
#elif 0 //poll
	struct pollfd fds[1024] = {0};
	fds[sockfd].fd = sockfd;
	fds[sockfd].events = POLLIN;
	int maxfd = sockfd;
	while (1) {

		int nready = poll(fds,maxfd+1,-1);
		if (fds[sockfd].revents & POLLIN) {
			struct sockaddr_in clientaddr;
			socklen_t len = sizeof(clientaddr);
			int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
			printf("nready:%d,accept: %d\n",nready,clientfd);

			fds[clientfd].fd = clientfd;
			fds[clientfd].events = POLLIN;
			maxfd = clientfd;
		}

		int i = 0;
		for (i = sockfd+1;i<=maxfd;i++) {
			if (fds[i].revents & POLLIN) {
				char buffer[128] = {0};
				int count = recv(i,buffer,128,0);
				if(0 == count) {
					printf("disconnect\n");
					fds[i].fd = -1;
					fds[i].revents = 0;
					close(i);
					continue;
				}
				//发送数据
				send(i,buffer,count,0);
				printf("clientfd: %d, count: %d, buffer: %s",i,count,buffer);
			}
		}
	}
#else //epoll
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

				printf("clientfd:%d\n",clientfd);
			}
			else if (events[i].events &EPOLLIN) {
				char buffer[128] = {0};
				int count = recv(connfd,buffer,128,0);
				if(0 == count) {
					printf("disconnect\n");
					epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
					close(connfd);
					continue;
				}
				//发送数据
				send(connfd,buffer,count,0);
			}
		}
	}
	
#endif
	//close函数是系统调用，不是socket里面的东西
	getchar();
	//close(clientfd);
	//return 0;
}
