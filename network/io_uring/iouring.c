// io_uring tcp server
// multhread select/poll/epoll resctor coroutine iouring
# include <liburing.h>

# include <stdio.h>
# include <string.h>
# include <unistd.h>

# include <sys/socket.h>
# include <netinet/in.h>


#define ENTEIES_LENGTH 4096
#define PORT 2048


enum {
    READ,
    WRITE,
    ACCEPT
};

struct conninfo{
    int connfd;
    int type;
};

void set_write_event(struct io_uring* ring, int fd, 
                const void *buf, size_t len, int flags){
    //register
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    io_uring_prep_send(sqe, fd, buf, len, flags);
    struct conninfo ci ={
        .connfd = fd,
        .type = WRITE
    };
    memcpy(&sqe->user_data, &ci, sizeof(struct conninfo));
}

void set_read_event(struct io_uring* ring, int fd, 
             void *buf, size_t len, int flags){
    //register
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    io_uring_prep_recv(sqe, fd, buf, len, flags);
    struct conninfo ci ={
        .connfd = fd,
        .type = READ
    };
    memcpy(&sqe->user_data, &ci, sizeof(struct conninfo));
}

void set_accept_event(struct io_uring* ring, int fd, struct sockaddr* cliaddr,
                        socklen_t* clilen, unsigned flags){
    //register
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    io_uring_prep_accept(sqe, fd, cliaddr, clilen, flags);
    struct conninfo ci ={
        .connfd = fd,
        .type = ACCEPT
    };
    memcpy(&sqe->user_data, &ci, sizeof(struct conninfo));
}
int main(){
	
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serveraddr, clientaddr;
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);
    if(-1 == bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr))){
        return -2;
    }
    listen(sockfd, 10);

    //初始化，参数化两个队列，squeue和cqueue
    struct io_uring_params params;
    memset(&params, 0, sizeof(params));
    struct io_uring ring;
    io_uring_queue_init_params(ENTEIES_LENGTH, &ring, &params);

    //register
    //struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    socklen_t len = sizeof(clientaddr);
    set_accept_event(&ring, sockfd, (struct sockaddr*)&clientaddr, &len, 0);
    io_uring_submit(&ring);
    char buffer[128] = {0};

    while (1) {

        //注册一个cqueue
        struct io_uring_cqe* cqe;
        //阻塞等待至少一个io操作完成，返回至少一个cqe。ref返回0就成功
        io_uring_wait_cqe(&ring, &cqe);
        
        //取cqe中的io进行处理
        struct io_uring_cqe* cqes[10];
        int cqecount = io_uring_peek_batch_cqe(&ring, cqes, 10);
        
        int i = 0;
        unsigned count = 0;
        for (i;i < cqecount;i++) {
            cqe = cqes[i];
            count ++;
            struct conninfo ci;
            memcpy(&ci, &cqe->user_data, sizeof(struct conninfo));
            if (ci.type == ACCEPT) {
                int connfd = cqe -> res;
                
                set_read_event(&ring, connfd, buffer, 128, 0);
                io_uring_submit(&ring);

            } else if (ci.type == READ) {
                int bytes_read = cqe -> res;
                if (cqe -> res == 0) {
                    close(ci.connfd);
                } else if (cqe -> res < 0) {

                } else {
                    //buffer内容残留
                    buffer[bytes_read] = '\0';
                    printf("buffer: %s\n",buffer);
                    set_write_event(&ring, ci.connfd, buffer, bytes_read, 0);
                    io_uring_submit(&ring);
                }
            } else if (ci.type == WRITE) {
                set_read_event(&ring, ci.connfd, buffer, 128, 0);
                io_uring_submit(&ring);
            }
        }
        io_uring_cq_advance(&ring,count);
    }
}
