#ifndef __SERVER_H
#define __SERVER_H

#include <stdint.h>

enum udp_server_state_t { CLOSED = 0, LISTENING, UNKNOWN };

struct udp_server_ctx;
struct udp_server_cfg {
  char *src_ip;
  uint16_t src_port;
};

int udp_server_ctx_init(struct udp_server_cfg *cfg,
                        struct udp_server_ctx **ctx);
int udp_server_ctx_destroy(struct udp_server_ctx *ctx);
enum udp_server_state_t udp_server_state(struct udp_server_ctx *ctx);
void *udp_server_serve(struct udp_server_ctx *ctx);
#endif