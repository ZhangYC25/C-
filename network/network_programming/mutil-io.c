#include <sys/socket.h>
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
#if 0

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
#endif
#else
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
#endif
	//close函数是系统调用，不是socket里面的东西
	getchar();
	//close(clientfd);
	//return 0;
}
