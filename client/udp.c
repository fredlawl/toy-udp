#define PRINT_FMT "udp client: "

#define _DEFAULT_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "udp.h"

struct udp_client_ctx {
  struct client_ops *ops;
  struct client_cfg *cfg;
  int sk;
};

static void __close(struct udp_client_ctx *ctx) { close(ctx->sk); }

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

static void *serve(struct client_ctx *ctx) {
  char buff[128] = {0};
  ssize_t bytes_read = 0;
  struct udp_client_ctx *udp_ctx = (void *) ctx;
  struct client_cfg *cfg = udp_ctx->cfg;
  int sk = udp_ctx->sk;

  printf(PRINT_FMT "attempting client connect\n");

  sk = __connect(cfg);
  if (!sk) {
    fprintf(stderr, PRINT_FMT "failed to connect to %s:%d\n", cfg->dest_ip,
            cfg->dest_port);
    goto exit;
  }

  memcpy(buff, "testing", sizeof("testing"));
  write(sk, buff, sizeof(buff));

  while ((bytes_read = read(sk, buff, sizeof(buff))) > 0) {
    printf(PRINT_FMT "recv: %s\n", buff);
    memset(buff, 0, sizeof(buff));
  }

exit:
  __close(udp_ctx);
  return NULL;
}

struct client_ops udp_client = {.serve = serve};

int client_udp_ctx_init(struct client_cfg *cfg, struct client_ctx **ctx) {
  struct udp_client_ctx *udp_ctx = malloc(sizeof(*udp_ctx));
  if (!ctx) {
    errno = ENOMEM;
    return -1;
  }

  udp_ctx->ops = &udp_client;
  udp_ctx->cfg = cfg;
  udp_ctx->sk = -1;
  *ctx = (struct client_ctx *) udp_ctx;
  return 0;
}

int client_udp_ctx_destroy(struct client_ctx *ctx) {
  struct udp_client_ctx *udp_ctx;
  if (!ctx) {
    return 0;
  }

  udp_ctx = (void *) ctx;
  __close(udp_ctx);
  free(udp_ctx);
  return 0;
}
