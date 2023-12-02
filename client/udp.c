#define PRINT_FMT "client: "

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client.h"

struct client_ctx {
  struct client_cfg *cfg;
  int sk;
};

static void __close(struct client_ctx *ctx) { close(ctx->sk); }

static int __connect(struct client_cfg *cfg) {
  int sk = 0;
  int err = 0;
  struct in_addr in_addr;
  struct sockaddr_in sock_addr = {.sin_family = AF_INET,
                                  .sin_port = htons(cfg->dest_port)};

  err = inet_aton(cfg->dest_ip, &in_addr);
  if (err == 0) {
    perror(PRINT_FMT "inet_aton()");
    goto exit;
  }

  sock_addr.sin_addr = in_addr;
  sk = socket(AF_INET, SOCK_DGRAM, 0);
  if (sk < 0) {
    perror(PRINT_FMT "socket()");
  }

  err = connect(sk, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
  if (err) {
    perror(PRINT_FMT "connect()");
    goto exit;
  }

  return sk;

exit:
  close(sk);
  return 0;
}

int client_ctx_init(struct client_cfg *cfg, struct client_ctx **ctx) {
  *ctx = malloc(sizeof(**ctx));
  if (!*ctx) {
    errno = ENOMEM;
    return -1;
  }

  (*ctx)->cfg = cfg;
  (*ctx)->sk = -1;
  return 0;
}

int client_ctx_destroy(struct client_ctx *ctx) {
  __close(ctx);
  free(ctx);
  return 0;
}

void *client_serve(struct client_ctx *ctx) {
  char buff[128] = {0};
  ssize_t bytes_read = 0;
  struct client_cfg *cfg = ctx->cfg;

  printf(PRINT_FMT "attempting client connect\n");

  ctx->sk = __connect(cfg);
  if (!ctx->sk) {
    fprintf(stderr, PRINT_FMT "failed to connect to %s:%d\n", cfg->dest_ip,
            cfg->dest_port);
    goto exit;
  }

  memcpy(buff, "testing", sizeof("testing"));
  write(ctx->sk, buff, sizeof(buff));

  while ((bytes_read = read(ctx->sk, buff, sizeof(buff))) > 0) {
    printf(PRINT_FMT "recv: %s\n", buff);
    memset(buff, 0, sizeof(buff));
  }

exit:
  __close(ctx);
  return NULL;
}