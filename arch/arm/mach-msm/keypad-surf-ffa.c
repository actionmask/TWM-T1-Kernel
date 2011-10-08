/*
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/platform_device.h>
#include <linux/gpio_event.h>

#include <asm/mach-types.h>

/* don't turn this on without updating the ffa support */
#define SCAN_FUNCTION_KEYS 0

static unsigned int keypad_row_gpios[] = { 35, 34, 33 };
static unsigned int keypad_col_gpios[] = { 42, 41, 40, 39 };

static unsigned int keypad_row_gpios_8k_ffa[] = { 35, 34, 33 };
static unsigned int keypad_col_gpios_8k_ffa[] = { 42, 41, 40, 39};

#define KEYMAP_INDEX(row, col) ((row)*ARRAY_SIZE(keypad_col_gpios) + (col))
#define FFA_8K_KEYMAP_INDEX(row, col) ((row)* \
				ARRAY_SIZE(keypad_col_gpios_8k_ffa) + (col))

static const unsigned short keypad_keymap_surf[ARRAY_SIZE(keypad_col_gpios) *
					  ARRAY_SIZE(keypad_row_gpios)] = {
	[KEYMAP_INDEX(0, 0)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(0, 1)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(0, 2)] = KEY_ZOOM,
	[KEYMAP_INDEX(0, 3)] = KEY_CAMERA,

	[KEYMAP_INDEX(1, 0)] = KEY_SEND,
	[KEYMAP_INDEX(1, 1)] = KEY_BACK,
	//IR-sensor button event is KEY_CENTER
	[KEYMAP_INDEX(1, 2)] = 232, //IR DOME
	[KEYMAP_INDEX(1, 3)] = KEY_RESERVED,
	
	[KEYMAP_INDEX(2, 0)] = KEY_MENU,
	[KEYMAP_INDEX(2, 1)] = KEY_HOME,
	[KEYMAP_INDEX(2, 2)] = KEY_SEARCH, 
	/*[KEYMAP_INDEX(2, 3)] = KEY_RESERVED,*/	
};

static const unsigned short keypad_keymap_ffa[ARRAY_SIZE(keypad_col_gpios) *
					      ARRAY_SIZE(keypad_row_gpios)] = {
	[KEYMAP_INDEX(0, 0)] = KEY_VOLUMEUP,
	[KEYMAP_INDEX(0, 1)] = KEY_VOLUMEDOWN,
	[KEYMAP_INDEX(0, 2)] = KEY_ZOOM,
	[KEYMAP_INDEX(0, 3)] = KEY_CAMERA,

	[KEYMAP_INDEX(1, 0)] = KEY_SEND,
	[KEYMAP_INDEX(1, 1)] = KEY_BACK,
	//IR-sensor button event is KEY_CENTER
	[KEYMAP_INDEX(1, 2)] = 232,//IR DOME
	[KEYMAP_INDEX(1, 3)] = KEY_RESERVED,

	[KEYMAP_INDEX(2, 0)] = KEY_MENU,
	[KEYMAP_INDEX(2, 1)] = KEY_HOME,
	[KEYMAP_INDEX(2, 2)] = KEY_SEARCH, 
	/*[KEYMAP_INDEX(2, 3)] = KEY_RESERVED,*/
};

#define QSD8x50_FFA_KEYMAP_SIZE (ARRAY_SIZE(keypad_col_gpios_8k_ffa) * \
			ARRAY_SIZE(keypad_row_gpios_8k_ffa))

static const unsigned short keypad_keymap_8k_ffa[QSD8x50_FFA_KEYMAP_SIZE] = {

	[FFA_8K_KEYMAP_INDEX(0, 0)] = KEY_VOLUMEUP,
	[FFA_8K_KEYMAP_INDEX(0, 1)] = KEY_VOLUMEDOWN,
	[FFA_8K_KEYMAP_INDEX(0, 2)] = KEY_ZOOM,
	[FFA_8K_KEYMAP_INDEX(0, 3)] = KEY_CAMERA,

	[FFA_8K_KEYMAP_INDEX(1, 0)] = KEY_SEND,
	[FFA_8K_KEYMAP_INDEX(1, 1)] = KEY_BACK,
	//IR-sensor button event is KEY_CENTER
	[FFA_8K_KEYMAP_INDEX(1, 2)] = 232,//IR DOME
	[FFA_8K_KEYMAP_INDEX(1, 3)] = KEY_RESERVED,
	
	[FFA_8K_KEYMAP_INDEX(2, 0)] = KEY_MENU,
	[FFA_8K_KEYMAP_INDEX(2, 1)] = KEY_HOME,
	[FFA_8K_KEYMAP_INDEX(2, 2)] = KEY_SEARCH,
	/*[FFA_8K_KEYMAP_INDEX(2, 3)] =  KEY_RESERVED,*/
};

/* SURF keypad platform device information */
static struct gpio_event_matrix_info surf_keypad_matrix_info = {
	.info.func	= gpio_event_matrix_func,
	.keymap		= keypad_keymap_surf,
	.output_gpios	= keypad_row_gpios,
	.input_gpios	= keypad_col_gpios,
	.noutputs	= ARRAY_SIZE(keypad_row_gpios),
	.ninputs	= ARRAY_SIZE(keypad_col_gpios),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_DRIVE_INACTIVE |
			  GPIOKPF_PRINT_UNMAPPED_KEYS
};

static struct gpio_event_info *surf_keypad_info[] = {
	&surf_keypad_matrix_info.info
};

static struct gpio_event_platform_data surf_keypad_data = {
	.name		= "surf_keypad",
	.info		= surf_keypad_info,
	.info_count	= ARRAY_SIZE(surf_keypad_info)
};

struct platform_device keypad_device_surf = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &surf_keypad_data,
	},
};

/* 8k FFA keypad platform device information */
static struct gpio_event_matrix_info keypad_matrix_info_8k_ffa = {
	.info.func	= gpio_event_matrix_func,
	.keymap		= keypad_keymap_8k_ffa,
	.output_gpios	= keypad_row_gpios_8k_ffa,
	.input_gpios	= keypad_col_gpios_8k_ffa,
	.noutputs	= ARRAY_SIZE(keypad_row_gpios_8k_ffa),
	.ninputs	= ARRAY_SIZE(keypad_col_gpios_8k_ffa),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_DRIVE_INACTIVE |
			  GPIOKPF_PRINT_UNMAPPED_KEYS
};

static struct gpio_event_info *keypad_info_8k_ffa[] = {
	&keypad_matrix_info_8k_ffa.info
};

static struct gpio_event_platform_data keypad_data_8k_ffa = {
	.name		= "8k_ffa_keypad",
	.info		= keypad_info_8k_ffa,
	.info_count	= ARRAY_SIZE(keypad_info_8k_ffa)
};

struct platform_device keypad_device_8k_ffa = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &keypad_data_8k_ffa,
	},
};

/* 7k FFA keypad platform device information */
static struct gpio_event_matrix_info keypad_matrix_info_7k_ffa = {
	.info.func	= gpio_event_matrix_func,
	.keymap		= keypad_keymap_ffa,
	.output_gpios	= keypad_row_gpios,
	.input_gpios	= keypad_col_gpios,
	.noutputs	= ARRAY_SIZE(keypad_row_gpios),
	.ninputs	= ARRAY_SIZE(keypad_col_gpios),
	.settle_time.tv.nsec = 40 * NSEC_PER_USEC,
	.poll_time.tv.nsec = 20 * NSEC_PER_MSEC,
	.flags		= GPIOKPF_LEVEL_TRIGGERED_IRQ | GPIOKPF_DRIVE_INACTIVE |
			  GPIOKPF_PRINT_UNMAPPED_KEYS
};

static struct gpio_event_info *keypad_info_7k_ffa[] = {
	&keypad_matrix_info_7k_ffa.info
};

static struct gpio_event_platform_data keypad_data_7k_ffa = {
	.name		= "7k_ffa_keypad",
	.info		= keypad_info_7k_ffa,
	.info_count	= ARRAY_SIZE(keypad_info_7k_ffa)
};

struct platform_device keypad_device_7k_ffa = {
	.name	= GPIO_EVENT_DEV_NAME,
	.id	= -1,
	.dev	= {
		.platform_data	= &keypad_data_7k_ffa,
	},
};

