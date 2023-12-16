#pragma once

#include "client.h"

int client_udp_ctx_init(struct client_cfg *cfg, struct client_ctx **ctx);
int client_udp_ctx_destroy(struct client_ctx *ctx);
