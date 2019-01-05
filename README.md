# Esp quick start

This is a sample project for [esp-open-rtos](https://github.com/SuperHouse/esp-open-rtos) based on esphttpd example. It contains:

 - http server,
 - OTA flashing via http,
 - mdns server,
 - wifi configuration page
 - automatic switching between AP and station, depending on chosen network accessibility.

## Prerequisites

This project runs with esp-open-rtos. 

## How to start

Before doing anything make file named `local_config` and in there point `SDK_PATH`  to your sdk root e.g.:

```
touch local_config && echo SDK_PATH = ~/Documents/src/esp-open-rtos>local_config
```

Then flash with:

```
make flash
```

After flashing connect to access point named `esp-open-rtos AP` (password: `esp-open-rtos`). If there isn't such network and you configured the connection previously it's likely that esp is already connected - just skip this step.

Go to [esp_test.local/](esp_test.local/). Here you can change station and AP settings. 

Any subsequent flash can be made OTA (providing esp is connected to the same network as development PC):
```
make webflash
```
Webflash uses esp mdns address by default, so if your OS doesn't support mdns you have to change `ESP_IP` in Makefile to an actual ip address (e.g. `ESP_IP ?= 192.168.0.2`)

## AP and station switching

At startup esp tries to connected with previously set WiFi. If this fails, it reboots and creates an access point while still trying to connect to the network. If at any point connection is successful, esp restarts, turning off the AP.
