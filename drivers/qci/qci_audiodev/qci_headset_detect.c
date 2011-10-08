/* Quanta SYS Driver
 *
 * Copyright (C) 2009 Quanta Computer Inc.
 * Author: Marty Lin <Marty.Lin@quantatw.com>
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/switch.h>
#include <linux/types.h>
#include <linux/input.h>
#include <mach/pmic.h>
#include <linux/delay.h>
#include <linux/earlysuspend.h>
#include <mach/msm_rpcrouter.h>
#if 0
#define D(fmt, args...) printk(KERN_INFO "qci_headset_detect: " fmt, ##args)
#define ERR(fmt, args...) printk(KERN_ERR "qci_headset_detect: " fmt, ##args)
#else
#define D(fmt, args...) do {} while (0)
#define ERR(fmt, args...) do {} while (0)
#endif

#define SWITCH_GPIO 17
#define DETECT_GPIO 18
#define SWITCH_ENABLED 1
#define SWITCH_DISABLED 0
#define KEY_SOFT1		229
struct qci_headset_detect_data {
	struct switch_dev sdev;
	unsigned gpio;
	const char *name_on;
	const char *name_off;
	const char *state_on;
	const char *state_off;
	int irq;
	unsigned mic_switch;
	unsigned headset_switch;
	struct work_struct work;
	struct input_dev *ipdev;
};

#define QCI_HS_API_PROG             0x300000a0
#define QCI_HS_API_VERS             0x00010001
#define QCI_HS_DETECT_PROC          2
static struct msm_rpc_endpoint *qci_headset_ep;

void hs_detect_status_rpc(unsigned int amp_on_off)
{
	int rc = 0;
	
		struct qci_headset_detect_req {
		struct rpc_request_hdr hdr;
		uint32_t data;
	} req;

	qci_headset_ep = msm_rpc_connect_compatible(QCI_HS_API_PROG, QCI_HS_API_VERS, 0);
	if (IS_ERR(qci_headset_ep)) {
		printk(KERN_ERR "%s: msm_rpc_connect failed! rc = %ld\n",
			__func__, PTR_ERR(qci_headset_ep));
		return -EINVAL;
	}

req.data = cpu_to_be32(amp_on_off);
	rc = msm_rpc_call(qci_headset_ep,QCI_HS_DETECT_PROC, &req, sizeof(req), 5 * HZ);	

	if (rc)
		printk(KERN_ERR
			"%s: msm_rpc_call failed! rc = %d\n", __func__, rc);

	msm_rpc_close(qci_headset_ep);
}

static void qci_headset_detect_work(struct work_struct *work)
{
	int state = 0;
	int enabled = 0;
	int ret = 0;

	struct qci_headset_detect_data	*data =
		container_of(work, struct qci_headset_detect_data, work);

	state = gpio_get_value(data->gpio);

	D("IRQ Handler GPIO(%d) Status : %d ( 1 = enable, 0 = disable )\n",data->gpio , state);

		ret = pmic_mic_en(state);
		D("pmic_mic_en() status : %d ( 1 = enable, 0 = disable )\n", state);
		ret = pmic_mic_is_en(&enabled);
		D("after pmic_mic_is_en() status : %d ( 1 = enable, 0 = disable )\n", enabled);
		
		gpio_set_value(SWITCH_GPIO, state);
		D("gpio(%d) configured to : %d\n", SWITCH_GPIO, gpio_get_value(SWITCH_GPIO));
		input_report_key(data->ipdev, KEY_SOFT1, 1);
		mdelay(1);
		input_report_key(data->ipdev, KEY_SOFT1, 0);		

	switch_set_state(&data->sdev, state);

}

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{

	struct qci_headset_detect_data *switch_data =
	    (struct qci_headset_detect_data *)dev_id;

	schedule_work(&switch_data->work);
	return IRQ_HANDLED;
}

static ssize_t qci_headset_detect_print_state(struct switch_dev *sdev, char *buf)
{
	struct qci_headset_detect_data	*switch_data =
		container_of(sdev, struct qci_headset_detect_data, sdev);
	const char *state;
	if (switch_get_state(sdev))
		state = switch_data->state_on;
	else
		state = switch_data->state_off;

	if (state)
		return sprintf(buf, "%s\n", state);
	return -1;
}

static ssize_t show_gpio_no(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct switch_dev *sdev = (struct switch_dev *)
		dev_get_drvdata(dev);

	struct qci_headset_detect_data	*switch_data =
		container_of(sdev, struct qci_headset_detect_data, sdev);

	return sprintf(buf, "%d\n", switch_data->gpio);
}
static DEVICE_ATTR(gpio_no, S_IRUGO | S_IWUSR, show_gpio_no, NULL);

static int qci_headset_detect_probe(struct platform_device *pdev)
{
	struct gpio_switch_platform_data *pdata = pdev->dev.platform_data;
	struct qci_headset_detect_data *switch_data;
	int ret = 0;
	struct input_dev *ipdev;
	int state;
	int rc;

	if (!pdata)
		return -EBUSY;

	switch_data = kzalloc(sizeof(struct qci_headset_detect_data), GFP_KERNEL);
	if (!switch_data)
		return -ENOMEM;

	switch_data->sdev.name = pdata->name;
	switch_data->gpio = pdata->gpio;
	switch_data->name_on = pdata->name_on;
	switch_data->name_off = pdata->name_off;
	switch_data->state_on = pdata->state_on;
	switch_data->state_off = pdata->state_off;
	switch_data->sdev.print_state = qci_headset_detect_print_state;

	switch_data->headset_switch = SWITCH_GPIO;

	ret = gpio_tlmm_config(GPIO_CFG(switch_data->gpio, 0, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	ret = gpio_tlmm_config(GPIO_CFG(switch_data->headset_switch, 0, GPIO_OUTPUT,  GPIO_NO_PULL, GPIO_2MA), GPIO_ENABLE);
	if (ret) {
	    ERR("%s: gpio_tlmm_config(%#x)=%d\n",
	       __func__, GPIO_CFG(switch_data->gpio, 0, GPIO_INPUT,  GPIO_PULL_UP, GPIO_2MA), ret);
	    ERR("%s: gpio_tlmm_config(%#x)=%d\n",
	       __func__, GPIO_CFG(switch_data->headset_switch, 0, GPIO_OUTPUT,  GPIO_NO_PULL, GPIO_2MA), ret);
		return -EIO;
	}

    	ret = switch_dev_register(&switch_data->sdev);
	if (ret < 0){
		ERR("err_switch_dev_register\n");
		goto err_switch_dev_register;
	}

	ret = gpio_request(switch_data->gpio, pdev->name);
	if (ret < 0){
		ERR("err_request_gpio\n");
		goto err_request_gpio;
	}

	ret = gpio_request(switch_data->headset_switch, pdev->name);
	if (ret < 0){
		ERR("err_request_gpio\n");
		goto err_request_gpio;
	}

	ret = gpio_direction_input(switch_data->gpio);
	if (ret < 0){
		ERR("err_set_gpio_input\n");
		goto err_set_gpio_input;
	}

	state = gpio_get_value(switch_data->gpio);
	D("Check headset status : %d ( 1 = plug in, 0 = remove )\n", state );
	if(0x1 ==state){
		gpio_direction_output(switch_data->headset_switch, SWITCH_ENABLED);
	}
	else{
		gpio_direction_output(switch_data->headset_switch, SWITCH_DISABLED);
	}


	INIT_WORK(&switch_data->work, qci_headset_detect_work);

	switch_data->irq = gpio_to_irq(switch_data->gpio);
	if (switch_data->irq < 0) {
		ret = switch_data->irq;
		goto err_detect_irq_num_failed;
	}

	ret = request_irq(switch_data->irq, gpio_irq_handler,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, 
			  pdev->name, switch_data);

	if (ret < 0)
		goto err_request_irq;

		ipdev = input_allocate_device();
		if (!ipdev) {
			rc = -ENOMEM;
			goto err_request_irq;
		}
		input_set_drvdata(ipdev, switch_data);
		ipdev->name = "qci_hs_switch";
		switch_data->ipdev = ipdev;

		ipdev->id.vendor	= 0x0001;
		ipdev->id.product	= 1;
		ipdev->id.version	= 1;

		input_set_capability(ipdev, EV_KEY, KEY_SOFT1);

		rc = input_register_device(ipdev);
		if (rc) {
			dev_err(&ipdev->dev,"qci_headset_detect_probe: input_register_device rc=%d\n", rc);
		goto err_reg_input_dev;
		}

		platform_set_drvdata(pdev, switch_data);
	
	/* Perform initial detection */
	qci_headset_detect_work(&switch_data->work);

	ret = device_create_file(switch_data->sdev.dev, &dev_attr_gpio_no);
	if (ret < 0)
		goto err_create_file;

	return 0;
	
	err_create_file:
	err_reg_input_dev:
	input_free_device(ipdev);
	err_request_irq:
	err_detect_irq_num_failed:
	err_set_gpio_input:
	gpio_free(switch_data->gpio);
	err_request_gpio:
    	switch_dev_unregister(&switch_data->sdev);
	err_switch_dev_register:
	kfree(switch_data);

	return ret;
}

static int __devexit qci_headset_detect_remove(struct platform_device *pdev)
{
	struct qci_headset_detect_data *switch_data = platform_get_drvdata(pdev);

	device_remove_file(switch_data->sdev.dev, &dev_attr_gpio_no);

	cancel_work_sync(&switch_data->work);
	
	input_unregister_device(switch_data->ipdev);

	gpio_free(switch_data->gpio);
	switch_dev_unregister(&switch_data->sdev);
	kfree(switch_data);

	return 0;
}

#ifdef CONFIG_PM
#define headset_suspend NULL
#define headset_resume NULL
#endif

static struct platform_driver qci_headset_detect_driver = {
	.probe		= qci_headset_detect_probe,
	.remove		= __devexit_p(qci_headset_detect_remove),
	.driver		= {
		.name	= "qci_headset_detect",
		.owner	= THIS_MODULE,
	},
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void  qci_headset_early_suspend(struct early_suspend *handler) {
	hs_detect_status_rpc(0x0);
}

static void  qci_headset_late_resume(struct early_suspend *handler) {
	hs_detect_status_rpc(0x1);
}

static struct early_suspend qci_headset_suspend = {
	.suspend = qci_headset_early_suspend,
	.resume = qci_headset_late_resume,
	.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN,
};
#endif

static int __init qci_headset_detect_init(void)
{
	#ifdef CONFIG_HAS_EARLYSUSPEND
	register_early_suspend(&qci_headset_suspend);
	#endif

	return platform_driver_register(&qci_headset_detect_driver);
}

static void __exit qci_headset_detect_exit(void)
{
	#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&qci_headset_suspend);
	#endif

	platform_driver_unregister(&qci_headset_detect_driver);
}

module_init(qci_headset_detect_init);
module_exit(qci_headset_detect_exit);

MODULE_AUTHOR("Marty Lin <marty.lin@quantatw.com>");
MODULE_DESCRIPTION("QCI Headset Detect");
MODULE_LICENSE("GPL");
