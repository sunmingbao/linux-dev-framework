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
#include <linux/pagemap.h>
#include <asm/byteorder.h>
#include <asm/io.h>

#define		MODULE_NAME		"kernel_dev_frm"
#define		VERSION			"1.0.0"

#define		PFX		MODULE_NAME": "

#if 1
#define    DBG_PRINT(fmt, args...) \
    do \
    { \
        printk(KERN_ERR PFX "%s(%d)-%s:\n"fmt"\n", __FILE__,__LINE__,__FUNCTION__,##args); \
    } while (0)
#else
#define    DBG_PRINT(fmt, args...)  /* */
#endif

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
	spinlock_t lock;
    int condition_ok;
	struct task_struct *the_owner;
	void *shm;
};

static struct mod_data the_mod_data;

static int init_mod_data(void)
{
    spin_lock_init(&(the_mod_data.lock));
    the_mod_data.shm = (void *) __get_free_pages(GFP_KERNEL, 1);
	if (NULL == the_mod_data.shm) {
		return -ENOMEM;
	}
	
	the_mod_data.the_owner = NULL;

	return 0;
}

static void deinit_mod_data(void)
{
	free_pages((unsigned long)the_mod_data.shm, 1);
	the_mod_data.the_owner = NULL;
}


struct per_file_data
{
    int a;
};

static void init_pf_data(struct per_file_data *pf_data)
{

}

/*
 *	Kernel Interfaces
 */

static int the_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	struct per_file_data *pf_data;
	spin_lock(&(the_mod_data.lock));

	if (the_mod_data.the_owner)
	{
		ret = -EBUSY;
		goto exit;
	}
		
	pf_data = kmalloc(sizeof(struct per_file_data), GFP_KERNEL);
	if (NULL == pf_data)
	{
		ret = -ENOMEM;
		goto exit;
	}

	init_pf_data(pf_data);

	file->private_data = pf_data;
	the_mod_data.the_owner = current;
	
exit:

	spin_unlock(&(the_mod_data.lock));
	return ret;
}

static int the_release(struct inode *inode, struct file *file)
{
	struct per_file_data *pf_data;
	
	spin_lock(&(the_mod_data.lock));

	pf_data = file->private_data;
	kfree(pf_data);
	the_mod_data.the_owner = NULL;
	spin_unlock(&(the_mod_data.lock));
	
	return 0;
}

static ssize_t the_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
    __set_current_state(TASK_INTERRUPTIBLE);
    if (!the_mod_data.condition_ok)
		schedule();

    __set_current_state(TASK_RUNNING);
	if (signal_pending(current))
        return -EINTR;
	
	return count;
}

static char file_data[32];

static ssize_t the_read(struct file *file, char __user *buf,
				size_t count, loff_t *ppos)
{
    loff_t offset = *ppos;
	size_t data_left = sizeof(file_data) - offset;
	size_t csize=(count<data_left?count:data_left);
	
	if (0==data_left)
		return 0;

	if (copy_to_user(buf, (file_data + offset), csize))
		return -EFAULT;

	*ppos += csize;
	return csize;
}

static int the_mmap(struct file *filp, struct vm_area_struct *vma)
 {
     unsigned long pfn = (virt_to_phys(the_mod_data.shm) >> PAGE_SHIFT);

     if (remap_pfn_range(vma, vma->vm_start, pfn,
                vma->vm_end - vma->vm_start,
                vma->vm_page_prot))
         return -EAGAIN;

     return 0;
 }

static const struct file_operations the_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= the_write,
	.read	    = the_read,
	.mmap       = the_mmap,
	.open		= the_open,
	.release	= the_release,
};

static struct miscdevice the_miscdev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= MODULE_NAME,
	.fops	= &the_fops,
};

static int __init the_init(void)
{
	int err = 0;
	
	DBG_PRINT("==");

	err = init_mod_data();
	if (err)
	    goto out;

	err = misc_register(&the_miscdev);
	if (err) {
		printk(KERN_ERR PFX "cannot register the_miscdev");
		goto out;
	}

out:
	return err;

}

static void __exit the_exit(void)
{
	misc_deregister(&the_miscdev);
	deinit_mod_data();

}

module_init(the_init);
module_exit(the_exit);

MODULE_VERSION(VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mingbao Sun <sunmingbao@126.com>");
MODULE_DESCRIPTION("kernel develop quick start code");

