#define PRINT_FMT "srv: "

#include "server.h"
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "lock.h"

struct udp_server_ctx {
  struct udp_server_cfg *cfg;
  enum udp_server_state_t state;
  void *lock;
};

static const char MSG_ACK[5] = "ACK\n";
static const char MSG_NACK[6] = "NACK\n";

static enum udp_server_state_t __set_state(struct udp_server_ctx *ctx,
                                           enum udp_server_state_t state) {
  enum udp_server_state_t prev;
  util_lock(ctx->lock);
  prev = ctx->state;
  ctx->state = state;
  util_unlock(ctx->lock);
  return prev;
}

static int __bind(struct udp_server_cfg *cfg) {
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

static int __recv(int sk) {
  int err = 0;
  char buff[512] = {0};
  ssize_t bytes_read, bytes_sent = 0;
  struct sockaddr_in client;
  socklen_t client_len = sizeof(client);

  while (1) {
    memset(buff, 0, sizeof(buff));
    bytes_read = recvfrom(sk, buff, sizeof(buff) - 1, MSG_TRUNC,
                          (struct sockaddr *)&client, &client_len);

    /*
      MSG_TRUNC allows us to know if a message is too big. We can't perform
      another read here, but coupled with MSG_PEEK, we could dynamically
      allocate memory to shove the data into a bigger buffer, but that
      complicates this playground
    */
    if (bytes_read < 0 || bytes_read > sizeof(buff)) {
      bytes_sent = sendto(sk, MSG_NACK, sizeof(MSG_NACK), 0,
                          (struct sockaddr *)&client, client_len);
      if (bytes_sent <= 0) {
        fprintf(stderr, PRINT_FMT "unable to send data back\n");
      }
      continue;
    }

    __handle_msg(sk, buff, bytes_read <= sizeof(buff), &client, client_len);
  }

  return err;
}

int udp_server_ctx_init(struct udp_server_cfg *cfg,
                        struct udp_server_ctx **ctx) {
  int err = 0;
  *ctx = malloc(sizeof(**ctx));
  if (!*ctx) {
    errno = ENOMEM;
    return -1;
  }

  (*ctx)->cfg = cfg;
  (*ctx)->lock = util_lock_init();
  (*ctx)->state = CLOSED;
  return err;
}

int udp_server_ctx_destroy(struct udp_server_ctx *ctx) {
  free(ctx);
  return 0;
}

void *udp_server_serve(struct udp_server_ctx *ctx) {
  int sk;
  struct udp_server_cfg *cfg = ctx->cfg;

  printf(PRINT_FMT "attempting to create server\n");
  sk = __bind(cfg);
  if (!sk) {
    fprintf(stderr, PRINT_FMT "failed to establish the server\n");
    return NULL;
  }

  printf(PRINT_FMT "now listening on %s:%d\n", cfg->src_ip, cfg->src_port);

  __set_state(ctx, LISTENING);

  if (!__recv(sk)) {
    fprintf(stderr, PRINT_FMT "error handling messages\n");
  }

  __set_state(ctx, CLOSED);

  close(sk);
  util_lock_destroy(ctx->lock);
  return NULL;
}

enum udp_server_state_t udp_server_state(struct udp_server_ctx *ctx) {
  int ret = UNKNOWN;
  util_lock(ctx->lock);
  ret = ctx->state;
  util_unlock(ctx->lock);
  return ret;
}