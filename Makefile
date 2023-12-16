PROJECT_DIR := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
CC := gcc
INCLUDES := -I$(PROJECT_DIR)
TARGET := main
CFLAGS := -g -Wall

OBJ += main.o server/server.o server/udp.o client/client.o client/udp.o client/tcp.o

CONFIG_WITH_LOCKS ?= n
OBJ-LOCK-y := util/pthread_lock.o
OBJ-LOCK-n := 
OBJ += $(OBJ-LOCK-$(CONFIG_WITH_LOCKS))

ifeq ($(CONFIG_WITH_LOCKS),y)
	VARS += -DCONFIG_WITH_LOCKS
endif

CFLAGS += $(VARS)

.PHONY: all
all: main

.PHONY: clean
clean:
	rm -rf $(OBJ) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ -lpthread $^

$(OBJ): %.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<
