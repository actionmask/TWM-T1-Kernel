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

#include "ov3640.h"


struct register_address_value_pair const
ov3640_preview_snapshot_mode_reg_settings_array[] = {
	{0x3012, 0x80},
	{0x3085, 0x20},
	{0x304d, 0x45},
	{0x30a7, 0x5e},
	{0x3087, 0x16},
	{0x309C, 0x1a},
	{0x30a2, 0xe4},
	{0x30aa, 0x42},
	{0x30b0, 0xff},
	{0x30b1, 0xff},
	{0x30b2, 0x13},
	{0x300e, 0x38},
	{0x300f, 0x21},
	{0x3010, 0x20},
	{0x3011, 0x00},
	{0x304c, 0x82},
	{0x30d7, 0x10},
	{0x30d9, 0x0d},
	{0x30db, 0x08},
	{0x3016, 0x82},
	{0x3018, 0x38},
	{0x3019, 0x30},
	{0x301a, 0x61},
	{0x307d, 0x00},
	{0x3087, 0x02},
	{0x3082, 0x20},
	{0x3015, 0x10},
	{0x3014, 0x0c},
	{0x3013, 0xf7},
	{0x303c, 0x08},
	{0x303d, 0x18},
	{0x303e, 0x06},
	{0x303F, 0x0c},
	{0x3030, 0x62},
	{0x3031, 0x26},
	{0x3032, 0xe6},
	{0x3033, 0x6e},
	{0x3034, 0xea},
	{0x3035, 0xae},
	{0x3036, 0xa6},
	{0x3037, 0x6a},
	{0x3104, 0x02},
	{0x3105, 0xfd},
	{0x3106, 0x00},
	{0x3107, 0xff},
	{0x3300, 0x12},
	{0x3301, 0xde},
	{0x3302, 0xcf},
	{0x336a, 0x52},
	{0x3370, 0x46},
	{0x3376, 0x38},
	{0x3300, 0x13},
	{0x30b8, 0x20},
	{0x30b9, 0x17},
	{0x30ba, 0x04},
	{0x30bb, 0x08},
	{0x3507, 0x06},
	{0x350a, 0x4f},
	{0x3100, 0x32},
	{0x3304, 0x00},
	{0x3400, 0x02},
	{0x3404, 0x22},
	{0x3500, 0x00},
	{0x3600, 0xc4},
	{0x3610, 0x60},
	{0x350a, 0x4f},
	{0x3088, 0x08},
	{0x3089, 0x00},
	{0x308a, 0x06},
	{0x308b, 0x00},
	{0x308d, 0x04},
	{0x3086, 0x03},
	{0x3086, 0x00},
	{0x3012, 0x10},
	{0x3023, 0x06},
	{0x3026, 0x03},
	{0x3027, 0x04},
	{0x302a, 0x03},
	{0x302b, 0x10},
	{0x3075, 0x24},
	{0x300d, 0x01},
	{0x30d7, 0x90},
	{0x3069, 0x04},
	{0x303e, 0x00},
	{0x303f, 0xc0},
	{0x3302, 0xef},
	{0x335f, 0x34},
	{0x3360, 0x0c},
	{0x3361, 0x04},
	{0x3362, 0x34},
	{0x3363, 0x08},
	{0x3364, 0x04},
	{0x3403, 0x42},
	{0x3088, 0x04},
	{0x3089, 0x00},
	{0x308a, 0x03},
	{0x308b, 0x00},
	{0x3100, 0x02},
	{0x3301, 0xde},
	{0x3304, 0x00},
	{0x3400, 0x00},
	{0x3404, 0x02},
	{0x3600, 0xc4},
	{0x30a9, 0xbd},
	{0x3317,0x10},
	{0x3316,0xf0},
	{0x3312,0x30},
	{0x3314,0x42},
	{0x3313,0x15},
	{0x3315 ,0x1e},
	{0x3311 ,0xd4},
	{0x3310 ,0xff},
	{0x330c,0x0e},
	{0x330d,0x0e},
	{0x330e,0x50},
	{0x330f,0x55},
	{0x330b,0x0e},
	{0x3306,0xfc},
	{0x3307,0x11},
	{0x3308,0x25},
	{0x3340 , 0x20},
	{0x3341 , 0x64},
	{0x3342 , 0x08},
	{0x3343 , 0x3f},
	{0x3344 , 0xbf},
	{0x3345 , 0xff},
	{0x3346 , 0xff},
	{0x3347 , 0xf1},
	{0x3348 , 0x0d},
	{0x3349 , 0x98},
	{0x333f , 0x06},
        {0x3367,  0x34},
	{0x3368 , 0x30},
	{0x3369 , 0x20},
	{0x336a , 0x59},
	{0x336b , 0x87},
	{0x336c , 0xc2},
	{0x336d , 0x34},
	{0x336e , 0x40},
	{0x336f , 0x30},
	{0x3370 , 0x53},
	{0x3371 , 0x87},
	{0x3372 , 0xc2},
	{0x3373 , 0x34},
	{0x3374 , 0x30},
	{0x3375 , 0x30},
	{0x3376 , 0x4d},
	{0x3377 , 0x87},
	{0x3378 , 0xc2},
	{0x331b,0x0e},
	{0x331c,0x1a},
	{0x331d,0x31},
	{0x331e,0x5a},
	{0x331f,0x69},
	{0x3320,0x75},
	{0x3321,0x7e},
	{0x3322,0x88},
	{0x3323,0x8f},
	{0x3324,0x96},
	{0x3325,0xa3},
	{0x3326,0xaf},
	{0x3327,0xc4},
	{0x3328,0xd7},
	{0x3329,0xe8},
	{0x332a,0x20},      
	{0x3030 , 0x62},
	{0x3031 , 0x26},
	{0x3032 , 0xe6},
	{0x3033 , 0x6e},
	{0x3034 , 0xea},
	{0x3035 , 0xae},
	{0x3036 , 0xa6},
	{0x3037 , 0x6a},
        {0x3018,0x35},
	{0x3019,0x2a },
	{0x301a,0x61},
	{0x303c,0x08},
	{0x303d,0x18},
	{0x303e,0x06},
	{0x303F,0x0c},
	{0x3030,0x62},
	{0x3031,0x26},
	{0x3032,0xe6},
	{0x3033,0x6e},
	{0x3034,0xea},
	{0x3035,0xae},
	{0x3036,0xa6},
	{0x3037,0x6a},
	{0x3047,0x05},
	{0x3070,0x00},
	{0x3071,0xec},
	{0x3072,0x00},
	{0x3073,0xc4},
	{0x301c,0x02},
	{0x301d,0x03},
};

struct register_address_value_pair const
ov3640_preview_mode_reg_settings_array[] = {
	// OV3640, YUV, XGA keysetting
	{0x3012,0x10},
	{0x3020,0x01},
	{0x3021,0x1d},
	{0x3022,0x00},
	{0x3023,0x06},
	{0x3024,0x08},
	{0x3025,0x18},
	{0x3026,0x03},
	{0x3027,0x04},
	{0x302a,0x03},
	{0x302b,0x10},                                      
	{0x3075,0x24},
	{0x300d,0x01},
	{0x30d7,0x90},
	{0x3069,0x04},
	{0x3302,0xef},
	{0x335f,0x34},
	{0x3360,0x0c},
	{0x3361,0x04},
	{0x3362,0x34},
	{0x3363,0x08},
	{0x3364,0x04},
	{0x3403,0x42},
	{0x3088,0x04},
	{0x3089,0x00},
	{0x308a,0x03},
	{0x308b,0x00},
	{0x304c,0x82},
	{0x3011,0x00},
	{0x3014, 0x0c},
	{0x3013, 0xf7},
	{0x3366,0x15},
	{0x3302, 0xcf},


};

struct register_address_value_pair const
ov3640_snapshot_mode_reg_settings_array[] = {
	{0x3013,0xF0},
	{0x3014,0x04},
	{0x3012,0x00},
	{0x3020,0x01},
	{0x3021,0x1d},
	{0x3022,0x00},
	{0x3023,0x0a},
	{0x3024,0x08},
	{0x3025,0x18},
	{0x3026,0x06},
	{0x3027,0x0c},
	{0x302a,0x06},
	{0x302b,0x20},                                      
	{0x3075,0x44},
	{0x300d,0x00},
	{0x30d7,0x10},
	{0x3069,0x44},
	{0x303e,0x01},
	{0x303f,0x80},
	{0x3302,0xee},
	{0x335f,0x68},
	{0x3360,0x18},
	{0x3361,0x0c},
	{0x3362,0x68},
	{0x3363,0x08},
	{0x3364,0x04},
	{0x3403,0x42},
	{0x3088,0x08},
	{0x3089,0x00},
	{0x308a,0x06},
	{0x308b,0x00},
	{0x304c,0x81},
	{0x3011,0x01},
	{0x3366,0x10}


};

struct ov3640_reg ov3640_regs = {
	.prev_snap_reg_settings = &ov3640_preview_snapshot_mode_reg_settings_array[0],
	.prev_snap_reg_settings_size = ARRAY_SIZE(
		ov3640_preview_snapshot_mode_reg_settings_array),
	.prev_reg_settings = &ov3640_preview_mode_reg_settings_array[0],
	.prev_reg_settings_size =
		ARRAY_SIZE(ov3640_preview_mode_reg_settings_array),
	.snap_reg_settings = &ov3640_snapshot_mode_reg_settings_array[0],
	.snap_reg_settings_size =
		ARRAY_SIZE(ov3640_snapshot_mode_reg_settings_array),
};



