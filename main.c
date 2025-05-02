#include "./include/mongoose.h"
#include "./include/lib.h"
#include "./include/taixin_sdk.h"   // tu stub o SDK real
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(void) {
  struct mg_mgr mgr;
  mg_mgr_init(&mgr);

  // Inicializa tu SDK (o stub)
  taixin_init();

  // Arranca el listener
  mg_http_listen(&mgr, LISTEN_ADDR, event_handler, &mgr);
  printf("Servidor corriendo en %s\n", LISTEN_ADDR);

  // Loop de eventos
  for (;;) mg_mgr_poll(&mgr, 1000);

  mg_mgr_free(&mgr);
  return 0;
}
