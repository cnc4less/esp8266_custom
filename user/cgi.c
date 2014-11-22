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
#include "uart.h"

#include "config_store.h"

static volatile uint8_t rx_buffer_done = 0;
static volatile uint8_t rx_buffer_index = 0;
char rx_buffer[MAX_STM32_RESPONSE_LENGTH];
static uart_t* uart0;

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

//Template code for the type of the device.
int ICACHE_FLASH_ATTR cgiGetDeviceType(HttpdConnData *connData)
{
	char buff[128];
    char password[PASSWD_LENGTH];

    int len=0;
    
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/json");
	httpdEndHeaders(connData);
	
    len=httpdFindArg(connData->getArgs, "passwd", password, PASSWD_LENGTH);
	
	if (len!=0) {
		if (config_get_password() == atoi(password)){
			os_sprintf(buff, "{\"device_type\" : \"0x%x\"}", config_get_device_type());
			os_sprintf(buff, "{\n \"result\": { \n\"noPassword\": \"0\",\n\"wrongPassword\": \"0\",\n\"device_type\" : \"0x%2x\" }\n}", config_get_device_type());
		} else {
			len=os_sprintf(buff, "{\n \"result\": { \n\"noPassword\": \"0\",\n\"wrongPassword\": \"1\",\n\"mac_address\":\"NULL\" }\n}");
		}
	} else {
		len=os_sprintf(buff, "{\n \"result\": { \n\"noPassword\": \"1\",\n\"wrongPassword\": \"0\"\n,\"mac_address\":\"NULL\" }\n}");
	}
	
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
	return HTTPD_CGI_DONE;
}


//Template code for the getMaccAddress page.
int ICACHE_FLASH_ATTR cgiGetMacAddress(HttpdConnData *connData) 
{
	char buff[128];
    char password[PASSWD_LENGTH];
    char macaddr[6];
    int len=0;
    
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/json");
	httpdEndHeaders(connData);
	
	len=httpdFindArg(connData->getArgs, "passwd", password, PASSWD_LENGTH);
	
	if (len!=0) {
		if (config_get_password() == atoi(password)){
			wifi_get_macaddr(SOFTAP_IF, (uint8_t*) macaddr);
			os_sprintf(buff, "{\n \"result\": { \n\"noPassword\": \"0\",\n\"wrongPassword\": \"0\",\n\"mac_address\" : \"%2x:%2x:%2x:%2x:%2x:%2x\" }\n}", MAC2STR(macaddr));
		} else {
			len=os_sprintf(buff, "{\n \"result\": { \n\"noPassword\": \"0\",\n\"wrongPassword\": \"1\",\n\"mac_address\":\"NULL\" }\n}");
		}
	} else {
		len=os_sprintf(buff, "{\n \"result\": { \n\"noPassword\": \"1\",\n\"wrongPassword\": \"0\"\n,\"mac_address\":\"NULL\" }\n}");
	}
	
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
	return HTTPD_CGI_DONE;
}

void ICACHE_FLASH_ATTR rx_handler_cb( char c)
{
	if (c == '#'){
		rx_buffer[rx_buffer_index]='\0';
		rx_buffer_index = 0;
		rx_buffer_done = 1;
	} else {
		rx_buffer[rx_buffer_index++] = c;
	}
}

int ICACHE_FLASH_ATTR cgiGetCurrentVoltages(HttpdConnData *connData) 
{
	char buff[128];
    char password[PASSWD_LENGTH];

    int len=0;

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	
    httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "text/json");
	httpdEndHeaders(connData);
	
    len=httpdFindArg(connData->getArgs, "passwd", password, PASSWD_LENGTH);
	
	if (len!=0) {
		if (config_get_password() == atoi(password)){
			uart0_init(BIT_RATE_115200, &rx_handler_cb);
			uart0_transmit(uart0, "getVoltages#", 12);
			uart0_wait_for_transmit(uart0);
			if (rx_buffer_done == 0)
			{ 
				os_delay_us(15000);
			}
			rx_buffer_done = 0;
			//uart0_uninit(uart0);
			len=os_sprintf(buff, "{\n \"result\": { \n\"noPassword\": \"0\",\n\"wrongPassword\": \"0\",\n\"adcValues\": [%s] }\n}", rx_buffer);
		} else {
			len=os_sprintf(buff, "{\n \"result\": { \n\"noPassword\": \"0\",\n\"wrongPassword\": \"1\",\n\"adcValues\":\"NULL\" }\n}");
		}
	} else {
			len=os_sprintf(buff, "{\n \"result\": { \n\"noPassword\": \"1\",\n\"wrongPassword\": \"0\"\n,\"adcValues\":\"NULL\" }\n}");
	}
	
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
	return HTTPD_CGI_DONE;
}

//Template code for the server config page.
void ICACHE_FLASH_ATTR tplPassword(HttpdConnData *connData, char *token, void **arg) {
	char buff[64];
	if (token==NULL) return;

	if (os_strcmp(token, "accessPassword")==0) {
		os_sprintf(buff, "%d", config_get_password());
	}
	espconn_sent(connData->conn, (uint8 *)buff, os_strlen(buff));
}

//Cgi that updates the server port and schedules a reboot
int ICACHE_FLASH_ATTR cgiPassword(HttpdConnData *connData) {
	int len;
	char buff[1024];
	
	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}

	len=httpdFindArg(connData->postBuff, "password", buff, sizeof(buff));
	if (len!=0) {
		config_update_password(atoi(buff));
	}

	httpdRedirect(connData, "password_cfg.tpl");
	return HTTPD_CGI_DONE;
}
