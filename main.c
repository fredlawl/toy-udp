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
  pthread_t tid;

  int err = 0;
  pthread_attr_t attr = {0};

  struct server_ctx *server_ctx = NULL;
  struct server_cfg server_cfg = {.src_ip = DEST_IP, .src_port = DEST_PORT};

  struct client_ctx *client_ctx = NULL;
  struct client_cfg client_cfg = {.dest_ip = DEST_IP, .dest_port = DEST_PORT};

  err = server_ctx_init(&server_cfg, &server_ctx);
  if (err) {
    perror(PRINT_FMT "udp_server_ctx_init()");
    goto exit;
  }

  err = pthread_attr_init(&attr);
  if (err) {
    perror(PRINT_FMT "pthread_attr_init()");
    goto exit;
  }

  err = pthread_create(&tid, &attr, server_serve, server_ctx);
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
  while (udp_server_state(server_ctx) != LISTENING) {
  }

  err = client_ctx_init(&client_cfg, &client_ctx);
  if (err) {
    perror(PRINT_FMT "udp_client_ctx_init()");
    goto exit;
  }

  client_serve(client_ctx);

  err = pthread_join(tid, NULL);
  if (err) {
    perror(PRINT_FMT "pthread_join()");
  }

exit:
  client_ctx_destroy(client_ctx);
  pthread_detach(tid);
  pthread_attr_destroy(&attr);
  server_ctx_destroy(server_ctx);
  return err;
}
