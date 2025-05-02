#ifndef LIB_H
#define LIB_H
#include "taixin_sdk.h"  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LISTEN_ADDR   "http://localhost:8000"
#define WWW_DIR        "www"
#define TEMPLATES_DIR  WWW_DIR "/templates"
#define STATIC_URI   WWW_DIR    "/static/"
#define COMPONENTS_URI  WWW_DIR "/components/"

static const char *USERS[][2] = {
  {"admin", "admin"},
  {"user",  "user"},
  {NULL,    NULL}
};

void event_handler(struct mg_connection *c, int ev, void *ev_data);
#endif