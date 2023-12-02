CC := gcc
INCLUDES := -I.
TARGET := main

OBJ += main.o server/udp.o client/udp.o

WITH_LOCKS ?= y
OBJ-LOCK-y := util/pthread_lock.o
OBJ-LOCK-n := util/no_lock.o
OBJ += $(OBJ-LOCK-$(WITH_LOCKS))

.PHONY: all
all: main

.PHONY: clean
clean:
	rm -rf $(OBJ) $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ -lpthread $^

$(OBJ): %.o : %.c
	$(CC) -g -Wall $(INCLUDES) -o $@ -c $<
