// taixin_sdk.c  (STUB)
#include <stdio.h>
#include "./include/taixin_sdk.h"

void taixin_init(void) {
  printf("[STUB SDK] init()\n");
}
void taixin_reboot(void) {
  printf("[STUB SDK] reboot()\n");
}
void taixin_restore_defaults(void) {
  printf("[STUB SDK] restore_defaults()\n");
}
void taixin_set_config(int rssi, int mcs, int power, int snr) {
  printf("[STUB SDK] set_config: RSSI=%d MCS=%d PWR=%d SNR=%d\n",
         rssi, mcs, power, snr);
}
void taixin_set_wifi(const char *ssid, const char *enc, const char *pwd) {
  printf("[STUB SDK] set_wifi: SSID=%s ENC=%s PWD=%s\n",
         ssid, enc, pwd);
}
