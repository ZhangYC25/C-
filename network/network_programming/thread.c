# include <stdio.h>
# include <string.h>
# include <error.h>
# include <stdlib.h>
# include <unista.h>

# include <sys/socket.h>
# include <netinet/in.h>

# include <pthread.h>
#define PORT 2048

void* client_thread(void* arg){
    int clientfd = *(int*)arg;
    while (1) {
        //接收信息
        char buffer[128] = {0};
        int count = recv(clientfd, buffer, 128, 0);
        if(count == 0){
            close(clientfd);
            break;
        }
        //回显
        send(clientfd, buffer, count, 0);
        printf("clientfd: %d, count: %d, buffer: %s\n",
				clientfd,count,buffer);
        fflush(stdout);  // 强制刷新 stdout 缓冲区
    }
}

int main(){
    //create a socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //绑定 IP 和 端口
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);

    if(bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr)) == -1){
        perror("bind");
        return -1;
    }

    //监听 sockfd
    listen(sockfd, 10);

    while (1) {
        //创建 客户端 fd
        struct sockaddr_in clientaddr;
        socklen_t len = sizeof(clientaddr);
        
        //接受之后，创建新线程，让线程去做。
        int clientfd = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
        
        //
        int* fd_ptr = malloc(sizeof(int));
        *fd_ptr = clientfd;
        pthread_t thpread_id;
        pthread_create(&thpread_id, NULL, client_thread, fd_ptr);

        //分离线程确，保线程安全
        pthread_detach(thpread_id);
    }

}