#ifndef CGI_H
#define CGI_H

#include "httpd.h"

void tplServer(HttpdConnData *connData, char *token, void **arg);
void tplPassword(HttpdConnData *connData, char *token, void **arg);

int cgiServer(HttpdConnData *connData);
int cgiGetMacAddress(HttpdConnData *connData);
int cgiGetCurrentVoltages(HttpdConnData *connData);
int cgiGetDeviceType(HttpdConnData *connData);
int cgiPassword(HttpdConnData *connData);

#endif
