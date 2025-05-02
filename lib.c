#include "./include/mongoose.h"
#include "./include/taixin_sdk.h"
#include "./include/lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// Funciones auxiliares
static bool uri_is(struct mg_str uri, const char *s) {
    return mg_strcmp(uri, mg_str(s)) == 0;
}

int vcmp(const struct mg_str *s, const char *cstr) {
    return mg_strcmp(*s, mg_str(cstr));
}

int check_credentials(const char *u, const char *p) {
    for (int i = 0; USERS[i][0]; i++)
        if (strcmp(u, USERS[i][0]) == 0 && strcmp(p, USERS[i][1]) == 0)
            return 1;
    return 0;
}

const char *get_user(struct mg_http_message *hm) {
    struct mg_str *hdr = mg_http_get_header(hm, "Cookie");
    if (!hdr || hdr->len == 0) return NULL;

    struct mg_str val = mg_http_get_header_var(*hdr, mg_str("usuario"));
    if (val.len == 0) return NULL;

    static char buf[64];
    size_t n = val.len < sizeof(buf)-1 ? val.len : sizeof(buf)-1;
    memcpy(buf, val.buf, n);
    buf[n] = '\0';
    return buf;
}

void event_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev != MG_EV_HTTP_MSG) return;
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    char path[256];

    if (strncmp(hm->uri.buf, "/static/", 8) == 0) {
      char path[256];
      snprintf(path, sizeof(path), "www%.*s", (int)hm->uri.len, hm->uri.buf);
  
      FILE *fp = fopen(path, "r");
      if (!fp) {
          printf("Archivo estático no encontrado: %s\n", path);
          mg_http_reply(c, 404, "", "No encontrado");
          return;
      }
      fclose(fp);
  
      struct mg_http_serve_opts opts;
      memset(&opts, 0, sizeof(opts));
      mg_http_serve_file(c, c->fn_data, path, &opts);
      return;
  }
  
  

    // Home
    if (uri_is(hm->uri, "/") && hm->uri.len == 1) {
      struct mg_http_serve_opts opts;
      memset(&opts, 0, sizeof(opts));
      opts.mime_types = "text/html";
      mg_http_serve_file(c, c->fn_data, "www/templates/home.html", &opts);
      return;
    }
    

    // Login
    if (uri_is(hm->uri, "/login")) {
        if (vcmp(&hm->method, "POST") == 0) {
            char u[32], p[32];
            mg_http_get_var(&hm->body, "username", u, sizeof(u));
            mg_http_get_var(&hm->body, "password", p, sizeof(p));
            if (check_credentials(u, p)) {
                mg_printf(c,
                    "HTTP/1.1 302 Found\r\n"
                    "Set-Cookie: usuario=%s; Path=/\r\n"
                    "Location: %s\r\n\r\n",
                    u,
                    strcmp(u, "admin") == 0 ? "/admin" : "/comandos"
                );
                return;
            }
        }
        struct mg_http_serve_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.mime_types = "text/html";
        mg_http_serve_file(c, c->fn_data, TEMPLATES_DIR "/login.html", &opts);
        return;
    }

    // Admin
    if (uri_is(hm->uri, "/admin")) {
        const char *user = get_user(hm);
        if (!user || strcmp(user, "admin") != 0) {
            mg_http_reply(c, 302, "Location: /login\r\n", "");
            return;
        }
        struct mg_http_serve_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.mime_types = "text/html";
        mg_http_serve_file(c, c->fn_data, TEMPLATES_DIR "/admin.html", &opts);
        return;
    }

    // Comandos
    if (uri_is(hm->uri, "/commands")) {
        const char *user = get_user(hm);
        // if (!user) {
        //     mg_http_reply(c, 302, "Location: /login\r\n", "");
        //     return;
        // }
        struct mg_http_serve_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.mime_types = "text/html";
        mg_http_serve_file(c, c->fn_data, TEMPLATES_DIR "/commands.html", &opts);
        return;
    }

    if (strncmp(hm->uri.buf, "/run-command", 12) == 0 && vcmp(&hm->method, "GET") == 0) {
      char cmd[64] = {0};
      char arg[128] = {0};
      mg_http_get_var(&hm->query, "cmd", cmd, sizeof(cmd));
      mg_http_get_var(&hm->query, "arg", arg, sizeof(arg));
  
      char path[256];
      snprintf(path, sizeof(path), "./scripts/%s.sh %s", cmd, arg);
  
      printf(">> Ejecutando: %s\n", path);
  
      FILE *fp = popen(path, "r");
      if (!fp) {
          mg_http_reply(c, 500, "", "Error al ejecutar script\n");
          return;
      }
      char output[2048] = {0};
      fread(output, 1, sizeof(output) - 1, fp);
      pclose(fp);
  
      mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "%s", output);
      return;  // ⚠️ IMPORTANTE
  }
  if (uri_is(hm->uri, "/save-log") && vcmp(&hm->method, "POST") == 0) {
    // Obtener timestamp actual
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    char filename[128];
    strftime(filename, sizeof(filename), "log-%Y-%m-%d_%H-%M-%S.log", tm_info);

    // Escribir el archivo
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        mg_http_reply(c, 500, "", "No se pudo crear el archivo de log");
        return;
    }

    fwrite(hm->body.buf, 1, hm->body.len, fp);
    fclose(fp);

    mg_http_reply(c, 200, "", "Log guardado en %s", filename);
    return;
}
    // Config
    if (uri_is(hm->uri, "/config")) {
        if (vcmp(&hm->method, "POST") == 0) {
            char action[16];
            if (mg_http_get_var(&hm->body, "action", action, sizeof(action)) > 0) {
                if (strcmp(action, "reboot") == 0)       taixin_reboot();
                else if (strcmp(action, "default") == 0) taixin_restore_defaults();
            }
            char buf[16];
            int rssi=0, mcs=0, power=0, snr=0;
            if (mg_http_get_var(&hm->body, "rssi",  buf, sizeof(buf)) > 0) rssi  = atoi(buf);
            if (mg_http_get_var(&hm->body, "mcs",   buf, sizeof(buf)) > 0) mcs   = atoi(buf);
            if (mg_http_get_var(&hm->body, "power", buf, sizeof(buf)) > 0) power = atoi(buf);
            if (mg_http_get_var(&hm->body, "snr",   buf, sizeof(buf)) > 0) snr   = atoi(buf);
            taixin_set_config(rssi, mcs, power, snr);
        }
        struct mg_http_serve_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.mime_types = "text/html";
        mg_http_serve_file(c, c->fn_data, TEMPLATES_DIR "/config.html", &opts);
        return;
    }

    // Wi-Fi
    if (uri_is(hm->uri, "/wifi")) {
        if (vcmp(&hm->method, "POST") == 0) {
            char ssid[64], enc[32], pwd[64];
            mg_http_get_var(&hm->body, "ssid",       ssid, sizeof(ssid));
            mg_http_get_var(&hm->body, "encryption", enc,  sizeof(enc));
            mg_http_get_var(&hm->body, "password",   pwd,  sizeof(pwd));
            taixin_set_wifi(ssid, enc, pwd);
        }
        struct mg_http_serve_opts opts;
        memset(&opts, 0, sizeof(opts));
        opts.mime_types = "text/html";
        mg_http_serve_file(c, c->fn_data, TEMPLATES_DIR "/wifi.html", &opts);
        return;
    }

    if (uri_is(hm->uri, "/set-wifi") && vcmp(&hm->method, "POST") == 0) {
      char ssid[64] = {0};
      char password[64] = {0};
  
      // Extraer JSON manualmente (simple)
      sscanf(hm->body.buf, "{\"ssid\":\"%63[^\"]\",\"password\":\"%63[^\"]\"}", ssid, password);
  
      // Convertir la password a clave WPA-PSK hexadecimal (64 caracteres) con wpa_passphrase
      char psk[65] = {0};
      char cmd[256];
      snprintf(cmd, sizeof(cmd), "wpa_passphrase \"%s\" \"%s\" | grep psk= | tail -n1 | cut -d= -f2", ssid, password);
  
      FILE *fp = popen(cmd, "r");
      if (fp) {
          fgets(psk, sizeof(psk), fp);
          pclose(fp);
      }
  
      // Eliminar salto de línea si existe
      size_t len = strlen(psk);
      if (len > 0 && psk[len - 1] == '\n') psk[len - 1] = '\0';
  
      // Ejecutar comandos
      char set_ssid[128], set_key[128], set_psk[256];
      snprintf(set_ssid, sizeof(set_ssid), "hgpriv hg0 set ssid=%s", ssid);
      snprintf(set_key, sizeof(set_key), "hgpriv hg0 set key_mgmt=WPA-PSK");
      snprintf(set_psk, sizeof(set_psk), "hgpriv hg0 set wpa_psk=%s", psk);
  
      system(set_ssid);
      system(set_key);
      system(set_psk);
  
      mg_http_reply(c, 200, "", "WiFi actualizado: %s", ssid);
      return;
  }
  
    // 404
    mg_http_reply(c, 404, "Content-Type: text/plain\r\n", "404 Not Found\n");
}
