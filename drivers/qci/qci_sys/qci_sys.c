/* Quanta SYS Driver
 *
 * Copyright (C) 2009 Quanta Computer Inc.
 * Author: Ivan Chang <Ivan.Chang@quantatw.com>
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

#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/module.h>

struct kobject *qci_kobj;

extern int qci_read_hw_id();

static ssize_t hw_id_show(struct kobject *kobj, struct kobj_attribute *attr, char * buf) {
	int pcb;
	pcb = qci_read_hw_id();
       return snprintf(buf, 64, "PCB HW ID = %d\n", pcb);
}

static ssize_t hw_id_store(struct kobject *kobj, struct kobj_attribute *attr, const char * buf, size_t n) {
	return 0;
}

static struct kobj_attribute hw_id = {
        .attr   = {
                .name = "hw_id",
                .mode = 0644,
        },
        .show   = hw_id_show,
        .store   = hw_id_store,
};

static struct attribute * g[] = {
	&hw_id.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = g,
};

static int __init qci_sys_init(void) {
	printk("QCI System Management Init.\n");

	qci_kobj = kobject_create_and_add("qci", NULL);
	if (!qci_kobj)
		return -ENOMEM;

	/* Add suspend_time to power subsys. */
	return sysfs_create_group(qci_kobj, &attr_group);
}

late_initcall(qci_sys_init);

MODULE_AUTHOR("Quanta Computer Inc.");
MODULE_DESCRIPTION("Quanta SYS Driver");
MODULE_LICENSE("GPL v2");

