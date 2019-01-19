/*
Some random cgi routines. Used in the LED example and the page that returns the entire
flash as a binary. Also handles the hit counter on the main page.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <espressif/esp_common.h>

#include "cgi.h"

int cgiWiFiStaCredentials(HttpdConnData *connData)
{
	char ssid[128];
	char pass[128];

	if (connData->conn == NULL)
	{
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	httpdFindArg(connData->getArgs, "ssid", ssid, sizeof(ssid));
	httpdFindArg(connData->getArgs, "pass", pass, sizeof(pass));
	if (ssid[0] == '\0')
	{
		printf("no ssid provided\n");
		httpdStartResponse(connData, 400); //bad request
		httpdHeader(connData, "Content-Type", "application/json");
		httpdEndHeaders(connData);
		httpdSend(connData, "{\"response\":\"ssid must be present\"}", -1);
		return HTTPD_CGI_DONE;
	}
	printf("changing station credentials - ssid: %s pw %s\n", ssid, pass);
	sysparam_set_int8("sta_configured", true);

	struct sdk_station_config sta_config;
	sdk_wifi_station_get_config(&sta_config);
	//memcpy because we dont want trailing \0 on last character - it would be outside ssid
	memcpy(sta_config.ssid, ssid, 32);
	memcpy(sta_config.password, pass, 64);
	sdk_wifi_station_set_config(&sta_config);
	sdk_wifi_station_connect();

	httpdStartResponse(connData, 200); //ok
	httpdHeader(connData, "Content-Type", "application/json");
	httpdEndHeaders(connData);
	httpdSend(connData, "{\"response\":\"ok\"}", -1);
	return HTTPD_CGI_DONE;
}

int cgiWiFiApCredentials(HttpdConnData *connData)
{
	char ssid[128];
	char pass[128];
	char hidden_str[128];
	bool hidden;

	if (connData->conn == NULL)
	{
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	httpdFindArg(connData->getArgs, "ssid", ssid, sizeof(ssid));
	httpdFindArg(connData->getArgs, "pass", pass, sizeof(pass));
	httpdFindArg(connData->getArgs, "hidden", hidden_str, sizeof(hidden_str));
	if (ssid[0] == '\0')
	{
		printf("no ssid provided\n");
		httpdStartResponse(connData, 400); //bad request
		httpdHeader(connData, "Content-Type", "application/json");
		httpdEndHeaders(connData);
		httpdSend(connData, "{\"response\":\"ssid must be present\"}", -1);
		return HTTPD_CGI_DONE;
	}

	hidden = hidden_str &&
		 (strcmp(hidden_str, "true") == 0);

	printf("changing AP credentials - ssid: %s, pw: %s, hidden: %i (%s)\n", ssid, pass, hidden, hidden_str);
	struct sdk_softap_config ap_config;
	sysparam_set_string("ap_ssid", &ssid);
	sysparam_set_string("ap_pass", &pass);
	sysparam_set_int8("ap_hidden", hidden);

	httpdStartResponse(connData, 200); //ok
	httpdHeader(connData, "Content-Type", "application/json");
	httpdEndHeaders(connData);
	httpdSend(connData, "{\"response\":\"ok\"}", -1);
	return HTTPD_CGI_DONE;
}
