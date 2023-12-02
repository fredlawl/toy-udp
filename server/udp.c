#define PRINT_FMT "srv: "

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "util/lock.h"

struct server_ctx {
  struct server_cfg *cfg;
  enum server_state_t state;
  void *lock;
  int sk;
};

static const char MSG_ACK[5] = "ACK\n";
static const char MSG_NACK[6] = "NACK\n";

static enum server_state_t __set_state(struct server_ctx *ctx,
                                       enum server_state_t state) {
  enum server_state_t prev;
  util_lock(ctx->lock);
  prev = ctx->state;
  ctx->state = state;
  util_unlock(ctx->lock);
  return prev;
}

static int __bind(struct server_cfg *cfg) {
  int sk;
  int err = 0;
  struct in_addr in_addr;
  struct sockaddr_in sock_addr = {.sin_family = AF_INET,
                                  .sin_port = htons(cfg->src_port)};

  err = inet_aton(cfg->src_ip, &in_addr);
  if (err == 0) {
    perror(PRINT_FMT "inet_aton()");
    goto exit;
  }

  sock_addr.sin_addr = in_addr;
  sk = socket(AF_INET, SOCK_DGRAM, 0);
  if (sk < 0) {
    perror(PRINT_FMT "socket()");
  }

  err = bind(sk, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
  if (err) {
    perror(PRINT_FMT "bind()");
    goto exit;
  }

  return sk;

exit:
  close(sk);
  return 0;
}

static void __handle_msg(int sk, char *buff, size_t size,
                         struct sockaddr_in *client, socklen_t client_len) {
  ssize_t bytes_sent;

  printf(PRINT_FMT "recv: %s\n", buff);
  bytes_sent = sendto(sk, MSG_ACK, sizeof(MSG_ACK), 0,
                      (struct sockaddr *)client, client_len);
  if (bytes_sent <= 0) {
    fprintf(stderr, PRINT_FMT "unable to send data back\n");
  }
}

static void __send_nack(int sk, struct sockaddr_in *client,
                        socklen_t client_len) {
  ssize_t bytes_sent;
  bytes_sent = sendto(sk, MSG_NACK, sizeof(MSG_NACK), 0,
                      (struct sockaddr *)&client, client_len);
  if (bytes_sent <= 0) {
    fprintf(stderr, PRINT_FMT "unable to send data back\n");
  }
}

/*
  MSG_TRUNC | MSG_PEEK flags for recvfrom() allows us to know what the full
  bytes sent are. We can use that to allocate a bigger buffer dynamically, and
  then perform another recvfrom(). Test works good, but lots of extra
  syscalls are needed.
*/
static int __recv_big_buffer(int sk) {
  int err = 0;
  char buff[512] = {0};
  ssize_t bytes_read = 0;
  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);

  while (1) {
    char *big_buff = NULL;
    memset(buff, 0, sizeof(buff));
    bytes_read = recvfrom(sk, buff, sizeof(buff) - 1, MSG_TRUNC | MSG_PEEK,
                          (struct sockaddr *)&client, &client_len);
    if (bytes_read < 0) {
      __send_nack(sk, &client, client_len);
    }

    big_buff = calloc(bytes_read, sizeof(*big_buff));
    if (!big_buff) {
      perror(PRINT_FMT "calloc()");
      __send_nack(sk, &client, client_len);
      continue;
    }

    bytes_read = recvfrom(sk, big_buff, bytes_read - 1, MSG_TRUNC,
                          (struct sockaddr *)&client, &client_len);
    if (bytes_read < 0) {
      __send_nack(sk, &client, client_len);
      free(big_buff);
      continue;
    }

    __handle_msg(sk, big_buff, bytes_read <= sizeof(buff), &client, client_len);
    free(big_buff);
  }

  return err;
}

static int __recv(int sk) {
  int err = 0;
  char buff[512] = {0};
  ssize_t bytes_read;
  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);

  while (1) {
    memset(buff, 0, sizeof(buff));
    bytes_read = recvfrom(sk, buff, sizeof(buff) - 1, MSG_TRUNC,
                          (struct sockaddr *)&client, &client_len);

    if (bytes_read < 0 || bytes_read > sizeof(buff)) {
      __send_nack(sk, &client, client_len);
      continue;
    }

    __handle_msg(sk, buff, bytes_read <= sizeof(buff), &client, client_len);
  }

  return err;
}

int server_ctx_init(struct server_cfg *cfg, struct server_ctx **ctx) {
  int err = 0;
  *ctx = malloc(sizeof(**ctx));
  if (!*ctx) {
    errno = ENOMEM;
    return -1;
  }

  (*ctx)->cfg = cfg;
  (*ctx)->lock = util_lock_init();
  (*ctx)->state = CLOSED;
  (*ctx)->sk = -1;
  return err;
}

static void __close(struct server_ctx *ctx) {
  __set_state(ctx, CLOSED);
  close(ctx->sk);
}

int server_ctx_destroy(struct server_ctx *ctx) {
  __close(ctx);
  util_lock_destroy(ctx->lock);
  free(ctx);
  return 0;
}

void *server_serve(struct server_ctx *ctx) {
  struct server_cfg *cfg = ctx->cfg;

  printf(PRINT_FMT "attempting to create server\n");
  ctx->sk = __bind(cfg);
  if (!ctx->sk) {
    fprintf(stderr, PRINT_FMT "failed to establish the server\n");
    goto exit;
  }

  printf(PRINT_FMT "now listening on %s:%d\n", cfg->src_ip, cfg->src_port);

  __set_state(ctx, LISTENING);

  if (!__recv(ctx->sk)) {
    fprintf(stderr, PRINT_FMT "error handling messages\n");
  }

exit:
  __close(ctx);
  return NULL;
}

enum server_state_t udp_server_state(struct server_ctx *ctx) {
  int ret = UNKNOWN;
  util_lock(ctx->lock);
  ret = ctx->state;
  util_unlock(ctx->lock);
  return ret;
}