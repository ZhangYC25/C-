
CC = gcc

FLAGS = -I ./Ntyco/core/ -L ./Ntyco/ -lntyco
SRCS = kvstore.c kvstore_array.c epoll_entry.c ntyco_entry.c
TARGET = kvstore
SUBDIR = ./Ntyco/

OBJS = $(SRCS:.c=.o)
#all:$(SUBDIR) $(APP)

#$(SUBDIR):


$(TARGET):$(OBJS)
	$(CC) $^ -o $@ $(FLAGS)

%.o:%.c
	$(CC) $(FLAGS) -c $^ -o $@

clean:
	rm -rf $(OBJS) $(TARGET)