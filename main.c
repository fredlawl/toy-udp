#define PRINT_FMT "main: "

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "client/client.h"
#include "server/server.h"

#define DEST_IP "127.0.0.1"
#define DEST_PORT 7111

int main(int argc, char *argv[]) {
  int err = 0;
  pthread_t tid = 0;
  pthread_attr_t attr = {0};

  struct server_cfg server_cfg = {.src_ip = DEST_IP, .src_port = DEST_PORT};
  struct client_cfg client_cfg = {.dest_ip = DEST_IP, .dest_port = DEST_PORT};

  struct server_ctx *udp_server_ctx = NULL;
  struct client_ctx *udp_client_ctx = NULL;
  struct client_ctx *tcp_client_ctx = NULL;

  // demo: tcp_client_ctx has different data from udp
  tcp_client.ctx_init(&client_cfg, &tcp_client_ctx);
  tcp_client.serve(tcp_client_ctx);
  tcp_client.ctx_destroy(tcp_client_ctx);

  err = udp_server.ctx_init(&server_cfg, &udp_server_ctx);
  if (err) {
    perror(PRINT_FMT "udp_server_ctx_init()");
    goto exit;
  }

  err = pthread_attr_init(&attr);
  if (err) {
    perror(PRINT_FMT "pthread_attr_init()");
    goto exit;
  }

  err = pthread_create(&tid, &attr, udp_server.serve, udp_server_ctx);
  if (err) {
    perror(PRINT_FMT "pthread_create()");
    goto exit;
  }

  /*
    Wait for server to start. This is my poor-man's signaling.
    This could spin forever if the server state never changes.
    Normally, a state-machine would do the trick here, but I
    want to keep the client in the "main" thread.
  */
  while (udp_server.server_state(udp_server_ctx) != LISTENING) {
  }

  err = udp_client.ctx_init(&client_cfg, &udp_client_ctx);
  if (err) {
    perror(PRINT_FMT "udp_client_ctx_init()");
    goto exit;
  }

  udp_client.serve(udp_client_ctx);

  err = pthread_join(tid, NULL);
  if (err) {
    perror(PRINT_FMT "pthread_join()");
  }

exit:
  udp_client.ctx_destroy(udp_client_ctx);
  pthread_detach(tid);
  pthread_attr_destroy(&attr);
  udp_server.ctx_destroy(udp_server_ctx);
  return err;
}
