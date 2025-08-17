# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <error.h>

# include <netinet/in.h>
# include <sys/epoll.h>

# define PORT 2048
int main(){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);
    if(-1 == bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr))){
        perror("bind");
        return -1;
    }
    listen(sockfd, 10);

    //epoll
    /*
    struct epoll_event{
        events;
        struct data;
    };
    epoll_create()
    epoll_ctl()
    epoll_wait()
    */
    int epfd = epoll_create(1);

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = sockfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);

    struct epoll_event evs[1024] = {0};
    while (1) {
        int nready = epoll_wait(epfd, evs, 1024, -1);
        for(int i = 0;i < nready;i++){
            int connfd = evs[i].data.fd;
            if(connfd == sockfd){
                struct sockaddr_in clientaddr;
				socklen_t len = sizeof(clientaddr);
				int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr,&len);
				printf("nready:%d,accept: %d\n",nready,clientfd);

                ev.events = EPOLLIN;
                ev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &ev);
            }
            else if (evs[i].events & EPOLLIN) {
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
    return 0;
}