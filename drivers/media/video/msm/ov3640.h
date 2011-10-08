/* Copyright (c) 2009, Quanta. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */

#ifndef OV3640_H
#define OV3640_H

#include <linux/types.h>
#include <mach/camera.h>

extern struct ov3640_reg ov3640_regs;

enum ov3640_width {
	WORD_LEN,
	BYTE_LEN,
	WORD_ADDR_BYTE_LEN
};

struct ov3640_i2c_reg_conf {
	unsigned short waddr;
	unsigned short wdata;
	enum ov3640_width width;
	unsigned short mdelay_time;
};

struct ov3640_reg {
	const struct register_address_value_pair *prev_snap_reg_settings;
	uint16_t prev_snap_reg_settings_size;	
	const struct register_address_value_pair  *prev_reg_settings;
	uint16_t prev_reg_settings_size;
	const struct register_address_value_pair  *snap_reg_settings;
	uint16_t snap_reg_settings_size;
};

#endif /* OV3640_H */
