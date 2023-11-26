#define PRINT_FMT "client: "

#include "client.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct udp_client_ctx {
  struct udp_client_cfg *cfg;
};

static int __connect(struct udp_client_cfg *cfg) {
  int sk;
  int err = 0;
  struct in_addr in_addr;
  struct sockaddr_in sock_addr = {.sin_family = AF_INET,
                                  .sin_port = htons(cfg->dest_port)};

  err = inet_aton(cfg->dest_ip, &in_addr);
  if (err == 0) {
    perror(PRINT_FMT "inet_aton()");
    goto exit;
  }

  sock_addr.sin_addr = in_addr;
  sk = socket(AF_INET, SOCK_DGRAM, 0);
  if (sk < 0) {
    perror(PRINT_FMT "socket()");
  }

  err = connect(sk, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
  if (err) {
    perror(PRINT_FMT "connect()");
    goto exit;
  }

  return sk;

exit:
  close(sk);
  return 0;
}

int udp_client_ctx_init(struct udp_client_cfg *cfg,
                        struct udp_client_ctx **ctx) {
  *ctx = malloc(sizeof(**ctx));
  if (!*ctx) {
    errno = ENOMEM;
    return -1;
  }

  (*ctx)->cfg = cfg;
  return 0;
}

int udp_client_ctx_destroy(struct udp_client_ctx *ctx) {
  free(ctx);
  return 0;
}

void *udp_client_serve(struct udp_client_ctx *ctx) {
  int sk;
  char buff[128] = {0};
  ssize_t bytes_read = 0;
  struct udp_client_cfg *cfg = ctx->cfg;

  printf(PRINT_FMT "attempting client connect\n");

  sk = __connect(cfg);
  if (!sk) {
    fprintf(stderr, PRINT_FMT "failed to connect to %s:%d\n", cfg->dest_ip,
            cfg->dest_port);
    return NULL;
  }

  memcpy(buff, "testing", sizeof("testing"));
  write(sk, buff, sizeof(buff));

  while ((bytes_read = read(sk, buff, sizeof(buff))) > 0) {
    printf(PRINT_FMT "recv: %s\n", buff);
    memset(buff, 0, sizeof(buff));
  }

  close(sk);
  return NULL;
}