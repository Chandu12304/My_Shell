CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

SRCS = shell.c process.c memory_manager.c
OBJS = $(SRCS:.c=.o)
TARGET = myshell

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) 