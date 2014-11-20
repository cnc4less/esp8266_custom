#ifndef CGI_H
#define CGI_H

#include "httpd.h"

void tplCounter(HttpdConnData *connData, char *token, void **arg);
void tplServer(HttpdConnData *connData, char *token, void **arg);
int ICACHE_FLASH_ATTR cgiServer(HttpdConnData *connData);
#endif
