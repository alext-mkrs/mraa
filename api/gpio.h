/*
 * Originally from mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 * Copyright (c) 2014 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdio.h>

#include "maa.h"

typedef struct {
    int pin;
    int pinMap;
    char path[64];
    FILE *value_fp;
} gpio_t;

typedef char gpio_mode_t[16];
typedef char gpio_dir_t[16];

maa_result_t maa_gpio_init(gpio_t *gpio, int pin);
int maa_gpio_set(int pin);
void maa_gpio_mode(gpio_t *gpio, gpio_mode_t mode);
void maa_gpio_dir(gpio_t *gpio, gpio_dir_t dir);

void maa_gpio_close(gpio_t *gpio);
int maa_gpio_read(gpio_t *gpio);
void maa_gpio_write(gpio_t *gpio, int value);
