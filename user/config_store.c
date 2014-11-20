/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#include "user_config.h"
#include "config_store.h"
#include "user_interface.h"
#include "spi_flash.h"
#include "ets_sys.h"
#include "espmissingincludes.h"

#define CONFIG_START_SECTOR 0x3C
#define CONFIG_SECTOR (CONFIG_START_SECTOR + 0)
#define CONFIG_ADDR (SPI_FLASH_SEC_SIZE * CONFIG_SECTOR)

#define CONFIG_WIFI_SECTOR 0x7E

static config_t s_config;
static int s_config_loaded = 0;

void ICACHE_FLASH_ATTR config_read(config_t* config)
{
    spi_flash_read(CONFIG_ADDR, (uint32*) config, sizeof(config_t));
}

void ICACHE_FLASH_ATTR config_write(config_t* config)
{
    ETS_UART_INTR_DISABLE();
    spi_flash_erase_sector(CONFIG_SECTOR);
    spi_flash_write(CONFIG_ADDR, (uint32*) config, sizeof(config_t));
    ETS_UART_INTR_ENABLE();
}

config_t* ICACHE_FLASH_ATTR config_get()
{
    if (!s_config_loaded)
    {
        config_read(&s_config);
        s_config_loaded = 1;
    }
    return &s_config;
}

void ICACHE_FLASH_ATTR config_save()
{
    config_write(&s_config);
    config_t tmp;
    config_read(&tmp);
    if (memcmp(&tmp, &s_config, sizeof(config_t)) != 0)
    {
        os_printf("Error : config verify failed");
    }
}

config_t* ICACHE_FLASH_ATTR config_init()
{
    config_t* config = config_get();
    if (config->version != CONFIG_VERSION)
    {
        config_init_default();
    }
    return config;
}

void ICACHE_FLASH_ATTR config_init_default()
{
    config_t* config = config_get();
    config->version = CONFIG_VERSION;
    config->server_port = DEFAULT_PORT;
    config->device_type = DEVICE_TYPE;
    os_sprintf(config->passwd, DEFAULT_PASSWD, PASSWD_LENGTH);
    config_save();
    
    ETS_UART_INTR_DISABLE();
    spi_flash_erase_sector(CONFIG_WIFI_SECTOR);
    ETS_UART_INTR_ENABLE();
}

void ICACHE_FLASH_ATTR config_update_server_port(int32_t port)
{
   config_t* config = config_get();
   config->server_port = port;
   config_save();
}

uint32_t ICACHE_FLASH_ATTR config_get_device_type()
{
	config_t* config = config_get();
    return config->device_type;
}

uint32_t ICACHE_FLASH_ATTR config_get_server_port()
{
	config_t* config = config_get();
    return config->server_port;
}

void ICACHE_FLASH_ATTR config_get_passwd(char password[8])
{
	config_t* config = config_get();
	strncpy(password, config->passwd, PASSWD_LENGTH);
}
