#ifndef __CLIENT_H
#define __CLIENT_H

#include <stdint.h>

struct udp_client_ctx;
struct udp_client_cfg {
  char *dest_ip;
  uint16_t dest_port;
};

int udp_client_ctx_init(struct udp_client_cfg *cfg,
                        struct udp_client_ctx **ctx);
int udp_client_ctx_destroy(struct udp_client_ctx *ctx);
void *udp_client_serve(struct udp_client_ctx *ctx);

#endif