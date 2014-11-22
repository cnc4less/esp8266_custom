/* Copyright (c) 2014 Ivan Grokhotkov. All rights reserved. 
 * This file is part of the atproto AT protocol library
 *
 * Redistribution and use is permitted according to the conditions of the
 * 3-clause BSD license to be found in the LICENSE file.
 */

#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include "c_types.h"

typedef struct {
    uint32_t version;
    uint32_t server_port;
    uint16_t device_type;
    uint16_t passwd;
    // bump CONFIG_VERSION when adding new fields
} config_t;


config_t* config_get();
void config_save();
config_t* config_init();
void config_init_default();
void config_update_server_port(int32_t port);
void config_update_password(uint16_t password);
uint16_t config_get_device_type();
uint32_t config_get_server_port();
uint16_t config_get_password();

#endif//CONFIG_STORE_H
