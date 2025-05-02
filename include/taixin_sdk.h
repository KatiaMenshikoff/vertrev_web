// taixin_sdk.h
#ifndef TAIXIN_SDK_H
#define TAIXIN_SDK_H

void taixin_init(void);
void taixin_reboot(void);
void taixin_restore_defaults(void);
void taixin_set_config(int rssi, int mcs, int power, int snr);
void taixin_set_wifi(const char *ssid, const char *enc, const char *pwd);

#endif
