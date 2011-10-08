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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <mach/msm_rpcrouter.h>
#include <mach/gpio.h>
#include <linux/err.h>


#if 1
#define INFO(fmt, args...) printk(KERN_INFO "qci_pmic: " fmt, ##args)
#define DBG(fmt, args...) printk(KERN_DEBUG "qci_pmic: " fmt, ##args)
#define ERR(fmt, args...) printk(KERN_ERR "qci_pmic: " fmt, ##args)
#else
#define INFO(fmt, args...) do {} while (0)
#define DBG(fmt, args...) do {} while (0)
#endif

#define PM_LIBPROG      0x30000061
#define PM_LIBVERS      0x10001
#define ONCRPC_PM_SET_SPEAKER_GAIN_PROC 21 
#define ONCRPC_PM_SPKR_GET_GAIN_PROC 38
typedef enum
{
    PM_LEFT_SPKR,
    PM_RIGHT_SPKR,
    PM_SPKR_OUT_OF_RANGE          /* Not valid */
}pm_spkr_left_right_type;
/* Valid gain values for the PMIC Speaker */
typedef enum
{
    PM_SPKR_GAIN_MINUS16DB,      /* -16 db */
    PM_SPKR_GAIN_MINUS12DB,      /* -12 db */
    PM_SPKR_GAIN_MINUS08DB,      /* -08 db */
    PM_SPKR_GAIN_MINUS04DB,      /* -04 db */
    PM_SPKR_GAIN_00DB,           /*  00 db */
    PM_SPKR_GAIN_PLUS04DB,       /* +04 db */
    PM_SPKR_GAIN_PLUS08DB,       /* +08 db */
    PM_SPKR_GAIN_PLUS12DB,       /* +12 db */
    PM_SPKR_GAIN_OUT_OF_RANGE    /* Not valid */
}pm_spkr_gain_type;

struct rpc_reply_speaker_gain {
	struct rpc_reply_hdr hdr;
	uint32_t 	dummy0;//speaker_gain_flag;
	pm_spkr_gain_type 	*dummy1;//*speaker_gain;
	pm_spkr_gain_type 	*speaker_gain;
}relpy_speaker_gain;

struct rpc_get_speaker_gain_args {
pm_spkr_left_right_type speaker_gain;
uint8_t gain;
};

struct msm_rpc_endpoint *pmic_rpc_endpoint;
static ssize_t set_speaker_gain(struct device *dev, struct device_attribute *attr,char *buf)
{
	unsigned long val = simple_strtoul(buf, NULL, 10);

	DBG("set_speaker_gain(%lu)\n", val);
	struct set_speaker_gan_req {
		struct rpc_request_hdr hdr;
		uint32_t data;
	} req;
	
	pmic_rpc_endpoint = msm_rpc_connect(PM_LIBPROG, PM_LIBVERS, 0);
	if (IS_ERR(pmic_rpc_endpoint)) {
		printk(KERN_ERR "init vib rpc failed!\n");
		pmic_rpc_endpoint = 0;
		return -1;
	}

	if((val >= 0x0)&&(val <= 0x7))
		req.data = cpu_to_be32(val);
	else{
		ERR("Invalid parameter (speaker gain range : 0 ~ 7) : %d \n",val);
		msm_rpc_close(pmic_rpc_endpoint);
		return -1;
	}

	msm_rpc_call(pmic_rpc_endpoint, ONCRPC_PM_SET_SPEAKER_GAIN_PROC, &req, sizeof(req), 5 * HZ);
	msm_rpc_close(pmic_rpc_endpoint);

	return sprintf(buf, "set_speaker_gain :%ld\n",val);
}
static ssize_t get_speaker_gain(struct device *dev, struct device_attribute *attr, char *buf)
{
	int rc ;
	struct set_speaker_gain_req {
		struct rpc_request_hdr hdr;
		struct rpc_get_speaker_gain_args args;
	} req;

	pmic_rpc_endpoint = msm_rpc_connect(PM_LIBPROG, PM_LIBVERS, 0);
	
	if (IS_ERR(pmic_rpc_endpoint)) {
			printk(KERN_ERR "init vib rpc failed!\n");
			pmic_rpc_endpoint = 0;
			return -1;
	}
	
	req.args.speaker_gain = cpu_to_be32(PM_LEFT_SPKR);
	req.args.gain = cpu_to_be32(0);
	rc = msm_rpc_call_reply(pmic_rpc_endpoint,
					ONCRPC_PM_SPKR_GET_GAIN_PROC,
					&req, sizeof( req),
					&relpy_speaker_gain, sizeof(relpy_speaker_gain),
					5 * HZ);
	if (rc < 0) {
		printk(KERN_ERR"%s: msm_rpc_call_reply failed! proc=%d rc=%d\n",
	       	__func__, ONCRPC_PM_SPKR_GET_GAIN_PROC, rc);
		return rc;
	}
	
	relpy_speaker_gain.dummy0 = be32_to_cpu(relpy_speaker_gain.dummy0);
	relpy_speaker_gain.dummy1 = be32_to_cpu(relpy_speaker_gain.dummy1);
	relpy_speaker_gain.speaker_gain = be32_to_cpu(relpy_speaker_gain.speaker_gain);
		
	DBG(KERN_ERR"PM_LEFT_SPKR --- relpy_speaker_gain.speaker_gain:%d\n",relpy_speaker_gain.speaker_gain);

	#if 0
	req.args.speaker_gain = cpu_to_be32(PM_RIGHT_SPKR);
	req.args.gain = cpu_to_be32(0);
	msm_rpc_call_reply(pmic_rpc_endpoint,
				ONCRPC_PM_SPKR_GET_GAIN_PROC,
				& req, sizeof( req),
				&relpy_speaker_gain, sizeof(relpy_speaker_gain),
				5 * HZ);
		
	relpy_speaker_gain.dummy0 = be32_to_cpu(relpy_speaker_gain.dummy0);
	relpy_speaker_gain.dummy1 = be32_to_cpu(relpy_speaker_gain.dummy1);
	relpy_speaker_gain.speaker_gain = be32_to_cpu(relpy_speaker_gain.speaker_gain);
	
	DBG(KERN_ERR"PM_RIGHT_SPKR --- relpy_speaker_gain.speaker_gain:%d\n",relpy_speaker_gain.speaker_gain);	
	#endif
	
	msm_rpc_close(pmic_rpc_endpoint);

	return sprintf(buf, "speaker_gain :%d\n", relpy_speaker_gain.speaker_gain);
}

static DEVICE_ATTR(speaker_gain, S_IRUGO |S_IWUSR, get_speaker_gain, set_speaker_gain);

static int __init qci_pmic_rpc_probe(struct platform_device *pdev)
{
	int ret = 0;

	DBG( "%s\n", __func__);

	if (!pdev->dev.platform_data) {
		ERR("%s: platform data not initialized\n", __func__);
		return -ENOSYS;
	}

	/* Create a switch for low power mode of semco module. */
	ret = device_create_file(&pdev->dev, &dev_attr_speaker_gain);

	return ret;
}

static int __devexit qci_pmic_rpc_remove(struct platform_device *pdev)
{
	DBG("%s\n", __func__);
	
	device_remove_file(&pdev->dev, &dev_attr_speaker_gain);

	return 0;
}

static struct platform_driver qci_pmic_rpc_driver = {
	.probe = qci_pmic_rpc_probe,
	.remove = __devexit_p(qci_pmic_rpc_remove),
	.driver = {
		.name = "qci_pmic_rpc",
		.owner = THIS_MODULE,
	},
};

static int __init qci_pmic_rpc_init(void)
{
	int ret;

	DBG("%s\n", __func__);
	ret = platform_driver_register(&qci_pmic_rpc_driver);
	return ret;
}

static void __exit qci_pmic_rpc_exit(void)
{
	DBG("%s\n", __func__);
	platform_driver_unregister(&qci_pmic_rpc_driver);
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("QCI PMIC remote control driver");
MODULE_VERSION("1.00");

module_init(qci_pmic_rpc_init);
module_exit(qci_pmic_rpc_exit);
