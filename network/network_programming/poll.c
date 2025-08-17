# include <stdio.h>
# include <unistd.h>
# include <string.h>
# include <error.h>

# include <netinet/in.h>
# include <sys/poll.h>

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
    //poll
    /*
    struct pollfd{
        int fd;
        events;
        revents;
    };
    */
    struct pollfd fds[1024] = {0};
    fds[sockfd].fd = sockfd;
    fds[sockfd].events = POLLIN;
    int maxfd = sockfd;
    while (1) {
        int nready = poll(fds, maxfd + 1, -1);
        if(fds[sockfd].revents & POLLIN){
            struct sockaddr_in clientaddr;
			socklen_t len = sizeof(clientaddr);
			int clientfd = accept(sockfd,(struct sockaddr*)&clientaddr,&len);

            fds[clientfd].fd = clientfd;
            fds[clientfd].events = POLLIN;
            if(clientfd > maxfd) maxfd = clientfd;
        }
        for (int i = sockfd+1;i <= maxfd;i++) {
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
				send(i, buffer, count, 0);
				printf("clientfd: %d, count: %d, buffer: %s\n",i,count,buffer);
			}
		}
    }
    return 0;
}
