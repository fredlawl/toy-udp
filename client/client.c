#include "client.h"

void client_serve(struct client_ctx *ctx) {
    ctx->ops->serve(ctx);
}