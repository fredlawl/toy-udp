#pragma once

#include "server.h"

int server_udp_ctx_init(struct server_cfg *cfg, struct server_ctx **ctx);
int server_udp_ctx_destroy(struct server_ctx *ctx);
