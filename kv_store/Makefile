
CC = gcc

FLAGS = -I ./Ntyco/core/ -L ./Ntyco/ -lntyco
SRCS = kvstore.c kvstore_array.c epoll_entry.c ntyco_entry.c rbtree.c mempool.c
TESTCASE_SRCS = test_qps.c
TESTCASE = test_qps
TARGET = kvstore
SUBDIR = ./Ntyco/

OBJS = $(SRCS:.c=.o)
#all:$(SUBDIR) $(APP)

#$(SUBDIR):

all: $(TARGET) $(TESTCASE)

$(TARGET):$(OBJS)
	$(CC) $^ -o $@ $(FLAGS)

$(TESTCASE):$(TESTCASE_SRCS)
	$(CC) $^ -o $@
%.o:%.c
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(OBJS) $(TARGET) $(TESTCASE)