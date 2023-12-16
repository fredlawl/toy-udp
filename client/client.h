#pragma once

#include <stdint.h>

struct client_ctx {
  struct client_ops *ops;
};

struct client_cfg {
  char *dest_ip;
  uint16_t dest_port;
};

struct client_ops {
  void *(*serve)(struct client_ctx *ctx);
};

void client_serve(struct client_ctx *ctx);
