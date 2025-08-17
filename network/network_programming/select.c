# include <stdio.h>
# include <string.h>
# include <error.h>
# include <unistd.h>

# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/select.h>

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

    //select 只关注 可读 事件
    /*
	maxfd:用于内部循环遍历
	rset:可读集合，是一个数组bit，用于可读判断
	只要有时间，就会返回
	select(maxfd,rset,wset,eset,timeout)
	*/
    fd_set rfds, rset;
    //bit位全部清空
    FD_ZERO(&rfds);
    //把某位 置 1
    FD_SET(sockfd, &rfds);
    int maxfd = sockfd;
    while (1) {
        rset = rfds;
        //for(i=..;i < ..;i++)
        int nready = select(maxfd+1, &rset, NULL, NULL, NULL);

        if(FD_ISSET(sockfd, &rset)){
            struct sockaddr_in clientaddr;
            socklen_t len = sizeof(clientaddr);
            int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);

            FD_SET(clientfd, &rfds);
            if(clientfd > maxfd) maxfd = clientfd;
        }
        for(int i = sockfd + 1;i <= maxfd;i++){
            if(FD_ISSET(i, &rset)){
                char buffer[128] = {0};
                int count = recv(i, buffer, 128, 0);
                if(0 == count){
                    FD_CLR(i, &rfds);
                    close(i);
                    continue;
                }
                //发送数据
				send(i,buffer,count,0);
				printf("clientfd: %d, count: %d, buffer: %s\n",i,count,buffer);
            }
        }
    }
    return 0;
}