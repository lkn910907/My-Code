/**
 * This is the module that get the grant gref from another
 * domain, and this domian can read the xenstore to get the port and gref,
 * will do a map page Hypercall. 
 * After this,the doamin can have access to the mapped page.
 *
 */

#include <linux/module.h>
//#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <xen/xenbus.h>
#include <xen/interface/grant_table.h>
#include <xen/interface/io/blkif.h>  // for definition of blkif_sring_t  
#include <xen/grant_table.h>
#include <linux/vmalloc.h>

#include <xen/events.h>
#include <xen/interface/event_channel.h>
#include <xen/interface/io/ring.h>

#include "dbg_kernel.h"

struct gnttab_map_grant_ref ops;
struct gnttab_unmap_grant_ref unmap_ops;

int remoteDomain=0;
int gref;
int port;

//module_param( gref, int , S_IRUGO);
//module_param( port, int, S_IRUGO);

struct info_t{
        int irq;
	int gref;
	int remoteDomain;
	int evtchn;
}info;

static irqreturn_t handle_evt (int irq, void *dev_id)
{
        return IRQ_HANDLED;
}

int init_module(void)
{
	struct vm_struct * v_start;
	int ret, err;

	struct xenbus_transaction trans;
        int rc;
	
	log_info("init_module");

	rc = xenbus_transaction_start(&trans);
        check(rc==0, "transaction start failed");

        rc = xenbus_scanf(trans, "yinwuzhe", "gref", "%d",&gref);
	check(rc!=-ERANGE, "xenbus_scanf failed");
	rc = xenbus_scanf(trans, "yinwuzhe", "port", "%d",&port);
	check(rc!=-ERANGE, "xenbus_scanf failed");
        xenbus_transaction_end(trans, 0);

        log_info("read from the xenstore is gref=%d,port=%d",gref, port);
	
	//this func reserves a range of kernel address space
	//and allocates pagetables to map that area, but no actual mapping here.
	v_start = alloc_vm_area(PAGE_SIZE , NULL);
	check(v_start, "xen:could not allocate page\n");
	
	/*
	//gnttab_set_map_op(struct gnttab_map_grant_ref *map, phys_addr_t addr,
	//                  uint32_t flags, grant_ref_t ref, domid_t domid)
	gnttab_set_map_op(&ops, (unsigned long)v_start->addr, GNTMAP_host_map,
				 gref , remoteDomain);
	
	//do the map hypercall
	ret = HYPERVISOR_grant_table_op(GNTTABOP_map_grant_ref, &ops, 1);
	check(ret==0, "xen: HYPERSIOR map grant ref failed.\n");
	
	//check the staus from the out param of the ops
	check(ops.status==0, "xen: HYPERSIOR map grant ref failed with status %d.\n",ops.status);
	
	//printk handle 
	log_info("xen:map grant success:\n\tshared_page=%x, handle=%d\n",
			(unsigned long)v_start->addr,ops.handle);
	
	//if mapped,init the unmap ops with the handle and host_addr
	unmap_ops.host_addr=(unsigned long)v_start->addr;
	unmap_ops.handle=ops.handle;
	*/

	info.remoteDomain=0;
	info.evtchn =port;
	err = bind_interdomain_evtchn_to_irqhandler(info.remoteDomain, info.evtchn, 
			handle_evt, 0,"get-grant",&info);
	check(err>=0,"bind_interdomain_evtchn_to_irqhandler failed.");
	info.irq=err;
	log_info("evtchn port = %d, the handle irq = %d", info.evtchn, info.irq);
	return 0;	
	
error:
	free_vm_area(v_start);
	return -1;
}

void cleanup_module(void)
{
	int ret;
	printk("xen:cleanup_module of get_grant\n");

	// unmap_ops.handle points to the pages that were mapped
	// unmap_ops->host_addr points to the the heap where pages were mapped
	ret = HYPERVISOR_grant_table_op(GNTTABOP_unmap_grant_ref, &unmap_ops, 1);  
	if(ret==0){
		log_info("cleanup: unmapped shared page \n" );
	}
	else{
		log_info(" cleanup:failed to unmap the shared page \n");
	}
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wuzhe Yin");
MODULE_DESCRIPTION("the module get an grant from another doamin");
