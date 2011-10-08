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
/*
 * Wireless LAN Power Switch Module
 * controls power to external Wireless LAN device
 * with interface to power management device
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <linux/rfkill.h>
#include <linux/rfkill.h>

#include <mach/gpio.h>

#if 1
#define INFO(fmt, args...) printk(KERN_INFO "qci_wlan_power: " fmt, ##args)
#define DBG(fmt, args...) printk(KERN_DEBUG "qci_wlan_power: " fmt, ##args)
#define ERR(fmt, args...) printk(KERN_ERR "qci_wlan_power: " fmt, ##args)
#else
#define INFO(fmt, args...) do {} while (0)
#define DBG(fmt, args...) do {} while (0)
#endif

#define WLAN_CHIPRST_GPIO 108

static int wlan_power_state;
static int (*power_control)(int enable);

static DEFINE_SPINLOCK(wlan_power_lock);

static ssize_t set_LowPowerMode(struct device *dev, struct device_attribute *attr,
		char *buf, size_t count)
{
	unsigned long val = simple_strtoul(buf, NULL, 10);
	if ( 0 == val ){
		DBG("leave low power mode.\n");
		gpio_set_value(WLAN_CHIPRST_GPIO , 1); 
	}
	else if ( 1 == val ){
		DBG("enter low power mode.\n");
		gpio_set_value(WLAN_CHIPRST_GPIO , 0); 
	}
	else
		return -EINVAL;

	return count;
}

static ssize_t get_LowPowerMode(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int enabled=0, lowpower=0;
	enabled = gpio_get_value(WLAN_CHIPRST_GPIO);
	if ( 0 == enabled ){
		DBG("low power mode enabled.\n");
		lowpower=1;
	}
	else if ( 1 == enabled){
		DBG("low power mode disabled.\n");
		lowpower=0;
	}
	else
		DBG("unknown status.\n");

	return sprintf(buf, "%d\n", lowpower);
}

static DEVICE_ATTR(lowpower, S_IRUGO | S_IWUSR, get_LowPowerMode, set_LowPowerMode);

static int wlan_power_param_set(const char *val, struct kernel_param *kp)
{
	int ret;

	DBG("%s: previous power_state=%d\n", __func__, wlan_power_state);

	/* lock change of state and reference */
	spin_lock(&wlan_power_lock);
	ret = param_set_bool(val, kp);
	if (power_control) {
		if (!ret)
			ret = (*power_control)(wlan_power_state);
		else
			ERR("%s param set bool failed (%d)\n", __func__, ret);
	} else {
		INFO("%s: deferring power switch until probe\n", __func__);
	}
	spin_unlock(&wlan_power_lock);
	INFO("%s: current power_state=%d\n", __func__, wlan_power_state);
	return ret;
}

module_param_call(power, wlan_power_param_set, param_get_bool,
		  &wlan_power_state, S_IWUSR | S_IRUGO);

static int __init wlan_power_probe(struct platform_device *pdev)
{
	int ret = 0;

	DBG( "%s\n", __func__);

	if (!pdev->dev.platform_data) {
		ERR("%s: platform data not initialized\n", __func__);
		return -ENOSYS;
	}

	spin_lock(&wlan_power_lock);
	power_control = pdev->dev.platform_data;

	/* Default power-on wireless */
	wlan_power_state = 1;
	if (wlan_power_state) {
		INFO("%s: handling deferred power switch\n", __func__);
	}
	ret = (*power_control)(wlan_power_state);
	spin_unlock(&wlan_power_lock);

	/* Create a switch for low power mode of semco module. */
	ret = device_create_file(&pdev->dev, &dev_attr_lowpower);

	return ret;
}

static int __devexit wlan_power_remove(struct platform_device *pdev)
{
	int ret;

	DBG("%s\n", __func__);
	
	device_remove_file(&pdev->dev, &dev_attr_lowpower);

	if (!power_control) {
		ERR("%s: power_control function not initialized\n",	__func__);
		return -ENOSYS;
	}
	spin_lock(&wlan_power_lock);
	wlan_power_state = 0;
	ret = (*power_control)(wlan_power_state);
	power_control = NULL;
	spin_unlock(&wlan_power_lock);

	return ret;
}

static struct platform_driver wlan_power_driver = {
	.probe = wlan_power_probe,
	.remove = __devexit_p(wlan_power_remove),
	.driver = {
		.name = "wlan_power",
		.owner = THIS_MODULE,
	},
};

static int __init wlan_power_init(void)
{
	int ret;

	DBG("%s\n", __func__);
	ret = platform_driver_register(&wlan_power_driver);
	return ret;
}

static void __exit wlan_power_exit(void)
{
	DBG("%s\n", __func__);
	platform_driver_unregister(&wlan_power_driver);
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("QCI WLAN power control driver");
MODULE_VERSION("1.00");
MODULE_PARM_DESC(power, "QCI WLAN power switch (bool): 0,1=off,on");

module_init(wlan_power_init);
module_exit(wlan_power_exit);
