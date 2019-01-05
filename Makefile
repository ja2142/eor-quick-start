PROGRAM = esphttpd
EXTRA_COMPONENTS = extras/dhcpserver extras/rboot-ota extras/libesphttpd extras/mdnsresponder

PROGRAM_SRC_DIR = . ./wificfg/
ESP_IP ?= esp_test.local

#Tag for OTA images. 0-27 characters. Change to eg your projects title.
LIBESPHTTPD_OTA_TAGNAME ?= generic

LIBESPHTTPD_MAX_CONNECTIONS ?= 8
LIBESPHTTPD_STACKSIZE ?= 2048

PROGRAM_CFLAGS += -DFREERTOS -DLIBESPHTTPD_OTA_TAGNAME="\"$(LIBESPHTTPD_OTA_TAGNAME)\"" -DFLASH_SIZE=$(FLASH_SIZE)
EXTRA_CFLAGS += -DMEMP_NUM_NETCONN=$(LIBESPHTTPD_MAX_CONNECTIONS)

include local_config
include $(SDK_PATH)/common.mk
