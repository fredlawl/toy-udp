CC := gcc
OBJ += main.o server.o client.o
WITH_LOCKS ?= y
OBJ-LOCK-y := pthread_lock.o
OBJ-LOCK-n := no_lock.o
OBJ := $(OBJ) $(OBJ-LOCK-$(WITH_LOCKS))
TARGET := main

.PHONY: all
all: main

.PHONY: clean
clean:
	rm -rf *.o $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ -lpthread $^

$(OBJ): %.o : %.c
	$(CC) -g -Wall -o $@ -c $<
