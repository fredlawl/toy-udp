#ifndef __CLIENT_H
#define __CLIENT_H

#include <stdint.h>

struct client_ctx;
struct client_cfg {
  char *dest_ip;
  uint16_t dest_port;
};

int client_ctx_init(struct client_cfg *cfg, struct client_ctx **ctx);
int client_ctx_destroy(struct client_ctx *ctx);
void *client_serve(struct client_ctx *ctx);

#endif