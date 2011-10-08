/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "msm_fb.h"
#include "mddihost.h"
#include "mddihosti.h"
#include <linux/earlysuspend.h>
int display_initialized = 0;
#define write_client_reg(__X, __Y, __Z) {\
  mddi_queue_register_write(__X, __Y, TRUE, 0);\
}
extern int qci_read_hw_id(void);
struct init_table {
    unsigned int reg;
    unsigned int val;
};
static struct init_table wintek_320x480_init_table[] = {
	{ 0xff,               0x00 }, 
	{ 0x16,                0x49},
	{ 0xe2,               0x00 }, 
	{ 0xe3,               0x00 }, 
	{ 0xf2,               0x00 }, 
	{ 0xe4,               0x1c }, 
	{ 0xe5,               0x1c }, 
        { 0xe6,               0x00},
	{ 0xe7,               0x1c }, 
	{ 0x19,               0x01 }, 
	{ 0,                    12      },
	{ 0x29,               0x01 }, 
	{ 0x18,               0x22 }, 
	{ 0x2a,               0x00 }, 
	{ 0x2b,               0x13 },
	{ 0x02,               0x00 }, 
	{ 0x03,               0x00},
	{ 0x04,               0x01 }, 
	{ 0x05,               0x3f }, 
	{ 0x06,               0x00 }, 
	{ 0x07,               0x00 }, 
	{ 0x08,               0x01 }, 
        { 0x09,               0xdf},
	{ 0x1f,               0x88 }, 
	{ 0xff,               0x02 }, 
        { 0x17,               0x03 },
	{ 0,                  6    },
	{ 0x17,               0x01 }, 
        { 0xff,               0x00 },
	{ 0x24,               0x91 }, 
	{ 0x25,               0x8a }, 
	{ 0x1b,               0x30 }, 
	{ 0,                  12   },
	{ 0x1d,               0x22 }, 
	{ 0,                  12   },
	{ 0x40,               0x00 }, 
	{ 0x41,               0x3c }, 
	{ 0x42,               0x38 }, 
	{ 0x43,               0x34 }, 
	{ 0x44,               0x2e }, 
	{ 0x45,               0x2f},
	{ 0x46,               0x41 }, 
	{ 0x47,               0x7d }, 
	{ 0x48,               0x0b }, 
	{ 0x49,               0x05 }, 
	{ 0x4a,               0x06 }, 
        { 0x4b,               0x12},
	{ 0x4c,               0x16 }, 
	{ 0x50,               0x10 }, 
	{ 0x51,               0x11 }, 
	{ 0x52,               0x0b }, 
	{ 0x53,               0x07 }, 
	{ 0x54,               0x03 }, 
	{ 0x55,               0x3f }, 
	{ 0x56,               0x02 }, 
	{ 0x57,               0x3e},
	{ 0x58,               0x09 }, 
	{ 0x59,               0x0d }, 
	{ 0x5a,               0x19 }, 
	{ 0x5b,               0x1a }, 
	{ 0x5c,               0x14 }, 
	{ 0x5d,               0xc0 }, 
        { 0x01,               0x02},
	{ 0x1a,               0x05 }, 
	{ 0x1c,               0x03 }, 
	{ 0,                  12   },
	{ 0x1f,               0x88 }, 
	{ 0,                  12   },
	{ 0x1f,               0x80 }, 
	{ 0,                  12   },
	{ 0x1f,               0x90 }, 
	{ 0,                  12   },
	{ 0x1f,               0xd2 }, 
	{ 0,                  12   },
	{ 0x28,               0x04 }, 
	{ 0,                  42   },
	{ 0x28,               0x38 }, 
	{ 0,                  42   },
	{ 0x28,               0x3c }, 
	{ 0,                  42   },
	{ 0x80,               0x00},
	{ 0x81,               0x00 }, 
	{ 0x82,               0x00 }, 
	{ 0x83,               0x00 }, 
	{ 0x17,               0x06 }, 
	{ 0x2d,               0x1f }, 
       { 0x60,               0x08},
	{ 0xe8,               0x90 }, 
    { 0, 0 }
};

static struct init_table wintek_320x480_resume_table[] = {
	{ 0xff,               0x00 }, 
        { 0x16,                0x49},
	{ 0xe2,               0x00 }, 
	{ 0xe3,               0x00 }, 
	{ 0xf2,               0x00 }, 
	{ 0xe4,               0x1c }, 
	{ 0xe5,               0x1c }, 
        { 0xe6,               0x00},
	{ 0xe7,               0x1c }, 
	{ 0x19,               0x01 }, 
	{ 0,                    12      },
	{ 0x1f,               0x88 }, 
	{ 0xff,               0x02 }, 
        { 0x17,               0x03 },
	{ 0,                  6    },
	{ 0x17,               0x01 }, 
        { 0xff,               0x00 },
        { 0x01,               0x02},
	{ 0x1a,               0x05 }, 
	{ 0x1c,               0x03 }, 
	{ 0,                  12   },
	{ 0x1f,               0x88 }, 
	{ 0,                  12   },
	{ 0x1f,               0x80 }, 
	{ 0,                  12   },
	{ 0x1f,               0x90 }, 
	{ 0,                  12   },
	{ 0x1f,               0xd2 }, 
	{ 0,                  12   },
	{ 0x28,               0x04 }, 
	{ 0,                  42   },
	{ 0x28,               0x38 }, 
	{ 0,                  42   },
	{ 0x28,               0x3c }, 
	{ 0,                  42   },
	{ 0x80,               0x00},
	{ 0x81,               0x00 }, 
	{ 0x82,               0x00 }, 
	{ 0x83,               0x00 }, 
	{ 0x17,               0x06 }, 
	{ 0x2d,               0x1f }, 
       { 0x60,               0x08},
	{ 0xe8,               0x90 }, 
    { 0, 0 }
};
static int prism_lcd_on(struct platform_device *pdev);
static int prism_lcd_off(struct platform_device *pdev);

#include <mach/gpio.h>
#include <linux/delay.h>
#define LCD_BACKLIGHT_GPIO	26
#define LCD_HVGA_RESETN_GPIO	23
#define LCD_HVGA_RESETN_GPIO_1 132
extern  int gpio_direction_output(unsigned gpio, int value);

static void mddi_prism_lcd_set_backlight(struct msm_fb_data_type *mfd);
int level = 33;
static void mddi_prism_lcd_set_backlight(struct msm_fb_data_type *mfd)
{
	int i;
	int count;
	
	if (mfd->bl_level == 0)
	{
		gpio_direction_output(LCD_BACKLIGHT_GPIO, 0);
		level = 0;
		udelay(1500);
		return;
	}
	       gpio_direction_output(LCD_BACKLIGHT_GPIO, 0);
	       udelay(1500);
	       gpio_direction_output(LCD_BACKLIGHT_GPIO, 1);
	       level = 33;
	       udelay(50);
	       count = level -mfd->bl_level;
	for (i=0; i<count; i++)
	{
		gpio_direction_output(LCD_BACKLIGHT_GPIO, 0); 
		udelay(1);
		gpio_direction_output(LCD_BACKLIGHT_GPIO, 1);
		udelay(1);
	}


}

static int prism_lcd_on(struct platform_device *pdev)
{
    unsigned n;
    if (!display_initialized) {
    n = 0;
    for(;;) {
        if(wintek_320x480_init_table[n].reg == 0) {
            if(wintek_320x480_init_table[n].val == 0) break;
            mdelay(wintek_320x480_init_table[n].val);
        } else {
            write_client_reg(wintek_320x480_init_table[n].reg, wintek_320x480_init_table[n].val, TRUE);
        }
        n++;
    }
    display_initialized = 1;
    }
	/* Set the MDP pixel data attributes for Primary Display */
	mddi_host_write_pix_attr_reg(0x00C3);

	return 0;
}

static int prism_lcd_off(struct platform_device *pdev)
{
	return 0;
}

static int __init prism_probe(struct platform_device *pdev)
{
	msm_fb_add_device(pdev);
	return 0;
}

static struct platform_driver this_driver = {
	.probe  = prism_probe,
	.driver = {
		.name   = "mddi_prism_wvga",
	},
};

static void prism_early_suspend(struct early_suspend *handler) {
      printk(KERN_ERR "[lcd] prism_early_suspend\n");
      write_client_reg(0xFF, 0x00, TRUE);
       write_client_reg(0x28,0x38, TRUE);
       mdelay(40);
       write_client_reg(0x28,0x24, TRUE);
       mdelay(40);
       write_client_reg(0x28,0x04, TRUE);
       write_client_reg(0x1F,0x90, TRUE);
       mdelay(5);
       write_client_reg(0x1F,0x88, TRUE);
       write_client_reg(0x1C,0x00, TRUE);
       write_client_reg(0x1F,0x89, TRUE); 
       write_client_reg(0x19,0x00, TRUE);

}

static void prism_late_resume(struct early_suspend *handler) {
       unsigned n;
       u32 pcbid;
       printk(KERN_ERR "[LCD] prism_late_resume,relaese\n");
        //pcbid = qci_read_hw_id();
        pcbid = 2;
        if(pcbid==0)
        {
	gpio_direction_output(LCD_HVGA_RESETN_GPIO, 0);
	mdelay(2);
	gpio_direction_output(LCD_HVGA_RESETN_GPIO, 1);
	mdelay(12);
        }
        else
        {
	gpio_direction_output(LCD_HVGA_RESETN_GPIO_1, 0);
	mdelay(2);
	gpio_direction_output(LCD_HVGA_RESETN_GPIO_1, 1);
	mdelay(12);	
        }
       n = 0;
    for(;;) {
        if(wintek_320x480_init_table[n].reg == 0) {
            if(wintek_320x480_init_table[n].val == 0) break;
            mdelay(wintek_320x480_init_table[n].val);
        } else {
            write_client_reg(wintek_320x480_init_table[n].reg, wintek_320x480_init_table[n].val, TRUE);
        }
        n++;
    }
    
}
static struct early_suspend prism_power_suspend = {
	.suspend = prism_early_suspend,
	.resume = prism_late_resume,
	.level = EARLY_SUSPEND_LEVEL_DISABLE_FB-25,
};
static struct msm_fb_panel_data prism_panel_data = {
	.on = prism_lcd_on,
	.off = prism_lcd_off,
	.set_backlight = mddi_prism_lcd_set_backlight,
};

static struct platform_device this_device = {
	.name   = "mddi_prism_wvga",
	.id	= 0,
	.dev	= {
		.platform_data = &prism_panel_data,
	}
};

static int __init prism_init(void)
{
	int ret;
	struct msm_panel_info *pinfo;

#ifdef CONFIG_FB_MSM_MDDI_AUTO_DETECT
	u32 id;
	u32 pcbid;
        pcbid = 2;
        if(pcbid==0)
        {
	gpio_direction_output(LCD_HVGA_RESETN_GPIO, 0);
	mdelay(2);
	gpio_direction_output(LCD_HVGA_RESETN_GPIO, 1);
	mdelay(2);
        }
        else
        {
	gpio_direction_output(LCD_HVGA_RESETN_GPIO_1, 0);
	mdelay(2);
	gpio_direction_output(LCD_HVGA_RESETN_GPIO_1, 1);
	mdelay(2);	
        }		
	ret = msm_fb_detect_client("mddi_prism_wvga");
	if (ret == -ENODEV)
		return 0;

	if (ret) {
		id = mddi_get_client_id();
		if (id != 0x8357)
			return 0;
	}
#endif
	ret = platform_driver_register(&this_driver);
	if (!ret) {
		pinfo = &prism_panel_data.panel_info;
		pinfo->xres = 320;
		pinfo->yres = 480;
		pinfo->type = MDDI_PANEL;
		pinfo->pdest = DISPLAY_1;
		pinfo->mddi.vdopkt = MDDI_DEFAULT_PRIM_PIX_ATTR;
		pinfo->wait_cycle = 0;
		pinfo->bpp = 18;
		pinfo->fb_num = 2;
		pinfo->clk_rate = 61440000;
		pinfo->clk_min = 61440000;
		pinfo->clk_max =61440000;
		pinfo->lcd.vsync_enable = TRUE;
		pinfo->lcd.refx100 = 7180;
		pinfo->lcd.v_back_porch = 0;
		pinfo->lcd.v_front_porch = 10;
		pinfo->lcd.v_pulse_width = 0;
		pinfo->lcd.hw_vsync_mode = TRUE;
		pinfo->lcd.vsync_notifier_period = 0;
		pinfo->bl_max = 32;

		register_early_suspend(&prism_power_suspend);
		ret = platform_device_register(&this_device);
		if (ret)
			platform_driver_unregister(&this_driver);
	}

	return ret;
}

module_init(prism_init);
