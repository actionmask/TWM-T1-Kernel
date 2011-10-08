#ifndef __MJ2_GBATTERY_H__
#define __MJ2_GBATTERY_H__

#define GAUGE_2W_ADDR1	0x55

#define GAUGE_CTRL 0x00
#define GAUGE_MODE 0x01
#define GAUGE_AR_L 0x02
#define GAUGE_AR_H 0x03
#define GAUGE_ARTTE_L 0x04
#define GAUGE_ARTTE_H 0x05
#define GAUGE_TEMP_L 0x06
#define GAUGE_TEMP_H 0x07
#define GAUGE_VOTL_L 0x08
#define GAUGE_VOTL_H 0x09
#define GAUGE_FLAGS 0x0A
#define GAUGE_RSOC 0x0B
#define GAUGE_NAC_L 0x0C
#define GAUGE_NAC_H 0x0D
#define GAUGE_CACD_L 0x0E
#define GAUGE_CACD_H 0x0F
#define GAUGE_CACT_L 0x10
#define GAUGE_CACT_H 0x11
#define GAUGE_LMD_L 0x12
#define GAUGE_LMD_H 0x13
#define GAUGE_AI_L 0x14
#define GAUGE_AI_H 0x15
#define GAUGE_TTE_L 0x16
#define GAUGE_TTE_H 0x17
#define GAUGE_TTF_L 0x18
#define GAUGE_TTF_H 0x19
#define GAUGE_SI_L 0x1A
#define GAUGE_SI_H 0x1B
#define GAUGE_STTE_L 0x1C
#define GAUGE_STTE_H 0x1D
#define GAUGE_MLI_L 0x1E
#define GAUGE_MLI_H 0x1F
#define GAUGE_MLTTE_L 0x20
#define GAUGE_MLTTE_H 0x21
#define GAUGE_SAE_L 0x22
#define GAUGE_SAE_H 0x23
#define GAUGE_AP_L 0x24
#define GAUGE_AP_H 0x25
#define GAUGE_TTECP_L 0x26
#define GAUGE_TTECP_H 0x27
#define GAUGE_CYCL_L 0x28
#define GAUGE_CYCL_H 0x29
#define GAUGE_CYCT_L 0x2A
#define GAUGE_CYCT_H 0x2B
#define GAUGE_CSOC_L 0x2C

#define GAUGE_EE_EN 0x6E
#define GAUGE_ILMD 0x76
#define GAUGE_SEDVF 0x77
#define GAUGE_SEDV1 0x78
#define GAUGE_ISLC 0x79
#define GAUGE_DMFSD 0x7A
#define GAUGE_TAPER 0x7B
#define GAUGE_PKCFG 0x7C
#define GAUGE_IMLC 0x7D
#define GAUGE_DCOMP 0x7E
#define GAUGE_TCOMP 0x7F

#define GAUGE_SOC 0x2C

static int gauge_probe(struct i2c_client *client, const struct i2c_device_id *id);
static int gauge_remove(struct i2c_client *client);

static int gauge_read(u8 cmd_size, u8 *cmd, u8 reply_size, u8 *reply);
static int gauge_reply(struct i2c_client *client, u8 number_bytes, u8 *data_in);
static int gauge_cmd(struct i2c_client *client, u8 number_bytes, u8 *data_in);


int gauge_get_battery_temperature(void);
unsigned int gauge_get_battery_voltage(void);
unsigned int gauge_get_battery_rsoc(void);
s16 gauge_get_battery_current(void);



#endif /* __MJ2_GBATTERY_H__ */
