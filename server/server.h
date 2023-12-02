#ifndef __SERVER_H
#define __SERVER_H

#include <stdint.h>

enum server_state_t { CLOSED = 0, LISTENING, UNKNOWN };

struct server_ctx;
struct server_cfg {
  char *src_ip;
  uint16_t src_port;
};

int server_ctx_init(struct server_cfg *cfg, struct server_ctx **ctx);
int server_ctx_destroy(struct server_ctx *ctx);
enum server_state_t udp_server_state(struct server_ctx *ctx);
void *server_serve(struct server_ctx *ctx);
#endif