/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

/*
   This is example code for the esphttpd library. It's a small-ish demo showing off 
   the server, including WiFi connection management capabilities, some IO and
   some pictures of cats.
   */

#include <string.h>
#include <stdio.h>

#include <espressif/esp_common.h>
#include <etstimer.h>
#include <sysparam.h>

#include <libesphttpd/httpd.h>
#include <libesphttpd/httpdespfs.h>
#include <libesphttpd/cgiwifi.h>
#include <libesphttpd/cgiflash.h>
#include <libesphttpd/auth.h>
#include <libesphttpd/espfs.h>
#include <libesphttpd/captdns.h>
#include <libesphttpd/webpages-espfs.h>
#include <dhcpserver.h>

#include <mdnsresponder.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <esp/uart.h>

#include "cgi.h"

#include "wificfg/wificfg.h"

#define AP_SSID "esp-test-AP"
#define AP_PSK "secret-password"

CgiUploadFlashDef uploadParams={
	.type=CGIFLASH_TYPE_FW,
	.fw1Pos=0x2000,
	.fw2Pos=((FLASH_SIZE*1024*1024)/2)+0x2000,
	.fwSize=((FLASH_SIZE*1024*1024)/2)-0x2000,
	.tagName=LIBESPHTTPD_OTA_TAGNAME
};


void mdnsStarterTask(void* arg){
	switch(sdk_wifi_get_opmode()) {//apropriate waiting scheme
		case STATIONAP_MODE:
			while(1){//no idea what this should wait for
				vTaskDelay(pdMS_TO_TICKS(5000)); 
				break;  
			}
			break;
		case STATION_MODE:
			while(1){//wait for connection
				if (station_connected()) {break;}
				vTaskDelay(pdMS_TO_TICKS(1000));    
			}
			break;
	}
	mdns_init();
	mdns_add_facility("esp_test", "device", NULL, mdns_TCP, 80, 600);
	printf("mdns started, bye\n");
	vTaskDelete(NULL);
}

/*
   This is the main url->function dispatching data struct.
   In short, it's a struct with various URLs plus their handlers. The handlers can
   be 'standard' CGI functions you wrote, or 'special' CGIs requiring an argument.
   They can also be auth-functions. An asterisk will match any url starting with
   everything before the asterisks; "*" matches everything. The list will be
   handled top-down, so make sure to put more specific rules above the more
   general ones. Authorization things (like authBasic) act as a 'barrier' and
   should be placed above the URLs they protect.
   */
HttpdBuiltInUrl builtInUrls[]={
	{"*", cgiRedirectApClientToHostname, "esp8266.nonet"},
	{"/", cgiEspFsHook, "/index.html"},
	{"/wificfg/", cgiEspFsHook, "/wificfg/index.html"},
	{"/led.tpl", cgiEspFsTemplate, tplLed},
	{"/index.tpl", cgiEspFsTemplate, tplCounter},
	{"/led.cgi", cgiLed, NULL},
#ifndef ESP32
	{"/flash/", cgiRedirect, "/flash/index.html"},
	{"/flash/next", cgiGetFirmwareNext, &uploadParams},
	{"/flash/upload", cgiUploadFirmware, &uploadParams},
	{"/flash/reboot", cgiRebootFirmware, NULL},
#endif

	{"/wificfg/wifiscan.cgi", cgiWiFiScan, NULL},
	{"/wificfg/changestation.cgi", cgiWiFiStaCredentials, NULL},
	{"/wificfg/changeap.cgi", cgiWiFiApCredentials, NULL},

	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL}
};

void wifiInit() {
	struct ip_info ap_ip;
	char* ssid;
	char* pass;
	bool hidden = false, configured = false;
	int8_t opmode;

	if(sysparam_get_int8("opmode", &opmode)!=SYSPARAM_OK){
		opmode = SOFTAP_MODE;
	}
	printf("opmode: %i\n", opmode);
	switch(opmode) {
		case NULL_MODE:
		case SOFTAP_MODE:
		case STATIONAP_MODE:

			sdk_wifi_set_opmode(STATIONAP_MODE);

			IP4_ADDR(&ap_ip.ip, 172, 16, 0, 1);
			IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
			IP4_ADDR(&ap_ip.netmask, 255, 255, 0, 0);
			sdk_wifi_set_ip_info(1, &ap_ip);

			struct sdk_softap_config ap_config = { 
				.ssid_hidden = 0,
				.channel = 3,
				.authmode = AUTH_WPA2_PSK,
				.max_connection = 3, 
				.beacon_interval = 100, 
			};
			if(sysparam_get_string("ap_ssid", &ssid)==SYSPARAM_OK 
					&& sysparam_get_string("ap_pass", &pass)==SYSPARAM_OK){
				if(sysparam_get_int8("ap_hidden", &hidden)==SYSPARAM_OK){
					ap_config.ssid_hidden = hidden;
				}
				memcpy((char*)ap_config.ssid, ssid, 32);
				strncpy((char*)ap_config.password, pass, 64);
				ap_config.ssid_len = strlen(ssid);
				free(ssid); 
				free(pass);
			}else{ //previously stored credentials not found, using default
				memcpy((char*)ap_config.ssid, AP_SSID, 32);printf("AP: %s, pass: %s\n", AP_SSID, AP_PSK);
				strncpy((char*)ap_config.password, AP_PSK, 64);
				ap_config.ssid_len = strlen(AP_SSID);
			}
			sdk_wifi_softap_set_config(&ap_config);

			ip_addr_t first_client_ip;
			IP4_ADDR(&first_client_ip, 172, 16, 0, 2);
			dhcpserver_start(&first_client_ip, 4);
			dhcpserver_set_dns(&ap_ip.ip);
			dhcpserver_set_router(&ap_ip.ip);
			break;
		case STATION_MODE:
			sdk_wifi_set_opmode(STATION_MODE);
			break;
	}
}

//Main routine. Initialize stdout, the I/O, filesystem and the webserver and we're done.
void user_init(void) {sdk_wifi_set_opmode(NULL_MODE);
	uart_set_baud(0, 115200);

	wifiInit();
	wificfg_start();
	captdnsInit();

	espFsInit((void*)(_binary_build_web_espfs_bin_start));
	httpdInit(builtInUrls, 80);

	xTaskCreate(mdnsStarterTask, "mdnsStarter", 256, NULL, 3, NULL);

	printf("\nReady\n");
}
