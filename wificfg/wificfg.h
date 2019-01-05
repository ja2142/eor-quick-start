#ifndef _WIFICFG_H
#define _WIFICFG_H

#include <stdbool.h>

void wificfg_start();

bool station_connected();
bool ap_active();

#endif