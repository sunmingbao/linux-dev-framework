#include <linux/kthread.h>
#include <linux/blkdev.h>
#include <linux/sysctl.h>
#include <linux/seq_file.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <linux/hdreg.h>
#include <linux/proc_fs.h>
#include <linux/random.h>
#include <linux/module.h>
#include <linux/reboot.h>
#include <linux/file.h>
#include <linux/compat.h>
#include <linux/delay.h>

#include <asm/uaccess.h>

#define		MODULE_NAME		"blk_drv_test"
#define		VERSION			"1.0.0"

#define		PFX		MODULE_NAME": "


#define MAX_HD_NR 3
#define SECTORS_PER_HD  (16384)

static int part_shift = 3;


#if 1
#define    DBG_PRINT(fmt, args...) \
    do \
    { \
        printk(KERN_ERR PFX "%s(%d)-%s:\n"fmt"\n", __FILE__,__LINE__,__FUNCTION__,##args); \
    } while (0)
#else
#define    DBG_PRINT(fmt, args...)  /* */
#endif

struct my_disk_struct {
	int 				index;
	int				major;
	int				minor;
	sector_t			sectors;
	size_t                   	bytes;
	void *				disk_space;
	struct request_queue 		*queue;
	struct gendisk			*disk;

};

static struct my_disk_struct *pt_disks[MAX_HD_NR];
static int disk_cnt;

static int blk_major;


static int my_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	struct my_disk_struct *pt_disk = bdev->bd_disk->private_data;

	geo->heads = 2;
	geo->sectors = 4;
	geo->cylinders = pt_disk->sectors / 8;
	return 0;
}

static const struct block_device_operations my_blk_fops = {
	.getgeo =	my_getgeo,
};

static void do_one_region(struct my_disk_struct * pt_disk, unsigned long sector, unsigned long nr, char *buffer, int write)
{
	unsigned long off = sector*512;
	unsigned long bytes = nr*512;

	if (write) {
		memcpy(pt_disk->disk_space + off, buffer, bytes);
	} else {
		memcpy(buffer, pt_disk->disk_space + off, bytes);
	}
	
}

static int process_bio(struct my_disk_struct * pt_disk, struct bio *bio)
{

	sector_t sector = bio->bi_sector;
	unsigned int size = bio->bi_size;
	int write = (bio_data_dir(bio) == WRITE);
	void *buf;
	int i;

	if (!size)
		return 0;


	for (i=0; i<bio->bi_vcnt; i++) {
		buf = kmap_atomic(bio->bi_io_vec[i].bv_page)+ bio->bi_io_vec[i].bv_offset;
		do_one_region(pt_disk, sector, bio->bi_io_vec[i].bv_len/512, buf, write);
		sector += bio->bi_io_vec[i].bv_len/512;
		kunmap_atomic(buf);
	}


	return 0;
}


static void my_make_request(struct request_queue *q, struct bio *bio)
{
	struct my_disk_struct *pt_disk = q->queuedata;
	int status;

	status = process_bio(pt_disk, bio);
	bio_endio(bio, status);
}

static int add_one_disk(struct my_disk_struct **ppt_disk, int i)
{
	struct my_disk_struct *pt_disk;
	struct gendisk *disk;
	int err;

	err = -ENOMEM;
	pt_disk = kzalloc(sizeof(*pt_disk), GFP_KERNEL);
	if (!pt_disk)
		goto out;

	pt_disk->sectors = SECTORS_PER_HD;
	pt_disk->bytes = pt_disk->sectors*512;
	pt_disk->disk_space = vmalloc(pt_disk->sectors*512);
	if (!pt_disk->disk_space)
		goto out_free_lo;
	memset(pt_disk->disk_space, 0, pt_disk->bytes);

	err = -ENOMEM;
	pt_disk->queue = blk_alloc_queue(GFP_KERNEL);
	if (!pt_disk->queue)
		goto out_free_disk_space;

	blk_queue_make_request(pt_disk->queue, my_make_request);
	blk_queue_logical_block_size(pt_disk->queue, 512);
	pt_disk->queue->queuedata = pt_disk;

	disk = pt_disk->disk = alloc_disk(1 << part_shift);
	if (!disk)
		goto out_free_queue;


	disk->flags |= GENHD_FL_EXT_DEVT;

	pt_disk->index= i;
	disk->major		= blk_major;
	disk->first_minor	= i << part_shift;
	disk->fops		= &my_blk_fops;
	disk->private_data	= pt_disk;
	disk->queue		= pt_disk->queue;
	set_capacity(disk, pt_disk->sectors);
	sprintf(disk->disk_name, "haha%d", i);
	add_disk(disk);
	*ppt_disk = pt_disk;
	return pt_disk->index;

out_free_queue:
	blk_cleanup_queue(pt_disk->queue);
out_free_disk_space:
	vfree(pt_disk->disk_space);
out_free_lo:
	kfree(pt_disk);
out:
	return err;
}

static void remove_my_disk(struct my_disk_struct *pt_disk)
{
	del_gendisk(pt_disk->disk);
	blk_cleanup_queue(pt_disk->queue);
	put_disk(pt_disk->disk);
	vfree(pt_disk->disk_space);
	kfree(pt_disk);
}

static void uninstall_disks(void)
{
	int drive;

	if (!disk_cnt)
		return;

	for (drive = 0 ; drive < MAX_HD_NR; drive++) {
		if (pt_disks[drive]) {
			DBG_PRINT("uninstall disk %d", drive);
			remove_my_disk(pt_disks[drive]);
			pt_disks[drive]=NULL;
			disk_cnt--;
		}

	}
}

static int install_disks(void)
{
	int drive;
	int err;
	for (drive = 0 ; drive < MAX_HD_NR; drive++) {

		err = add_one_disk(&pt_disks[drive], drive);
		if (err==drive)	{
			DBG_PRINT("install disk %d success", drive);
			disk_cnt++;
		} else {
			DBG_PRINT("install disk %d failed", drive);
			goto ABORT;

		}
	}

	return 0;

ABORT:
	uninstall_disks();
	return err;
}
static int __init the_init(void)
{
	int err = 0;
	
	DBG_PRINT("==init");
	err = register_blkdev(0, MODULE_NAME);
	if (err<0) {
		DBG_PRINT("register_blkdev failed, err=%d", err);
		goto out;
	}

	blk_major = err;
	DBG_PRINT("register_blkdev success, blk_major=%d", blk_major);

	err = install_disks();

out:
	return err;

}

static void __exit the_exit(void)
{
	DBG_PRINT("==exit ");

	uninstall_disks();

	if (blk_major) {
		DBG_PRINT("unregister_blkdev, blk_major=%d", blk_major);
		unregister_blkdev(blk_major, MODULE_NAME);
		blk_major = 0;
	}
}

module_init(the_init);
module_exit(the_exit);

MODULE_VERSION(VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mingbao Sun <sunmingbao@126.com>");
MODULE_DESCRIPTION("block device driver test code");

