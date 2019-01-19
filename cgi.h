#ifndef CGI_H
#define CGI_H

#include <libesphttpd/httpd.h>

int cgiWiFiApCredentials(HttpdConnData *connData);
int cgiWiFiStaCredentials(HttpdConnData *connData);

#endif
