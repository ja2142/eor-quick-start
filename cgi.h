#ifndef CGI_H
#define CGI_H

#include <libesphttpd/httpd.h>

int cgiLed(HttpdConnData *connData);
int cgiWiFiApCredentials(HttpdConnData *connData);
int cgiWiFiStaCredentials(HttpdConnData *connData);
int tplLed(HttpdConnData *connData, char *token, void **arg);
int tplCounter(HttpdConnData *connData, char *token, void **arg);

#endif
