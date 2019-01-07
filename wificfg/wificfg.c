#define WIFI_TIMEOUT_S 10

#include "wificfg.h"

#include <stdio.h>
#include <string.h>

#include <espressif/esp_common.h>

#include <FreeRTOS.h>

#include <dhcpserver.h>

static void start_AP(){
    printf("AP_start\n");
    sysparam_set_int8("opmode", STATIONAP_MODE);
    sdk_system_restart();
}

static void stop_AP(){
    printf("AP_stop\n");
    sysparam_set_int8("opmode", STATION_MODE);
    sdk_system_restart();
}

bool station_connected(){
    return sdk_wifi_station_get_connect_status() == STATION_GOT_IP;
}

bool ap_active(){
    return (sdk_wifi_get_opmode() == SOFTAP_MODE)|| (sdk_wifi_get_opmode() == STATIONAP_MODE);
}

static void wifiManagerTask(void *arg) {
    bool connected = false;
    int time_elapsed = 0;
    while(1){
        if(connected){ //monitoring for disconnection
            while(1){
                connected = station_connected();
                if (!connected) {break;}
                vTaskDelay(pdMS_TO_TICKS(5000));    
            }
            printf("disconnected\n");
        }else{ //waiting for connection
            while(1){
                connected = station_connected();
                if (connected) {break;}
                vTaskDelay(pdMS_TO_TICKS(1000));
                time_elapsed += 1;
                if(time_elapsed++>WIFI_TIMEOUT_S && !ap_active()){
                    start_AP();
                }
            }
            printf("connected\n");
            if(ap_active()){
                stop_AP();
            }
        }
        
    }
}

void wificfg_start(){
    xTaskCreate(wifiManagerTask, "wifiManagerTask", 256, NULL, 2, NULL);
}