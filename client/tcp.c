#define PRINT_FMT "tcp client: "

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client.h"
#include "tcp.h"

struct tcp_client_ctx {
  struct client_ops* ops;
  struct client_cfg *cfg;
  char sneaky_data[10];
};

static void *serve(struct client_ctx *ctx) {
  struct tcp_client_ctx *tcp_ctx = (void *)ctx;
  printf(PRINT_FMT "%s\n", tcp_ctx->sneaky_data);
  return NULL;
}

struct client_ops tcp_client = {
    .serve = serve};

int client_tcp_ctx_init(struct client_cfg *cfg, struct client_ctx **ctx) {
  struct tcp_client_ctx *tcp_ctx = malloc(sizeof(*tcp_ctx));
  if (!tcp_ctx) {
    errno = ENOMEM;
    return -1;
  }

  tcp_ctx->ops = &tcp_client;
  tcp_ctx->cfg = cfg;
  memcpy(tcp_ctx->sneaky_data, "sneaky!", sizeof("sneaky!"));
  *ctx = (struct client_ctx *)tcp_ctx;
  return 0;
}

int client_tcp_ctx_destroy(struct client_ctx *ctx) {
  struct tcp_client_ctx *client_ctx;
  if (!ctx) {
    return 0;
  }

  client_ctx = (void *)ctx;
  free(client_ctx);
  return 0;
}
