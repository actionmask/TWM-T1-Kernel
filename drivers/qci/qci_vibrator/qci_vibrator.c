/* Quanta Vibrator Driver
 *
 * Copyright (C) 2009 Quanta Computer Inc.
 * Author: Weitin Chiang <Weitin.Chiang@quantatw.com>
 * Author: Jacky Hsu <Jacky.Hsu@quantatw.com>
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
#include <mach/rpc_hsusb.h>
#include <../../../drivers/staging/android/timed_output.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <linux/sched.h>
#include <mach/msm_rpcrouter.h>

#define PM_LIBPROG      0x30000061
#define PM_LIBVERS      0x10001

#define PROCEDURE_SET_VIB_ON_OFF	22
#define PMIC_VIBRATOR_LEVEL	(3000)

static struct hrtimer vibe_timer;
static struct work_struct vibrator_work;
static spinlock_t vibe_lock;

static int qci_vibrator_get_time(struct timed_output_dev *dev);
static void qci_vibrator_enable(struct timed_output_dev *dev, int val);

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

spinlock_t lock;
bool isVibratorON = FALSE;
bool preVibstate = FALSE;

static void set_pmic_vibrator(int on)
{
	static struct msm_rpc_endpoint *vib_endpoint;
	struct set_vib_on_off_req {
		struct rpc_request_hdr hdr;
		uint32_t data;
	} req;
	
	if (!vib_endpoint) {
		vib_endpoint = msm_rpc_connect(PM_LIBPROG, PM_LIBVERS, 0);
		if (IS_ERR(vib_endpoint)) {
			printk(KERN_ERR "init vib rpc failed!\n");
			vib_endpoint = 0;
			return;
		}
	}

       if (on && preVibstate == TRUE)
		return;
	   
       if (!on && preVibstate == FALSE)
		return;

	
       if (on) {
		req.data = cpu_to_be32(PMIC_VIBRATOR_LEVEL);	
		preVibstate = TRUE;
	} else {
		req.data = cpu_to_be32(0);
		preVibstate = FALSE;
	}		

	msm_rpc_call(vib_endpoint, PROCEDURE_SET_VIB_ON_OFF, &req, sizeof(req), 5 * HZ);
}

static void update_vibrator(struct work_struct *work)
{
//	printk(KERN_INFO "[vibrator] update_vibrator : %d \n",isVibratorON);
	set_pmic_vibrator(isVibratorON);
}

static void qci_vibrator_enable(struct timed_output_dev *dev, int val)
{
	unsigned long	flags;
	
	spin_lock_irqsave(&vibe_lock, flags);
	hrtimer_cancel(&vibe_timer);

	printk(KERN_INFO "[vibrator] application set time: %d\n",val);	//add debug mesg

	if (val == 0) 	{
		isVibratorON = FALSE;
	} else if( val > 0 ) {
		val = (val > 15000 ? 15000 : val);	//make sure vibrate time < 15 sec or other times
		val = (val < 100 ? 100 : val);		//make sure vibrate time > 0.1 sec or other times
		isVibratorON = TRUE;

		hrtimer_start(&vibe_timer,
			      ktime_set(val / 1000, (val % 1000) * 1000000),
			      HRTIMER_MODE_REL);
	}
	spin_unlock_irqrestore(&vibe_lock, flags);	

	schedule_work(&vibrator_work);
}

static int qci_vibrator_get_time(struct timed_output_dev *dev)
{
	if (hrtimer_active(&vibe_timer)) {
		ktime_t r = hrtimer_get_remaining(&vibe_timer);
		return r.tv.sec * 1000 + r.tv.nsec / 1000000;
	} else
		return 0;
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	isVibratorON = FALSE;
	schedule_work(&vibrator_work);
	return HRTIMER_NORESTART;
}


/**************************G-Sensor*******************************************/
#include <linux/i2c.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/kobject.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/poll.h>  /*for copy_to_user()*/
#include <linux/i2c-dev.h>

static int __devinit kxte9_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int __devexit kxte9_remove(struct i2c_client *client);	
static ssize_t gsensor_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf);
		
static const struct i2c_device_id kxte9_id[] = {
	{ "kxte9", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, kxte9_id);

#define GSENSOR_DEV_MINOR      	MISC_DYNAMIC_MINOR
#define GSENSOR_DEV_NAME 		"kxte9"
#define GSENSOR_DRV_NAME 		"qci_gsensor"

#define GSENSOR_ENABLE 		1
#define GSENSOR_DISABLE 		0
#define GSENSOR_BUFSIZE 		32

/* General structure to hold the driver data */
struct gsensor_drv_data {
	struct i2c_client *client;
	struct file_operations gsensor_ops;
	struct miscdevice gsensor_dev;
	struct i2c_client *i2c;	

	struct mutex lock;
	char	gsensor_data[GSENSOR_BUFSIZE];
	
};

static struct i2c_driver kxte9_driver = {
	.driver = {
		.name = GSENSOR_DEV_NAME,
	},
	
	.probe = kxte9_probe,
	.remove = kxte9_remove,
	.id_table = kxte9_id,
};

static int gsensor_open( struct inode *inode, struct file  *pfile ){
	printk( KERN_INFO " [G-sensor] open \n");
	return 0;
}

static int gsensor_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg) {
	//printk(KERN_INFO "[G-sensor] ioctl, cmd=0x%02x, arg=0x%02lx\n", cmd, arg);
	return 0;
}

static ssize_t gsensor_read (struct file *pfile, char __user *userbuf, size_t count, loff_t *offset) 
{
	struct gsensor_drv_data* pData = container_of(pfile->f_op,  struct gsensor_drv_data, gsensor_ops);
	char *tmp;
	int ret = 0;	

	//prevent i2c busy
	if ( isVibratorON )
		return 0;

	if ( count > GSENSOR_BUFSIZE )
		count = GSENSOR_BUFSIZE;

	tmp = kmalloc(count,GFP_KERNEL);
	if (tmp==NULL)
		return -ENOMEM;

	//printk(KERN_INFO "G-sensor: reading %zu bytes.\n", count);

	mutex_lock(&pData->lock);
	ret = i2c_master_recv(pData->i2c, tmp, count);
	mutex_unlock(&pData->lock);
	
	if (ret >= 0)
		ret = copy_to_user(userbuf, tmp, count)?-EFAULT:ret;

	kfree(tmp);
	return ret;
}

static ssize_t gsensor_write (struct file *pfile, const char __user *userbuf, size_t count, loff_t *offset) 
{
	struct gsensor_drv_data* pData = container_of(pfile->f_op,  struct gsensor_drv_data, gsensor_ops);
	char* tmp;
	int ret = 0;

	tmp = kmalloc(count, GFP_KERNEL);
	if (tmp==NULL) 
		return -ENOMEM;
	
	if (copy_from_user(tmp, userbuf, count)) {
		kfree(tmp);
		printk(KERN_ERR "[G-sensor] Copy from user failed!\n");		
		return -EFAULT;
	}

	//printk(KERN_INFO "G-sensor: writing %zu bytes.\n", count);
	mutex_lock(&pData->lock);
	ret = i2c_master_send(pData->i2c, tmp, count);
	mutex_unlock(&pData->lock);
	
	kfree(tmp);
	
	return ret;
}

static int __devinit kxte9_probe(struct i2c_client *client, const struct i2c_device_id *id)	{
	int ret = -ENOMEM;

	struct gsensor_drv_data *pData = 0;
	struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);	

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_I2C_BLOCK))     {
	   dev_err(&client->dev, "i2c bus does not support g sensor\n");
	   ret = -ENODEV;
	   return ret;
	}	

	//QCOM I2C hardware adapter QCOM I2C bit-banging adapter QCOM I2C chip-enable adapter
	if(NULL == strstr(adapter->name, "MSM I2C adapter"))	{
    	   printk("adapter->name=%s\n", adapter->name);
	   ret = -ENODEV;
	   return ret;
	}	
	
	pData = kzalloc(sizeof(struct gsensor_drv_data), GFP_KERNEL);
	
	if (!pData)
		return ret;

	pData->i2c = client;
	//Data->gsensor_status = GSENSOR_ENABLE;
	
	i2c_set_clientdata(client, pData);
	pData->client = client;
	strlcpy(pData->client->name, GSENSOR_DRV_NAME, I2C_NAME_SIZE);
	client->driver = &kxte9_driver;

	mutex_init(&pData->lock);
	
	pData->gsensor_ops.owner = THIS_MODULE;
	pData->gsensor_ops.open = gsensor_open;
	pData->gsensor_ops.ioctl = gsensor_ioctl;
	pData->gsensor_ops.read = gsensor_read;
	pData->gsensor_ops.write = gsensor_write;
	pData->gsensor_ops.release = NULL/*gsensor_close*/;
	
	pData->gsensor_dev.minor = GSENSOR_DEV_MINOR;
	pData->gsensor_dev.name  = GSENSOR_DEV_NAME;
	pData->gsensor_dev.fops  = &pData->gsensor_ops;

	ret = misc_register(&pData->gsensor_dev);
	if (ret)
	{
		printk( KERN_ERR "[G-sensor] Misc device register failed\n");
		goto misc_register_fail;
	}

	printk( KERN_INFO " [G-sensor] Probe successful \n");			
	return 0;


misc_register_fail:
	misc_deregister(&pData->gsensor_dev);
	kfree(pData);
	return ret;

}

static int __devexit kxte9_remove(struct i2c_client *client)
{
	struct gsensor_drv_data *pData;
	
	pData = i2c_get_clientdata(client);
	misc_deregister(&pData->gsensor_dev);
	kfree(pData);
	return 0;	
}

/*********************************************************************
 *		Initialisation
 *********************************************************************/
static struct timed_output_dev qci_vibrator = {
	.name = "vibrator",
	.enable = qci_vibrator_enable,
	.get_time = qci_vibrator_get_time,
};


static struct platform_device *vib_pdev;

static int __init qci_vib_init(void)
{
	int ret = 0;
	printk(KERN_INFO "vibrator_init!\n");

	vib_pdev = platform_device_register_simple("vibrator", 0, NULL, 0);
	
	INIT_WORK(&vibrator_work, update_vibrator);

	hrtimer_init(&vibe_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	vibe_timer.function = vibrator_timer_func;

	ret = timed_output_dev_register(&qci_vibrator);
	if (ret)
		goto failed;
		
	printk("[qci_kxte9] i2c_add_driver()\n");	
	ret = i2c_add_driver(&kxte9_driver);
	if (ret)	
	        goto failed;

	printk(KERN_INFO "vib _success\n");
	return ret;

failed:
	printk(KERN_ERR "vib failed\n");
	platform_device_unregister(vib_pdev);
	return ret;
}

static void __exit qci_vib_exit(void)
{
	timed_output_dev_unregister(&qci_vibrator);			

	platform_device_unregister(vib_pdev);
	printk(KERN_INFO "vib __exit() \n");
	
	i2c_del_driver(&kxte9_driver);	
}

module_init(qci_vib_init);
module_exit(qci_vib_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Vibrator driver for Quanta machine");
