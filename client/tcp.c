#define PRINT_FMT "tcp client: "

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
  char sneaky_data[10];
};

static int __ctx_init(struct client_cfg *cfg, struct client_ctx **ctx) {
  *ctx = malloc(sizeof(**ctx));
  if (!*ctx) {
    errno = ENOMEM;
    return -1;
  }

  (*ctx)->cfg = cfg;
  memcpy((*ctx)->sneaky_data, "sneaky!", sizeof("sneaky!"));
  return 0;
}

static int __ctx_destroy(struct client_ctx *ctx) {
  free(ctx);
  return 0;
}

static void *__serve(struct client_ctx *ctx) {
  printf(PRINT_FMT "%s\n", ctx->sneaky_data);
  return NULL;
}

struct client_ops tcp_client = {
    .serve = &__serve, .ctx_init = &__ctx_init, .ctx_destroy = &__ctx_destroy};
