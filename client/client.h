#pragma once

#include <stdint.h>

struct client_ctx;
struct client_cfg {
  char *dest_ip;
  uint16_t dest_port;
};

struct client_ops {
  int (*ctx_init)(struct client_cfg *cfg, struct client_ctx **ctx);
  int (*ctx_destroy)(struct client_ctx *ctx);
  void *(*serve)(struct client_ctx *ctx);
};

extern struct client_ops udp_client;
extern struct client_ops tcp_client;
