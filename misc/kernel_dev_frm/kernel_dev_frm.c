/*
 * kernel develop quick start code.
 *
 * Copyright (C) 2017 Mingbao Sun <sunmingbao@126.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/jiffies.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <asm/byteorder.h>

#define		MODULE_NAME		"kernel_dev_frm"
#define		VERSION			"1.0.0"

#define		PFX		MODULE_NAME": "

struct mod_data {
#if defined(__BIG_ENDIAN_BITFIELD)
	__u8		a:4,
				b:4;
#elif defined(__LITTLE_ENDIAN_BITFIELD)
	__u8		b:4,
				a:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	int some_data;
};

static struct mod_data the_mod_data;

static int init_mod_data(void)
{
    the_mod_data.some_data=1;
	return 0;
}

static int __init the_init(void)
{
	int err = 0;

	err = init_mod_data();
	goto out;

out:
	return err;

}

static void __exit the_exit(void)
{
	;
}

module_init(the_init);
module_exit(the_exit);

MODULE_VERSION(VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mingbao Sun <sunmingbao@126.com>");
MODULE_DESCRIPTION("kernel develop quick start code");

