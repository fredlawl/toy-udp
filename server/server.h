#pragma once

#include <stdint.h>

enum server_state_t { CLOSED = 0, LISTENING, UNKNOWN };

struct server_ctx {
  struct server_ops *ops;
};

struct server_cfg {
  char *src_ip;
  uint16_t src_port;
};

struct server_ops {
  void *(*serve)(struct server_ctx *ctx);
  enum server_state_t (*server_state)(struct server_ctx *ctx);
};

void server_serve(struct server_ctx *ctx);
enum server_state_t server_state(struct server_ctx *ctx);
