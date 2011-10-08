/* Copyright (c) 2009, Code Aurora Forum. All rights reserved.
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


#include <linux/earlysuspend.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#include <asm/atomic.h>

#include <mach/msm_rpcrouter.h>
#include <mach/msm_battery.h>

#include <mach/rpc_hsusb.h>
#include <linux/timer.h>

#include <linux/i2c.h>
#include <mach/qci_gbattery.h>

#define GBATT_TIME_L 300000
#define GBATT_TIME 30000
#define GBATT_TIME_CHG 10000

#define BATTERY_RPC_PROG	0x30000089
#define BATTERY_RPC_VERS	0x00010001

#define BATTERY_RPC_CB_PROG	0x31000089
#define BATTERY_RPC_CB_VERS	0x00010003

#define CHG_RPC_PROG		0x3000001a
#define CHG_RPC_VERS		0x00010003




#define BATTERY_REGISTER_PROC                          	2
#define BATTERY_GET_CLIENT_INFO_PROC                   	3
#define BATTERY_MODIFY_CLIENT_PROC                     	4
#define BATTERY_DEREGISTER_CLIENT_PROC			5
#define BATTERY_SERVICE_TABLES_PROC                    	6
#define BATTERY_IS_SERVICING_TABLES_ENABLED_PROC       	7
#define BATTERY_ENABLE_TABLE_SERVICING_PROC            	8
#define BATTERY_DISABLE_TABLE_SERVICING_PROC           	9
#define BATTERY_READ_PROC                              	10
#define BATTERY_MIMIC_LEGACY_VBATT_READ_PROC           	11
#define BATTERY_ENABLE_DISABLE_FILTER_PROC 		14

#define VBATT_FILTER			2

#define BATTERY_CB_TYPE_PROC 		1
#define BATTERY_CB_ID_ALL_ACTIV       	1
#define BATTERY_CB_ID_LOW_VOL		2


#define BATTERY_MIN            	3500
#define BATTERY_MAX           	4350
#define BATTERY_LOW            	2800
#define BATTERY_HIGH           	4350
#define MSM_BATT_TIME		2000

#define ONCRPC_CHG_IS_CHARGING_PROC 		2
#define ONCRPC_CHG_IS_CHARGER_VALID_PROC 	3
#define ONCRPC_CHG_IS_BATTERY_VALID_PROC 	4
#define ONCRPC_CHG_UI_EVENT_READ_PROC 		5
#define ONCRPC_CHG_GET_GENERAL_STATUS_PROC 	12
#define ONCRPC_CHARGER_API_VERSIONS_PROC 	0xffffffff

#define CHARGER_API_VERSION  			0x00010003

#define DEFAULT_CHARGER_API_VERSION		0x00010001


#define BATT_RPC_TIMEOUT    10000

#define INVALID_BATT_HANDLE    -1

#define RPC_TYPE_REQ     0
#define RPC_TYPE_REPLY   1
#define RPC_REQ_REPLY_COMMON_HEADER_SIZE   (3 * sizeof(uint32_t))


#define SUSPEND_EVENT		(1UL << 0)
#define RESUME_EVENT		(1UL << 1)
#define CLEANUP_EVENT		(1UL << 2)


#define DPRINTK(fmt,args...)	do {} while(0)

#define DBG(x...) do {} while (0)



#ifdef CONFIG_HAS_EARLYSUSPEND
struct wake_lock g_batt_wlock;
static unsigned long g_batt_suspended=0;
#endif

		static unsigned long gbatt_update_time;
		static int get_batt_flag=1, first_get=0;
		struct work_struct gbatt_workqueue;
		struct mutex workqueue_lock;
		static unsigned long gbatt_statistic=0;
	
		struct i2c_client* gauge_i2c = NULL;
		u8  cmd[8];
		u8  rsp[13];

		static const struct i2c_device_id gbatt_id[] = {
			{ "gbattery", 0 },
			{ }
		};

		static struct i2c_driver gauge_i2c_driver = {
			.probe = gauge_probe,
			.remove = gauge_remove,
			.id_table = gbatt_id,
			.driver = {
				   .name = "gbattery",
				   },
			.class		= I2C_CLASS_HWMON,
		};

		static int gauge_probe(struct i2c_client *client, const struct i2c_device_id *id)
		{
			int err = 0;
			struct i2c_adapter *adapter = to_i2c_adapter(client->dev.parent);
			
		  	i2c_check_functionality(client->adapter, I2C_FUNC_I2C|I2C_FUNC_SMBUS_BYTE);									     


		  	if(NULL == strstr(adapter->name, "MSM I2C adapter"))
			{
				err = -ENODEV;
				goto exit;
			}
			
			gauge_i2c = kzalloc(sizeof(struct i2c_client), GFP_KERNEL);
			if (!gauge_i2c) {
				err = -ENOMEM;
				goto exit;
			}
			memcpy(gauge_i2c, client, sizeof(struct i2c_client));
			i2c_set_clientdata(client, gauge_i2c);
			

		exit:
			return err;	
			
		}

		static int gauge_remove(struct i2c_client *client)
		{
			kfree(i2c_get_clientdata(client));
			return 0;
		}
				
		static int gauge_read(u8 cmd_size, u8 *cmd, u8 reply_size, u8 *reply)
		{
		    int ret=0;

		    if(cmd_size && cmd!=NULL)
		    {		    
		    	ret=gauge_cmd(gauge_i2c,cmd_size, cmd);
		    }

		    if(reply_size && reply!=NULL)
		    {
		        ret  |= gauge_reply(gauge_i2c,reply_size, reply);
		    }

		     return ret;
		}

		static int gauge_cmd(struct i2c_client *client, u8 number_bytes, u8 *data_out)
		{
			int ret;

			if ((ret = i2c_master_send(client, data_out, number_bytes)) == number_bytes)
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}

		static int gauge_reply(struct i2c_client *client, u8 number_bytes, u8 *data_in)
		{
			int ret;
			
			if ((ret=i2c_master_recv(client, data_in, number_bytes)) == number_bytes)
			{
				return 0;
			}
			else
			{
				gbatt_statistic++;
				return -1;
			}
		}

		unsigned int gauge_get_battery_rsoc(void)
		{
			unsigned int rsoc;
			cmd[0] = GAUGE_SOC;
		    	gauge_read(1, cmd, 1, rsp);
		    	rsoc = (unsigned int) rsp[0];
			
			return rsoc;
		}

		unsigned int gauge_get_battery_voltage(void)
		{
			unsigned int battery_value;
			cmd[0] = GAUGE_VOTL_L;
		    	gauge_read(1, cmd, 1, rsp);
		    	battery_value = (unsigned int) rsp[0];
		    
		    	cmd[0] = GAUGE_VOTL_H;
		    	gauge_read(1, cmd, 1, rsp);
			battery_value = battery_value | ((unsigned int) rsp[0] << 8); 
			
			return battery_value;
		}

		int gauge_get_battery_temperature(void)
		{
			int die_tempeture;
			long int tmp;
			cmd[0] = GAUGE_TEMP_L;
		    	gauge_read(1, cmd, 1, rsp);
		    	die_tempeture = (int) rsp[0];
		    
		    	cmd[0] = GAUGE_TEMP_H;
		    	gauge_read(1, cmd, 1, rsp);
		    	tmp = die_tempeture = die_tempeture | (( int) rsp[0] << 8);  

			tmp = tmp - 2731;
			die_tempeture = tmp;					

			return die_tempeture;;
		}


		 s16 gauge_get_battery_current(void)
		{
			s16 bat_current;
			cmd[0] = GAUGE_AI_L;
		    	gauge_read(1, cmd, 1, rsp);
			bat_current=  rsp[0];
		    	
		    	cmd[0] = GAUGE_AI_H;
		    	gauge_read(1, cmd, 1, rsp);
			bat_current = (s16)( bat_current |( rsp[0] << 8) ); 	

			return bat_current;
		}

		static int  gbatt_init(void)
		{
			return i2c_add_driver(&gauge_i2c_driver);
		}

		 static void  gbatt_exit(void)
		{
			i2c_del_driver(&gauge_i2c_driver);
		}

enum {
	BATTERY_REGISTRATION_SUCCESSFUL = 0,
	BATTERY_DEREGISTRATION_SUCCESSFUL = BATTERY_REGISTRATION_SUCCESSFUL,
	BATTERY_MODIFICATION_SUCCESSFUL = BATTERY_REGISTRATION_SUCCESSFUL,
	BATTERY_INTERROGATION_SUCCESSFUL = BATTERY_REGISTRATION_SUCCESSFUL,
	BATTERY_CLIENT_TABLE_FULL = 1,
	BATTERY_REG_PARAMS_WRONG = 2,
	BATTERY_DEREGISTRATION_FAILED = 4,
	BATTERY_MODIFICATION_FAILED = 8,
	BATTERY_INTERROGATION_FAILED = 16,
	BATTERY_SET_FILTER_FAILED         = 32,
	BATTERY_ENABLE_DISABLE_INDIVIDUAL_CLIENT_FAILED  = 64,
	BATTERY_LAST_ERROR = 128,
};

enum {
	BATTERY_VOLTAGE_UP = 0,
	BATTERY_VOLTAGE_DOWN,
	BATTERY_VOLTAGE_ABOVE_THIS_LEVEL,
	BATTERY_VOLTAGE_BELOW_THIS_LEVEL,
	BATTERY_VOLTAGE_LEVEL,
	BATTERY_ALL_ACTIVITY,
	VBATT_CHG_EVENTS,
	BATTERY_VOLTAGE_UNKNOWN,
};

enum {
	CHG_UI_EVENT_IDLE,
	CHG_UI_EVENT_NO_POWER,
	CHG_UI_EVENT_VERY_LOW_POWER,
	CHG_UI_EVENT_LOW_POWER,
	CHG_UI_EVENT_NORMAL_POWER,
	CHG_UI_EVENT_DONE,
	CHG_UI_EVENT_INVALID,
	CHG_UI_EVENT_MAX32 = 0x7fffffff
};

enum chg_charger_status_type {
    CHARGER_STATUS_GOOD,
    CHARGER_STATUS_BAD,
    CHARGER_STATUS_WEAK,
    CHARGER_STATUS_INVALID
};

static char *charger_status[] = {
	"good", "bad", "weak", "invalid"
};

enum chg_charger_hardware_type {
    CHARGER_TYPE_NONE,
    CHARGER_TYPE_WALL,
    CHARGER_TYPE_USB_PC,
    CHARGER_TYPE_USB_WALL,
    CHARGER_TYPE_USB_CARKIT,
    CHARGER_TYPE_INVALID
};

static char *charger_type[] = {
	"No charger", "wall", "USB PC", "USB wall", "USB car kit",
	"invalid charger"
};


enum chg_battery_status_type {
    BATTERY_STATUS_GOOD,
    BATTERY_STATUS_BAD_TEMP,
    BATTERY_STATUS_BAD,
    BATTERY_STATUS_INVALID
};

static char *battery_status[] = {
	"good ", "bad temperature", "bad", "invalid"
};


enum chg_battery_level_type {
    BATTERY_LEVEL_DEAD,
    BATTERY_LEVEL_WEAK,
    BATTERY_LEVEL_GOOD,
    BATTERY_LEVEL_FULL,
    BATTERY_LEVEL_INVALID
};

static char *battery_level[] = {
	"dead", "weak", "good", "full", "invalid"
};

enum {
	CHG_CMD_DISABLE,
	CHG_CMD_ENABLE,
	CHG_CMD_INVALID,
	CHG_CMD_MAX32 = 0x7fffffff
};

struct batt_client_registration_req {

	struct rpc_request_hdr hdr;
	u32 desired_batt_voltage;
	u32 voltage_direction;
	u32 batt_cb_id;
	u32 cb_data;
	u32 more_data;
	u32 batt_error;
};

struct batt_client_registration_rep {
	struct rpc_reply_hdr hdr;
	u32 batt_clnt_handle;
};

struct rpc_reply_batt_chg {
	struct rpc_reply_hdr hdr;
	u32 	more_data;

	u32	charger_status;
	u32	charger_type;
	u32	battery_status;
	u32	battery_level;
	u32  battery_voltage;
	u32	battery_temp;
	u32	battery_cap;
};

static struct rpc_reply_batt_chg rep_batt_chg;

struct qci_battery_info {

	u32 voltage_max_design;
	u32 voltage_min_design;
	u32 chg_api_version;
	u32 batt_technology;
	u32 avail_chg_sources;
	u32 current_chg_source;
	u32 batt_status;
	u32 batt_health;
	u32 charger_valid;
	u32 batt_valid;
	u32 batt_capacity;
	s32 charger_status;
	s32 charger_type;
	s32 battery_status;
	s32 battery_level;
	s32 battery_voltage;
	s32 battery_temp;
	s32 battery_current;
	s32 battery_ot;
	u32(*calculate_capacity) (u32 voltage);
	s32 batt_handle;
	spinlock_t lock;

	struct power_supply *msm_psy_ac;
	struct power_supply *msm_psy_usb;
	struct power_supply *msm_psy_batt;

	struct msm_rpc_endpoint *batt_ep;
	struct msm_rpc_endpoint *chg_ep;

	struct workqueue_struct *msm_batt_wq;

	struct task_struct *cb_thread;

	wait_queue_head_t wait_q;

	struct early_suspend early_suspend;

	atomic_t handle_event;
	atomic_t event_handled;

	u32 type_of_event;
	uint32_t vbatt_rpc_req_xid;
};

static void batt_wait_for_batt_chg_event(struct work_struct *work);

static DECLARE_WORK(msm_batt_cb_work, batt_wait_for_batt_chg_event);

static int qci_batt_cleanup(void);

static struct qci_battery_info qci_batt_info = {
	.batt_handle = INVALID_BATT_HANDLE,
	.charger_status = -1,
	.charger_type = -1,
	.battery_status = -1,
	.battery_level = -1,
	.battery_voltage = -1,
	.battery_temp = -1,
	.battery_current = -1,
	.battery_ot = -1,
};

static enum power_supply_property msm_power_props[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static char *msm_power_supplied_to[] = {
	"battery",
};

static int msm_power_get_property(struct power_supply *psy,
				  enum power_supply_property psp,
				  union power_supply_propval *val)
{
	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:

		if (psy->type == POWER_SUPPLY_TYPE_MAINS) {

			val->intval = qci_batt_info.current_chg_source & AC_CHG
			    ? 1 : 0;

		}

		if (psy->type == POWER_SUPPLY_TYPE_USB) {

			val->intval = qci_batt_info.current_chg_source & USB_CHG
			    ? 1 : 0;

		}

		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static struct power_supply msm_psy_ac = {
	.name = "ac",
	.type = POWER_SUPPLY_TYPE_MAINS,
	.supplied_to = msm_power_supplied_to,
	.num_supplicants = ARRAY_SIZE(msm_power_supplied_to),
	.properties = msm_power_props,
	.num_properties = ARRAY_SIZE(msm_power_props),
	.get_property = msm_power_get_property,
};

static struct power_supply msm_psy_usb = {
	.name = "usb",
	.type = POWER_SUPPLY_TYPE_USB,
	.supplied_to = msm_power_supplied_to,
	.num_supplicants = ARRAY_SIZE(msm_power_supplied_to),
	.properties = msm_power_props,
	.num_properties = ARRAY_SIZE(msm_power_props),
	.get_property = msm_power_get_property,
};

static enum power_supply_property msm_batt_power_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CAPACITY,
};
	
static int polling = 0;
struct timer_list polling_timer;
u32 g_polling_val=0;


static void batt_update_psy_status_v0(void);
static void batt_update_psy_status_v1(void);

static int batt_power_get_property(struct power_supply *psy,
				       enum power_supply_property psp,
				       union power_supply_propval *val)
{
	switch (psp) {

	case POWER_SUPPLY_PROP_STATUS:
		val->intval = qci_batt_info.batt_status;
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		val->intval = qci_batt_info.batt_health;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		val->intval = qci_batt_info.batt_valid;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
		val->intval = qci_batt_info.batt_technology;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		val->intval = qci_batt_info.voltage_max_design;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MIN_DESIGN:
		val->intval = qci_batt_info.voltage_min_design;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		val->intval = qci_batt_info.battery_voltage;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		val->intval = qci_batt_info.batt_capacity;
		break;
	case POWER_SUPPLY_PROP_CURRENT_NOW:
		val->intval = qci_batt_info.battery_current;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		val->intval = qci_batt_info.battery_temp;
		break;
	
	default:
		return -EINVAL;
	}
	return 0;
}

static struct power_supply msm_psy_batt = {
	.name = "battery",
	.type = POWER_SUPPLY_TYPE_BATTERY,
	.properties = msm_batt_power_props,
	.num_properties = ARRAY_SIZE(msm_batt_power_props),
	.get_property = batt_power_get_property,
};


static int batt_get_batt_chg_status_v1(void)
{
	int rc ;
	struct rpc_req_batt_chg {
		struct rpc_request_hdr hdr;
		u32 more_data;
	} req_batt_chg;

	req_batt_chg.more_data = cpu_to_be32(1);

	memset(&rep_batt_chg, 0, sizeof(rep_batt_chg));

	rc = msm_rpc_call_reply(qci_batt_info.chg_ep,
				ONCRPC_CHG_GET_GENERAL_STATUS_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_batt_chg, sizeof(rep_batt_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		return rc;
	} else if (be32_to_cpu(rep_batt_chg.more_data)) {

		rep_batt_chg.charger_status =
			be32_to_cpu(rep_batt_chg.charger_status);

		rep_batt_chg.charger_type =
			be32_to_cpu(rep_batt_chg.charger_type);

		rep_batt_chg.battery_status =
			be32_to_cpu(rep_batt_chg.battery_status);

		rep_batt_chg.battery_level =
			be32_to_cpu(rep_batt_chg.battery_level);

		rep_batt_chg.battery_voltage =
			be32_to_cpu(rep_batt_chg.battery_voltage);

		rep_batt_chg.battery_temp =
			be32_to_cpu(rep_batt_chg.battery_temp);

		rep_batt_chg.battery_cap =
			be32_to_cpu(rep_batt_chg.battery_cap);


	} else {
		return -EIO;
	}

	return 0;
}

static void batt_update_psy_status_v1(void)
{

	batt_get_batt_chg_status_v1();

	if (qci_batt_info.charger_status == rep_batt_chg.charger_status &&
		qci_batt_info.charger_type == rep_batt_chg.charger_type &&
		qci_batt_info.battery_status ==  rep_batt_chg.battery_status &&
		qci_batt_info.battery_level ==  rep_batt_chg.battery_level &&
		qci_batt_info.battery_voltage  == rep_batt_chg.battery_voltage
		&& qci_batt_info.battery_temp ==  rep_batt_chg.battery_temp) {

		return;
	}

	qci_batt_info.battery_voltage  	= 	rep_batt_chg.battery_voltage;
	qci_batt_info.battery_temp 	=	rep_batt_chg.battery_temp;
	qci_batt_info.battery_level 	=  	rep_batt_chg.battery_level;
	qci_batt_info.charger_status 	= 	rep_batt_chg.charger_status;


	if (rep_batt_chg.battery_status != BATTERY_STATUS_INVALID) {

		qci_batt_info.batt_valid = 1;
		qci_batt_info.battery_status 	=  	rep_batt_chg.battery_status;


		if (qci_batt_info.battery_voltage > qci_batt_info.voltage_max_design) {

			qci_batt_info.batt_health =   POWER_SUPPLY_HEALTH_OVERVOLTAGE;
			qci_batt_info.batt_status = POWER_SUPPLY_STATUS_NOT_CHARGING;

		} else if( qci_batt_info.current_chg_source & AC_CHG || qci_batt_info.current_chg_source & USB_CHG){
		
			qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;
			qci_batt_info.batt_status =POWER_SUPPLY_STATUS_CHARGING;
			
			#ifdef CONFIG_HAS_EARLYSUSPEND
				wake_lock(&g_batt_wlock);
			#endif
			
		} else{
		
			qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;
			qci_batt_info.batt_status =POWER_SUPPLY_STATUS_DISCHARGING;
			
			#ifdef CONFIG_HAS_EARLYSUSPEND
				wake_lock_timeout(&g_batt_wlock, HZ / 2);
			#endif
			
		}

		qci_batt_info.batt_capacity = rep_batt_chg.battery_cap;
			
	
	} else {
	
		qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;
		qci_batt_info.batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
		qci_batt_info.batt_capacity = 0;
	}


	if (qci_batt_info.charger_type != rep_batt_chg.charger_type) {

		qci_batt_info.charger_type = rep_batt_chg.charger_type ;

		if (qci_batt_info.charger_type == CHARGER_TYPE_WALL) {

			qci_batt_info.current_chg_source &= ~USB_CHG;
			qci_batt_info.current_chg_source |= AC_CHG;

			qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;
			qci_batt_info.batt_status =POWER_SUPPLY_STATUS_CHARGING;

			power_supply_changed(&msm_psy_ac);

		} else if (qci_batt_info.charger_type ==
				CHARGER_TYPE_USB_WALL ||
				qci_batt_info.charger_type ==
				CHARGER_TYPE_USB_PC) {

			qci_batt_info.current_chg_source &= ~AC_CHG;
			qci_batt_info.current_chg_source |= USB_CHG;

			qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;
			qci_batt_info.batt_status =POWER_SUPPLY_STATUS_CHARGING;

			power_supply_changed(&msm_psy_usb);

		} else {


			qci_batt_info.batt_status =
				POWER_SUPPLY_STATUS_DISCHARGING;

			if (qci_batt_info.current_chg_source & AC_CHG) {

				qci_batt_info.current_chg_source &= ~AC_CHG;

				power_supply_changed(&msm_psy_ac);

			} else if (qci_batt_info.current_chg_source & USB_CHG) {

				qci_batt_info.current_chg_source &= ~USB_CHG;

				power_supply_changed(&msm_psy_usb);
			} else
				power_supply_changed(&msm_psy_batt);
		}
	} else
		power_supply_changed(&msm_psy_batt);



}


static int batt_get_batt_chg_status_v0(u32 *batt_charging,
					u32 *charger_valid,
					u32 *chg_batt_event)
{
	struct rpc_request_hdr req_batt_chg;

	struct rpc_reply_batt_volt {
		struct rpc_reply_hdr hdr;
		u32 voltage;
	} rep_volt;

	struct rpc_reply_chg_reply {
		struct rpc_reply_hdr hdr;
		u32 chg_batt_data;
	} rep_chg;

	int rc;
	*batt_charging = 0;
	*chg_batt_event = CHG_UI_EVENT_INVALID;
	*charger_valid = 0;

	rc = msm_rpc_call_reply(qci_batt_info.batt_ep,
				BATTERY_READ_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_volt, sizeof(rep_volt),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		return rc;
	}
	qci_batt_info.battery_voltage = be32_to_cpu(rep_volt.voltage);

	rc = msm_rpc_call_reply(qci_batt_info.chg_ep,
				ONCRPC_CHG_IS_CHARGING_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_chg, sizeof(rep_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		return rc;
	}
	*batt_charging = be32_to_cpu(rep_chg.chg_batt_data);

	rc = msm_rpc_call_reply(qci_batt_info.chg_ep,
				ONCRPC_CHG_IS_CHARGER_VALID_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_chg, sizeof(rep_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		return rc;
	}
	*charger_valid = be32_to_cpu(rep_chg.chg_batt_data);

	rc = msm_rpc_call_reply(qci_batt_info.chg_ep,
				ONCRPC_CHG_IS_BATTERY_VALID_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_chg, sizeof(rep_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		return rc;
	}
	qci_batt_info.batt_valid = be32_to_cpu(rep_chg.chg_batt_data);

	rc = msm_rpc_call_reply(qci_batt_info.chg_ep,
				ONCRPC_CHG_UI_EVENT_READ_PROC,
				&req_batt_chg, sizeof(req_batt_chg),
				&rep_chg, sizeof(rep_chg),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		return rc;
	}
	*chg_batt_event = be32_to_cpu(rep_chg.chg_batt_data);

	return 0;
}

static void batt_update_psy_status_v0(void)
{
	u32 batt_charging = 0;
	u32 chg_batt_event = CHG_UI_EVENT_INVALID;
	u32 charger_valid = 0;

	batt_get_batt_chg_status_v0(&batt_charging, &charger_valid,
				     &chg_batt_event);

	if (qci_batt_info.charger_valid != charger_valid) {

		qci_batt_info.charger_valid = charger_valid;
		if (qci_batt_info.charger_valid)
			qci_batt_info.current_chg_source |= USB_CHG;
		else
			qci_batt_info.current_chg_source &= ~USB_CHG;
		power_supply_changed(&msm_psy_usb);
	}

	if (qci_batt_info.batt_valid) {

		if (qci_batt_info.battery_voltage >
		    qci_batt_info.voltage_max_design)
			qci_batt_info.batt_health =
			    POWER_SUPPLY_HEALTH_OVERVOLTAGE;

		else if (qci_batt_info.battery_voltage
			 < qci_batt_info.voltage_min_design)
			qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_DEAD;
		else
			qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;

		if (batt_charging && qci_batt_info.charger_valid)
			qci_batt_info.batt_status =
			    POWER_SUPPLY_STATUS_CHARGING;
		else if (!batt_charging)
			qci_batt_info.batt_status =
			    POWER_SUPPLY_STATUS_DISCHARGING;

		if (chg_batt_event == CHG_UI_EVENT_DONE)
			qci_batt_info.batt_status = POWER_SUPPLY_STATUS_FULL;

		qci_batt_info.batt_capacity =
		    qci_batt_info.calculate_capacity(
				    qci_batt_info.battery_voltage);

	} else {
		qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_UNKNOWN;
		qci_batt_info.batt_status = POWER_SUPPLY_STATUS_UNKNOWN;
		qci_batt_info.batt_capacity = 0;
	}

	power_supply_changed(&msm_psy_batt);
}
static void gbatt_update_psy_status(void)
{

	batt_get_batt_chg_status_v1();

	if (qci_batt_info.charger_type != rep_batt_chg.charger_type) {

		qci_batt_info.charger_type = rep_batt_chg.charger_type ;

		if (qci_batt_info.charger_type == CHARGER_TYPE_WALL) {

			qci_batt_info.current_chg_source &= ~USB_CHG;
			qci_batt_info.current_chg_source |= AC_CHG;

			qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;
			qci_batt_info.batt_status =POWER_SUPPLY_STATUS_CHARGING;

			power_supply_changed(&msm_psy_ac);

		} else if (qci_batt_info.charger_type ==
				CHARGER_TYPE_USB_WALL ||
				qci_batt_info.charger_type ==
				CHARGER_TYPE_USB_PC) {

			qci_batt_info.current_chg_source &= ~AC_CHG;
			qci_batt_info.current_chg_source |= USB_CHG;


			qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;
			qci_batt_info.batt_status =POWER_SUPPLY_STATUS_CHARGING;

			power_supply_changed(&msm_psy_usb);

		} else {

			qci_batt_info.batt_status = POWER_SUPPLY_STATUS_DISCHARGING;

			if (qci_batt_info.current_chg_source & AC_CHG) {

				qci_batt_info.current_chg_source &= ~AC_CHG;

				power_supply_changed(&msm_psy_ac);

			} else if (qci_batt_info.current_chg_source & USB_CHG) {

				qci_batt_info.current_chg_source &= ~USB_CHG;

				power_supply_changed(&msm_psy_usb);
				
			}else{
			
				power_supply_changed(&msm_psy_batt);
				
			}
		}
	}
	
	qci_batt_info.charger_status 	= 	rep_batt_chg.charger_status;
	qci_batt_info.charger_type 	= 	rep_batt_chg.charger_type;
	qci_batt_info.battery_status 	=  	rep_batt_chg.battery_status;


}

static void get_gbatt_info(void){
	
	
	qci_batt_info.battery_voltage  = gauge_get_battery_voltage();
	qci_batt_info.battery_temp = gauge_get_battery_temperature();
	qci_batt_info.batt_capacity = gauge_get_battery_rsoc();
	qci_batt_info.battery_current = gauge_get_battery_current();

	get_batt_flag = 1;
	first_get =1;
	gbatt_update_time=jiffies;

	if ( (qci_batt_info.battery_temp > 470 || qci_batt_info.battery_temp < -50) 
		&&( qci_batt_info.current_chg_source > 0)
		&& qci_batt_info.battery_ot != 1)
	{
		msm_chg_usb_i_is_available(231);
		qci_batt_info.battery_ot = 1;
	}
	else if ( -40 <qci_batt_info.battery_temp && qci_batt_info.battery_temp < 460
		&&( qci_batt_info.current_chg_source > 0)
		&& qci_batt_info.battery_ot != 0)
	{
		msm_chg_usb_i_is_available(1231);
		qci_batt_info.battery_ot = 0;
	}

	return;
}


static void gbatt_work_f(struct work_struct *work)
{
	mutex_lock(&workqueue_lock);
	get_gbatt_info();
	power_supply_changed(&msm_psy_batt);
	mutex_unlock(&workqueue_lock);
}


void batt_polling_timer_func(unsigned long unused)
{

	schedule_work(&gbatt_workqueue);

	if( qci_batt_info.current_chg_source > 0 ){
	
		g_polling_val = GBATT_TIME_CHG;	

	}else if(qci_batt_info.batt_capacity>=15){
		
		g_polling_val = GBATT_TIME_L;
		
	}else{
	
		g_polling_val = GBATT_TIME;
		
	}
	
	mod_timer(&polling_timer, jiffies + msecs_to_jiffies(g_polling_val));

}

static int batt_register(u32 desired_batt_voltage,
			     u32 voltage_direction, u32 batt_cb_id, u32 cb_data)
{
	struct batt_client_registration_req req;
	struct batt_client_registration_rep rep;
	int rc;

	req.desired_batt_voltage = cpu_to_be32(desired_batt_voltage);
	req.voltage_direction = cpu_to_be32(voltage_direction);
	req.batt_cb_id = cpu_to_be32(batt_cb_id);
	req.cb_data = cpu_to_be32(cb_data);
	req.more_data = cpu_to_be32(1);
	req.batt_error = cpu_to_be32(0);

	rc = msm_rpc_call_reply(qci_batt_info.batt_ep,
				BATTERY_REGISTER_PROC, &req,
				sizeof(req), &rep, sizeof(rep),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {

		return rc;
	} else {
		rc = be32_to_cpu(rep.batt_clnt_handle);
		return rc;
	}
}

static int batt_modify_client(u32 client_handle, u32 desired_batt_voltage,
	     u32 voltage_direction, u32 batt_cb_id, u32 cb_data)
{
	int rc;

	struct batt_modify_client_req {
		struct rpc_request_hdr hdr;

		u32 client_handle;

		u32 desired_batt_voltage;

		u32 voltage_direction;

		u32 batt_cb_id;

		u32 cb_data;
	} req;

	req.client_handle = cpu_to_be32(client_handle);
	req.desired_batt_voltage = cpu_to_be32(desired_batt_voltage);
	req.voltage_direction = cpu_to_be32(voltage_direction);
	req.batt_cb_id = cpu_to_be32(batt_cb_id);
	req.cb_data = cpu_to_be32(cb_data);

	msm_rpc_setup_req(&req.hdr, BATTERY_RPC_PROG, BATTERY_RPC_VERS,
			 BATTERY_MODIFY_CLIENT_PROC);

	qci_batt_info.vbatt_rpc_req_xid = req.hdr.xid;

	rc = msm_rpc_write(qci_batt_info.batt_ep, &req, sizeof(req));

	if (rc < 0) {
		return rc;
	}

	return 0;
}

static int batt_deregister(u32 handle)
{
	int rc;
	struct batt_client_deregister_req {
		struct rpc_request_hdr req;
		s32 handle;
	} batt_deregister_rpc_req;

	struct batt_client_deregister_reply {
		struct rpc_reply_hdr hdr;
		u32 batt_error;
	} batt_deregister_rpc_reply;

	batt_deregister_rpc_req.handle = cpu_to_be32(handle);
	batt_deregister_rpc_reply.batt_error = cpu_to_be32(BATTERY_LAST_ERROR);

	rc = msm_rpc_call_reply(qci_batt_info.batt_ep,
				BATTERY_DEREGISTER_CLIENT_PROC,
				&batt_deregister_rpc_req,
				sizeof(batt_deregister_rpc_req),
				&batt_deregister_rpc_reply,
				sizeof(batt_deregister_rpc_reply),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		return rc;
	}

	if (be32_to_cpu(batt_deregister_rpc_reply.batt_error) !=
			BATTERY_DEREGISTRATION_SUCCESSFUL) {
		return -EIO;
	}
	return 0;
}

static int  batt_handle_suspend(void)
{
	int rc;

	if (qci_batt_info.batt_handle != INVALID_BATT_HANDLE) {

		rc = batt_modify_client(qci_batt_info.batt_handle,
				BATTERY_LOW, BATTERY_VOLTAGE_BELOW_THIS_LEVEL,
				BATTERY_CB_ID_LOW_VOL, BATTERY_LOW);

		if (rc < 0) {
			return rc;
		}
	}

	return 0;
}

static int  batt_handle_resume(void)
{
	int rc;

	if (qci_batt_info.batt_handle != INVALID_BATT_HANDLE) {

		rc = batt_modify_client(qci_batt_info.batt_handle,
				BATTERY_LOW, BATTERY_ALL_ACTIVITY,
			       BATTERY_CB_ID_ALL_ACTIV, BATTERY_ALL_ACTIVITY);
		if (rc < 0) {
			return rc;
		}
	}
	return 0;
}


static int  batt_handle_event(void)
{
	int rc;

	if (!atomic_read(&qci_batt_info.handle_event)) {
		return 0;
	}

	if (qci_batt_info.type_of_event & SUSPEND_EVENT) {

		rc = batt_handle_suspend();

		return rc;

	} else if (qci_batt_info.type_of_event & RESUME_EVENT) {

		rc = batt_handle_resume();

		return rc;

	} else if (qci_batt_info.type_of_event & CLEANUP_EVENT) {

		return 0;

	} else  {

		return 0;
	}
}


static void batt_handle_vbatt_rpc_reply(struct rpc_reply_hdr *reply)
{

	struct rpc_reply_vbatt_modify_client {
		struct rpc_reply_hdr hdr;
		u32 modify_client_result;
	} *rep_vbatt_modify_client;

	u32 modify_client_result;


	if (reply->xid != qci_batt_info.vbatt_rpc_req_xid) {

		kfree(reply);

		return;
	}
	if (reply->reply_stat != RPCMSG_REPLYSTAT_ACCEPTED) {

		kfree(reply);

		return;
	}

	if (reply->data.acc_hdr.accept_stat != RPC_ACCEPTSTAT_SUCCESS) {

		kfree(reply);

		return;
	}

	rep_vbatt_modify_client =
		(struct rpc_reply_vbatt_modify_client *) reply;

	modify_client_result =
		be32_to_cpu(rep_vbatt_modify_client->modify_client_result);


	kfree(reply);
}

static void batt_wake_up_waiting_thread(u32 event)
{
	qci_batt_info.type_of_event &= ~event;

	atomic_set(&qci_batt_info.handle_event, 0);
	atomic_set(&qci_batt_info.event_handled, 1);
	wake_up(&qci_batt_info.wait_q);
}


static void batt_wait_for_batt_chg_event(struct work_struct *work)
{
	void *rpc_packet;
	struct rpc_request_hdr *req;
	int rpc_packet_type;
	struct rpc_reply_hdr rpc_reply;
	int len;
	unsigned long flags;
	int rc;

	spin_lock_irqsave(&qci_batt_info.lock, flags);
	qci_batt_info.cb_thread = current;
	spin_unlock_irqrestore(&qci_batt_info.lock, flags);

	allow_signal(SIGCONT);

	if (qci_batt_info.chg_api_version >= CHARGER_API_VERSION)
		
				gbatt_update_psy_status();
			
	else
		batt_update_psy_status_v0();

	while (1) {

		rpc_packet = NULL;

		len = msm_rpc_read(qci_batt_info.batt_ep, &rpc_packet, -1, -1);

		if (len == -ERESTARTSYS) {

			flush_signals(current);

			rc = batt_handle_event();

			if (qci_batt_info.type_of_event & CLEANUP_EVENT) {

				batt_wake_up_waiting_thread(CLEANUP_EVENT);
				break;

			} else if (qci_batt_info.type_of_event &
					(SUSPEND_EVENT | RESUME_EVENT)) {

				if (rc < 0) {
					batt_wake_up_waiting_thread(
						SUSPEND_EVENT | RESUME_EVENT);
				}

				continue;
			}
		}


		if (len < 0) {
			continue;
		}

		if (len < RPC_REQ_REPLY_COMMON_HEADER_SIZE) {
			if (!rpc_packet)
				kfree(rpc_packet);
			continue;
		}

		req = (struct rpc_request_hdr *)rpc_packet;

		rpc_packet_type = be32_to_cpu(req->type);

		if (rpc_packet_type == RPC_TYPE_REPLY) {

			if (qci_batt_info.type_of_event &
				(SUSPEND_EVENT | RESUME_EVENT)) {

				batt_handle_vbatt_rpc_reply(rpc_packet);

				batt_wake_up_waiting_thread(
						SUSPEND_EVENT | RESUME_EVENT);

			} else {
				kfree(rpc_packet);
			}
			continue;
		}
		if (rpc_packet_type != RPC_TYPE_REQ) {
			kfree(rpc_packet);
			continue;
		}

		req->type = be32_to_cpu(req->type);
		req->xid = be32_to_cpu(req->xid);
		req->rpc_vers = be32_to_cpu(req->rpc_vers);

		if (req->rpc_vers != 2) {
			kfree(rpc_packet);
			continue;
		}

		req->prog = be32_to_cpu(req->prog);
		if (req->prog != BATTERY_RPC_CB_PROG) {
			kfree(rpc_packet);
			continue;
		}

		req->procedure = be32_to_cpu(req->procedure);

		if (req->procedure != BATTERY_CB_TYPE_PROC) {
			kfree(rpc_packet);
			continue;
		}

		rpc_reply.xid = cpu_to_be32(req->xid);
		rpc_reply.type = cpu_to_be32(RPC_TYPE_REPLY);
		rpc_reply.reply_stat = cpu_to_be32(RPCMSG_REPLYSTAT_ACCEPTED);
		rpc_reply.data.acc_hdr.accept_stat =
		    cpu_to_be32(RPC_ACCEPTSTAT_SUCCESS);
		rpc_reply.data.acc_hdr.verf_flavor = 0;
		rpc_reply.data.acc_hdr.verf_length = 0;

		len = msm_rpc_write(qci_batt_info.batt_ep,
				    &rpc_reply, sizeof(rpc_reply));
		if (len < 0)

		kfree(rpc_packet);

		if (qci_batt_info.chg_api_version >= CHARGER_API_VERSION)

					gbatt_update_psy_status();

		else
			batt_update_psy_status_v0();


		
	}

}

static int batt_send_event(u32 type_of_event)
{
	int rc;
	unsigned long flags;

	rc = 0;

	spin_lock_irqsave(&qci_batt_info.lock, flags);


	if (type_of_event & SUSPEND_EVENT)
		DPRINTK(KERN_INFO "%s() : Suspend event ocurred."
				"events = %08x\n", __func__, type_of_event);
	else if (type_of_event & RESUME_EVENT)
		DPRINTK(KERN_INFO "%s() : Resume event ocurred."
				"events = %08x\n", __func__, type_of_event);
	else if (type_of_event & CLEANUP_EVENT)
		DPRINTK(KERN_INFO "%s() : Cleanup event ocurred."
				"events = %08x\n", __func__, type_of_event);
	else {

		spin_unlock_irqrestore(&qci_batt_info.lock, flags);
		return -EIO;
	}

	qci_batt_info.type_of_event |=  type_of_event;

	if (qci_batt_info.cb_thread) {
		atomic_set(&qci_batt_info.handle_event, 1);
		send_sig(SIGCONT, qci_batt_info.cb_thread, 0);
		spin_unlock_irqrestore(&qci_batt_info.lock, flags);

		rc = wait_event_interruptible(qci_batt_info.wait_q,
			atomic_read(&qci_batt_info.event_handled));

		atomic_set(&qci_batt_info.event_handled, 0);
	} else {

		atomic_set(&qci_batt_info.handle_event, 1);
		spin_unlock_irqrestore(&qci_batt_info.lock, flags);
	}

	return rc;
}

static void batt_start_cb_thread(void)
{
	atomic_set(&qci_batt_info.handle_event, 0);
	atomic_set(&qci_batt_info.event_handled, 0);
	queue_work(qci_batt_info.msm_batt_wq, &msm_batt_cb_work);
}

static void batt_early_suspend(struct early_suspend *h);

static int qci_batt_cleanup(void)
{
	int rc = 0;
	int rc_local;

	if (qci_batt_info.msm_batt_wq) {
		batt_send_event(CLEANUP_EVENT);
		destroy_workqueue(qci_batt_info.msm_batt_wq);
	}

	if (qci_batt_info.batt_handle != INVALID_BATT_HANDLE) {

		rc = batt_deregister(qci_batt_info.batt_handle);

	}
	qci_batt_info.batt_handle = INVALID_BATT_HANDLE;

	if (qci_batt_info.msm_psy_ac)
		power_supply_unregister(qci_batt_info.msm_psy_ac);

	if (qci_batt_info.msm_psy_usb)
		power_supply_unregister(qci_batt_info.msm_psy_usb);
	
	if (qci_batt_info.msm_psy_batt)
		power_supply_unregister(qci_batt_info.msm_psy_batt);

	if (qci_batt_info.batt_ep) {
		rc_local = msm_rpc_close(qci_batt_info.batt_ep);

			if (!rc)
				rc = rc_local;
	}

	if (qci_batt_info.chg_ep) {
		rc_local = msm_rpc_close(qci_batt_info.chg_ep);
		if (rc_local < 0) {
			if (!rc)
				rc = rc_local;
		}
	}
#ifdef CONFIG_HAS_EARLYSUSPEND
	if (qci_batt_info.early_suspend.suspend == batt_early_suspend)
		unregister_early_suspend(&qci_batt_info.early_suspend);
#endif

		
		if(polling){
			del_timer(&polling_timer);
		}
		gbatt_exit();

	return rc;
}

static u32 batt_capacity(u32 current_voltage)
{
	u32 low_voltage = qci_batt_info.voltage_min_design;
	u32 high_voltage = qci_batt_info.voltage_max_design;

	return (current_voltage - low_voltage) * 100
	    / (high_voltage - low_voltage);
}

#ifdef CONFIG_HAS_EARLYSUSPEND
void batt_early_suspend(struct early_suspend *h)
{
	int rc;

	if (qci_batt_info.current_chg_source > 0)
		return;

	g_batt_suspended =1;
	rc = batt_send_event(SUSPEND_EVENT);

	if ( polling) {
		del_timer(&polling_timer);
	}



}

void batt_late_resume(struct early_suspend *h)
{
	int rc;

	if(g_batt_suspended){
		g_batt_suspended = 0;
	}else{
		return;
	}

	rc = batt_send_event(RESUME_EVENT);

		gbatt_update_psy_status();
		schedule_work(&gbatt_workqueue);
		if (polling) {
			setup_timer(&polling_timer, batt_polling_timer_func, 0);
			mod_timer(&polling_timer, jiffies + msecs_to_jiffies(g_polling_val));
		}


}
#endif


static int batt_enable_filter(u32 vbatt_filter)
{
	int rc;
	struct rpc_req_vbatt_filter {
		struct rpc_request_hdr hdr;
		u32 batt_handle;
		u32 enable_filter;
		u32 vbatt_filter;
	} req_vbatt_filter;

	struct rpc_rep_vbatt_filter {
		struct rpc_reply_hdr hdr;
		u32 filter_enable_result;
	} rep_vbatt_filter;

	req_vbatt_filter.batt_handle = cpu_to_be32(qci_batt_info.batt_handle);
	req_vbatt_filter.enable_filter = cpu_to_be32(1);
	req_vbatt_filter.vbatt_filter = cpu_to_be32(vbatt_filter);

	rc = msm_rpc_call_reply(qci_batt_info.batt_ep,
				BATTERY_ENABLE_DISABLE_FILTER_PROC,
				&req_vbatt_filter, sizeof(req_vbatt_filter),
				&rep_vbatt_filter, sizeof(rep_vbatt_filter),
				msecs_to_jiffies(BATT_RPC_TIMEOUT));
	if (rc < 0) {
		return rc;
	} else {
		rc =  be32_to_cpu(rep_vbatt_filter.filter_enable_result);

		if (rc != BATTERY_DEREGISTRATION_SUCCESSFUL) {
			return -EIO;
		}

		return 0;
	}
}


static int __devinit qci_batt_probe(struct platform_device *pdev)
{
	int rc;
	struct msm_psy_batt_pdata *pdata = pdev->dev.platform_data;

	if (pdev->id != -1) {
		return -EINVAL;
	}

	if (pdata->avail_chg_sources & AC_CHG) {
		rc = power_supply_register(&pdev->dev, &msm_psy_ac);
		if (rc < 0) {
			qci_batt_cleanup();
			return rc;
		}
		qci_batt_info.msm_psy_ac = &msm_psy_ac;
		qci_batt_info.avail_chg_sources |= AC_CHG;
	}

	if (pdata->avail_chg_sources & USB_CHG) {
		rc = power_supply_register(&pdev->dev, &msm_psy_usb);
		if (rc < 0) {
			qci_batt_cleanup();
			return rc;
		}
		qci_batt_info.msm_psy_usb = &msm_psy_usb;
		qci_batt_info.avail_chg_sources |= USB_CHG;
	}

	if (!qci_batt_info.msm_psy_ac && !qci_batt_info.msm_psy_usb) {
		qci_batt_cleanup();
		return -ENODEV;
	}

	qci_batt_info.voltage_max_design = pdata->voltage_max_design;
	qci_batt_info.voltage_min_design = pdata->voltage_min_design;
	qci_batt_info.batt_technology = pdata->batt_technology;
	qci_batt_info.calculate_capacity = pdata->calculate_capacity;

	if (!qci_batt_info.voltage_min_design)
		qci_batt_info.voltage_min_design = BATTERY_MIN;
	if (!qci_batt_info.voltage_max_design)
		qci_batt_info.voltage_max_design = BATTERY_MAX;


	if (qci_batt_info.batt_technology == POWER_SUPPLY_TECHNOLOGY_UNKNOWN)
		qci_batt_info.batt_technology = POWER_SUPPLY_TECHNOLOGY_LION;

	if (!qci_batt_info.calculate_capacity)
		qci_batt_info.calculate_capacity = batt_capacity;

	rc = power_supply_register(&pdev->dev, &msm_psy_batt);
	if (rc < 0) {
		qci_batt_cleanup();
		return rc;
	}
	qci_batt_info.msm_psy_batt = &msm_psy_batt;

	rc = batt_register(BATTERY_LOW, BATTERY_ALL_ACTIVITY,
			       BATTERY_CB_ID_ALL_ACTIV, BATTERY_ALL_ACTIVITY);
	if (rc < 0) {
		qci_batt_cleanup();
		return rc;
	}
	qci_batt_info.batt_handle = rc;

	rc =  batt_enable_filter(VBATT_FILTER);

#ifdef CONFIG_HAS_EARLYSUSPEND
	qci_batt_info.early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	qci_batt_info.early_suspend.suspend = batt_early_suspend;
	qci_batt_info.early_suspend.resume = batt_late_resume;
	register_early_suspend(&qci_batt_info.early_suspend);
#endif
	batt_start_cb_thread();

	return 0;
}

static struct platform_device *bat_pdev;
static int  qci_gbatt_init(void)
{
	int ret = 0;
	int err = 0;
	int rc;
	
	polling=1;

        gbatt_init();

	get_gbatt_info();


	gbatt_update_time = jiffies;
	
	qci_batt_info.batt_valid = 1;
	qci_batt_info.batt_health = POWER_SUPPLY_HEALTH_GOOD;
	qci_batt_info.batt_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
	qci_batt_info.voltage_min_design = BATTERY_MIN;
	qci_batt_info.voltage_max_design = BATTERY_MAX;
	qci_batt_info.batt_technology = POWER_SUPPLY_TECHNOLOGY_LION;
	qci_batt_info.battery_ot = 0;	
	
	bat_pdev = platform_device_register_simple("qci-gbattery", 0, NULL, 0);

	rc = power_supply_register(&bat_pdev->dev, &msm_psy_ac);
	if (rc < 0) {
		qci_batt_cleanup();
		return rc;
	}
	qci_batt_info.msm_psy_ac = &msm_psy_ac;
	qci_batt_info.avail_chg_sources |= AC_CHG;

	rc = power_supply_register(&bat_pdev->dev, &msm_psy_usb);
	if (rc < 0) {
		qci_batt_cleanup();
		return rc;
	}
	qci_batt_info.msm_psy_usb = &msm_psy_usb;
	qci_batt_info.avail_chg_sources |= USB_CHG;


	if (!qci_batt_info.msm_psy_ac && !qci_batt_info.msm_psy_usb) {
		qci_batt_cleanup();
		return -ENODEV;
	}

	rc = power_supply_register(&bat_pdev->dev, &msm_psy_batt);
	if (rc < 0) {
		qci_batt_cleanup();
		return rc;
	}
	qci_batt_info.msm_psy_batt = &msm_psy_batt;

	rc = batt_register(BATTERY_LOW, BATTERY_ALL_ACTIVITY,
			       BATTERY_CB_ID_ALL_ACTIV, BATTERY_ALL_ACTIVITY);
	
	if (rc < 0) {
		qci_batt_cleanup();
		return rc;
	}
	qci_batt_info.batt_handle = rc;
	
	rc =  batt_enable_filter(VBATT_FILTER);

#ifdef CONFIG_HAS_EARLYSUSPEND
	qci_batt_info.early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN;
	qci_batt_info.early_suspend.suspend = batt_early_suspend;
	qci_batt_info.early_suspend.resume = batt_late_resume;
	register_early_suspend(&qci_batt_info.early_suspend);
#endif


	batt_start_cb_thread();

	mutex_init(&workqueue_lock);
	INIT_WORK(&gbatt_workqueue, (work_func_t)gbatt_work_f);

	if (polling) {
		g_polling_val = GBATT_TIME;
		setup_timer(&polling_timer, batt_polling_timer_func, 0);
		mod_timer(&polling_timer, jiffies + msecs_to_jiffies(g_polling_val));
	}
	
	goto success;

success:

	return ret;
	
}

int msm_batt_get_charger_api_version(void)
{
	int rc ;
	struct rpc_reply_hdr *reply;

	struct rpc_req_chg_api_ver {
		struct rpc_request_hdr hdr;
		u32 more_data;
	} req_chg_api_ver;

	struct rpc_rep_chg_api_ver {
		struct rpc_reply_hdr hdr;
		u32 num_of_chg_api_versions;
		u32 *chg_api_versions;
	};

	u32 num_of_versions;

	struct rpc_rep_chg_api_ver *rep_chg_api_ver;


	req_chg_api_ver.more_data = cpu_to_be32(1);

	msm_rpc_setup_req(&req_chg_api_ver.hdr, CHG_RPC_PROG, CHG_RPC_VERS,
			 ONCRPC_CHARGER_API_VERSIONS_PROC);

	rc = msm_rpc_write(qci_batt_info.chg_ep, &req_chg_api_ver,
			sizeof(req_chg_api_ver));
	if (rc < 0) {

		return rc;
	}

	for (;;) {
		rc = msm_rpc_read(qci_batt_info.chg_ep, (void *) &reply, -1,
				BATT_RPC_TIMEOUT);
		if (rc < 0)
			return rc;
		if (rc < RPC_REQ_REPLY_COMMON_HEADER_SIZE) {


			rc = -EIO;
			break;
		}

		if (reply->type == RPC_TYPE_REQ) {

			kfree(reply);
			continue;
		}

		if (reply->xid != req_chg_api_ver.hdr.xid) {

			kfree(reply);
			continue;
		}
		if (reply->reply_stat != RPCMSG_REPLYSTAT_ACCEPTED) {
			rc = -EPERM;
			break;
		}
		if (reply->data.acc_hdr.accept_stat !=
				RPC_ACCEPTSTAT_SUCCESS) {
			rc = -EINVAL;
			break;
		}

		rep_chg_api_ver = (struct rpc_rep_chg_api_ver *)reply;

		num_of_versions =
			be32_to_cpu(rep_chg_api_ver->num_of_chg_api_versions);

		rep_chg_api_ver->chg_api_versions =  (u32 *)
			((u8 *) reply + sizeof(struct rpc_reply_hdr) +
			sizeof(rep_chg_api_ver->num_of_chg_api_versions));

		rc = be32_to_cpu(
			rep_chg_api_ver->chg_api_versions[num_of_versions - 1]);

		break;
	}
	kfree(reply);
	return rc;
}


static struct platform_driver qci_batt_driver;
static int __devinit batt_init_rpc(void)
{

	spin_lock_init(&qci_batt_info.lock);

	qci_batt_info.msm_batt_wq =
	    create_singlethread_workqueue("msm_battery");

	if (!qci_batt_info.msm_batt_wq) {

		return -ENOMEM;
	}

	qci_batt_info.batt_ep =
	    msm_rpc_connect_compatible(BATTERY_RPC_PROG, BATTERY_RPC_VERS, 0);

	if (qci_batt_info.batt_ep == NULL) {
		return -ENODEV;
	} else if (IS_ERR(qci_batt_info.batt_ep)) {
		int rc = PTR_ERR(qci_batt_info.batt_ep);
		qci_batt_info.batt_ep = NULL;
		return rc;
	}
	qci_batt_info.chg_ep =
	    msm_rpc_connect_compatible(CHG_RPC_PROG, CHG_RPC_VERS, 0);

	if (qci_batt_info.chg_ep == NULL) {
		return -ENODEV;
	} else if (IS_ERR(qci_batt_info.chg_ep)) {
		int rc = PTR_ERR(qci_batt_info.chg_ep);
		qci_batt_info.chg_ep = NULL;
		return rc;
	}

	qci_batt_info.chg_api_version =  msm_batt_get_charger_api_version();

	if (qci_batt_info.chg_api_version < 0)
		qci_batt_info.chg_api_version = DEFAULT_CHARGER_API_VERSION;


	init_waitqueue_head(&qci_batt_info.wait_q);

	return 0;
}

static int __devexit qci_batt_remove(struct platform_device *pdev)
{
	int rc;
	rc = qci_batt_cleanup();

	if (rc < 0) {
		return rc;
	}
	return 0;
}

static struct platform_driver qci_batt_driver = {
	.probe = qci_batt_probe,
	.remove = __devexit_p(qci_batt_remove),
	.driver = {
		   .name = "msm-battery",
		   .owner = THIS_MODULE,
		   },
};

static int __init qci_batt_init(void)
{
	int rc;

	rc = batt_init_rpc();

	if (rc < 0) {
		qci_batt_cleanup();
		return rc;
	}
	
	#ifdef CONFIG_HAS_EARLYSUSPEND
		wake_lock_init(&g_batt_wlock, WAKE_LOCK_SUSPEND,"msm_battery_active");
	#endif
	gbatt_update_time=1;
	qci_gbatt_init();

	return 0;
}

static void __exit qci_batt_exit(void)
{
	#ifdef CONFIG_HAS_EARLYSUSPEND
		wake_lock_destroy(&g_batt_wlock);
	#endif
	platform_driver_unregister(&qci_batt_driver);
}

module_init(qci_batt_init);
module_exit(qci_batt_exit);

MODULE_LICENSE("GPLv2");
MODULE_AUTHOR("Quanta Computer Inc.");
MODULE_DESCRIPTION("Battery driver for Qualcomm MSM chipsets on Quanta Devices");
MODULE_VERSION("1.0");
MODULE_ALIAS("platform:msm_battery");
