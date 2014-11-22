

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */

#include "user_interface.h"
#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "httpd.h"
#include "io.h"
#include "httpdespfs.h"
#include "cgi.h"
#include "cgiwifi.h"
#include "stdout.h"

#include "config_store.h"

HttpdBuiltInUrl builtInUrls[]={
	{"/", cgiRedirect, "/index.tpl"},
	{"/index.tpl", cgiEspFsTemplate, tplServer},
	{"/server_cfg.tpl", cgiEspFsTemplate, tplServer},
	{"/server_cfg.cgi", cgiServer, tplServer},
	{"/password_cfg.tpl", cgiEspFsTemplate, tplPassword},
	{"/password_cfg.cgi", cgiPassword, tplPassword},
	{"/getMacAddress", cgiGetMacAddress, NULL},
	{"/getDeviceType", cgiGetDeviceType, NULL},
	{"/getCurrentVoltages", cgiGetCurrentVoltages, NULL},
	
	//Routines to make the /wifi URL and everything beneath it work.
	{"/wifi", cgiRedirect, "/wifi/wifi.tpl"},
	{"/wifi/", cgiRedirect, "/wifi/wifi.tpl"},
	{"/wifi/wifiscan.cgi", cgiWiFiScan, NULL},
	{"/wifi/wifi.tpl", cgiEspFsTemplate, tplWlan},
	{"/wifi/connect.cgi", cgiWiFiConnect},

	{"*", cgiEspFsHook, NULL}, //Catch-all cgi function for the filesystem
	{NULL, NULL, NULL}
};

LOCAL struct esp_platform_saved_param esp_param;

void platform_init(void) {
	struct softap_config config;
	char custom_ssid[9];
    char macaddr[6];

	wifi_softap_get_config(&config);

	os_memset(config.ssid, 0, sizeof(config.ssid));

        wifi_get_macaddr(SOFTAP_IF, (uint8_t*) macaddr);

        os_sprintf(custom_ssid, "GT-%2x%2x%2x%2x%2x%2x", MAC2STR(macaddr));
        os_memcpy(config.ssid, custom_ssid, os_strlen(custom_ssid));

        wifi_softap_set_config(&config);

	wifi_set_opmode(STATIONAP_MODE);

}

void user_init(void) 
{
	ioInit();
	httpdInit(builtInUrls, config_get_server_port());
	platform_init();
	os_printf("\nReady\n");
}
