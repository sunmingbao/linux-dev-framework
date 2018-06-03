/*
 * drop all packets received from specified net device.
 *
 * Copyright (C) 2018 Mingbao Sun <sunmingbao@126.com>
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
#include <linux/smp.h>
#include <linux/threads.h>

#include <linux/mm.h>
#include <linux/netdevice.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/types.h>

#define		MODULE_NAME		"drop_packets"
#define		VERSION			"1.0.0"

#define		PFX		MODULE_NAME": "

#define    DBG_PRINT(fmt, args...) \
    do \
    { \
        if (gt_t_working_ctxt.enable_dbg_prt) \
        printk(KERN_ERR PFX "%s(%d)-%s:\n"fmt"\n", __FILE__,__LINE__,__FUNCTION__,##args); \
    } while (0)


typedef struct {
	char				name[IFNAMSIZ];
	struct net_device	*ndev;
	unsigned int		dsb_np_bit_orig;
	unsigned long		pkt_drop_cnt[NR_CPUS];
	unsigned long		byte_drop_cnt[NR_CPUS];
	
} netif_info;

typedef struct
{
	int enable_dbg_prt;
	int CONFIG_NET_CLS_ACT;
} t_working_ctxt;


#define    TARGET_DEV    "enp0s10"

static t_working_ctxt gt_t_working_ctxt = 
{
	.enable_dbg_prt = 1,
};


static netif_info *pt_netif;

static int get_netif_info(netif_info *pt_netif, const char *dev_name)
{
	struct net *net = current->nsproxy->net_ns;
	struct net_device *ndev = dev_get_by_name(net, dev_name);

	DBG_PRINT("==dev_name=%s", dev_name);

	if (!ndev) {
		DBG_PRINT("%s doesn't exist\n", dev_name);
		return  -ENODEV;
	}

	memset(pt_netif, 0, sizeof(netif_info));
	pt_netif->ndev = ndev;
	strcpy(pt_netif->name, ndev->name);
	pt_netif->dsb_np_bit_orig = (ndev->priv_flags & IFF_DISABLE_NETPOLL);

	DBG_PRINT("==");

	return 0;

}

#define netif_info_get_rcu(dev) \
	(rcu_dereference(dev->rx_handler_data))

static rx_handler_result_t drop_packet(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	netif_info *pt_netif = NULL;
	int this_cpu = safe_smp_processor_id();
	unsigned long pkt_cnt=0, byte_cnt=0;

	rcu_read_lock();
	/* netif_info on a net_device may be freed dynamicaly by ourself */
	pt_netif = netif_info_get_rcu(skb->dev);
	if (pt_netif)
	{
		pt_netif->pkt_drop_cnt[this_cpu]++;
		pt_netif->byte_drop_cnt[this_cpu] += skb->len;
		pkt_cnt = pt_netif->pkt_drop_cnt[this_cpu];
		byte_cnt = pt_netif->byte_drop_cnt[this_cpu];
	}

	rcu_read_unlock();
	
	kfree_skb(*pskb);
	*pskb = NULL;

	if (pt_netif)
		DBG_PRINT("==cpu=%d, drop_pkt_cnt=%lu, drop_byte_cnt=%lu", this_cpu, pkt_cnt, byte_cnt);
	
	return RX_HANDLER_CONSUMED;
}

static void netif_drop_packets_disable(netif_info *pt_netif)
{
	struct net_device *ndev = (pt_netif->ndev);
	if (!pt_netif->dsb_np_bit_orig)
	{
		pt_netif->ndev->priv_flags &= (~(IFF_DISABLE_NETPOLL));
		smp_wmb();
	}
	
	netdev_rx_handler_unregister(ndev);
	smp_wmb();
	
	dev_put(pt_netif->ndev);

	synchronize_rcu();
	kfree(pt_netif);

	DBG_PRINT("==");


}

static int netif_drop_packets_enable(netif_info *pt_netif)
{
	int ret = 0;
	ret = netdev_rx_handler_register(pt_netif->ndev, drop_packet,
					 pt_netif);
	if (ret) {
		DBG_PRINT("Error %d calling netdev_rx_handler_register\n", ret);
		goto out;
	}

#ifdef CONFIG_NETPOLL

	DBG_PRINT("==CONFIG_NETPOLL");

	/* no effection for net_device on which netpoll have already been set up */
	pt_netif->ndev->priv_flags |= IFF_DISABLE_NETPOLL;
	smp_wmb();
	if (rcu_dereference(pt_netif->ndev->npinfo))
	{
		/* netpoll already set up on this dev, rollback previous operations and return fail */
		netif_drop_packets_disable(pt_netif);
		ret = -EBUSY;
		goto out;

	}

	
#endif
	DBG_PRINT("==");

out:
	return ret;

}


static int __init the_init(void)
{
	int err = 0;
	
	DBG_PRINT("==");

#ifdef CONFIG_NET_CLS_ACT
	gt_t_working_ctxt.CONFIG_NET_CLS_ACT = 1;
#endif

	pt_netif = kmalloc(sizeof(netif_info), GFP_KERNEL);
	if (!pt_netif)
	{
		err = -ENOMEM;
		DBG_PRINT("==no memory");
		goto out;
	}

	err = get_netif_info(pt_netif, TARGET_DEV);
	if (err)
		goto out;

	err = netif_drop_packets_enable(pt_netif);
	if (err)
		goto out;

out:
	return err;

}

static void __exit the_exit(void)
{
	netif_drop_packets_disable(pt_netif);
	DBG_PRINT("==");
}

module_init(the_init);
module_exit(the_exit);

MODULE_VERSION(VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Mingbao Sun <sunmingbao@126.com>");
MODULE_DESCRIPTION("drop all packets received from specified net device");

