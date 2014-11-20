/*
Some random cgi routines.
*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * Jeroen Domburg <jeroen@spritesmods.com> wrote this file. As long as you retain 
 * this notice you can do whatever you want with this stuff. If we meet some day, 
 * and you think this stuff is worth it, you can buy me a beer in return. 
 * ----------------------------------------------------------------------------
 */


#include <string.h>
#include <osapi.h>
#include "user_interface.h"
#include "mem.h"
#include "httpd.h"
#include "cgi.h"
#include "io.h"
#include "espmissingincludes.h"

#include "config_store.h"


//Template code for the server config page.
void ICACHE_FLASH_ATTR tplServer(HttpdConnData *connData, char *token, void **arg) {
	char buff[64];
	if (token==NULL) return;

	if (os_strcmp(token, "httpdPort")==0) {
		os_sprintf(buff, "%d", config_get_server_port());
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}

//Cgi that updates the server port and schedules a reboot
int ICACHE_FLASH_ATTR cgiServer(HttpdConnData *connData) {
	int len;
	char buff[1024];
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	len=httpdFindArg(connData->postBuff, "port", buff, sizeof(buff));
	if (len!=0) {
		config_update_server_port(atoi(buff));
	}

	httpdRedirect(connData, "server_cfg.tpl");
	return HTTPD_CGI_DONE;
}

static long hitCounter=0;

//Template code for the counter on the index page.
void ICACHE_FLASH_ATTR tplCounter(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];

	if (token==NULL) return;

	if (os_strcmp(token, "counter")==0) {
		hitCounter++;
		os_sprintf(buff, "%ld", hitCounter);
	} else if (os_strcmp(token, "deviceType")==0) {
		os_sprintf(buff, "%d", config_get_device_type());
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}

//Template code for the getMaccAddress page.
void ICACHE_FLASH_ATTR tplGetMacAddress(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
    char password[PASSWD_LENGTH];
    char macaddr[6];
    int len=0;
    
	if (token==NULL) return;

    len=httpdFindArg(connData->getArgs, "passwd", buff, sizeof(buff));
	if (len!=0) {
		config_get_passwd(password);
		if (memcmp(password, buff, PASSWD_LENGTH)==0) {
			if (os_strcmp(token, "macAddress")==0) {
				wifi_get_macaddr(SOFTAP_IF, (uint8_t*) macaddr);
				os_sprintf(buff, "%2x:%2x:%2x:%2x:%2x:%2x", MAC2STR(macaddr));
			}
		} else {
			os_sprintf(buff, "Wrong Password !!");
		}
	} else {
		os_sprintf(buff, "Protected page !!");
	}
	
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}
