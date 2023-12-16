#include "server.h"

void server_serve(struct server_ctx *ctx) {
    ctx->ops->serve(ctx);
}

enum server_state_t server_state(struct server_ctx *ctx) {
    return ctx->ops->server_state(ctx);
}
