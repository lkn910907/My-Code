/*this is a module in DomU to create a Grant Reference,
 * and this with the event channel port will pass by Xenstore to the Dom0,then dom0  
 * can access the page in DomU by the GR, ...to do something
 * */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <xen/xenbus.h>

#include <xen/events.h>
#include <xen/interface/event_channel.h>

#include <asm/pgtable.h>
#include <xen/page.h>
#include <asm/sync_bitops.h>
#include <xen/interface/grant_table.h>
#include <xen/interface/xen.h>
#include <xen/grant_table.h>
#include "dbg_kernel.h"

int  page;

grant_ref_t gref=0;

//grant to which domain
#define DOMO_ID 0

struct info_t{
	grant_ref_t gref;
	int irq;
	int port;
}info;

static irqreturn_t handle_evt (int irq, void *dev_id)
{
	return IRQ_HANDLED;
}

int init_module(void){

	int mfn;
	int port , irq;
        struct evtchn_alloc_unbound alloUnb;
	struct xenbus_transaction trans;
	int rc;

	page = __get_free_pages(GFP_KERNEL,1);
	check(page , "xen: could not grant foreign access");

	mfn = virt_to_mfn(page);
	gref = gnttab_grant_foreign_access(DOMO_ID , mfn,0);
	check(gref, "xen: could not grant foreign access");

	log_info(" page = %d, mfn=%d", page,mfn);
	log_info("the grant table ref is %d",gref);
	printk(".......\n"); 

//there is a question in the DOMID_SELF,because i see the value 32752,which is wired
//so I just set it to 0
	alloUnb.dom = 0;
	alloUnb.remote_dom = 0;
	log_info("alloUnb.dom =  %d, alloUnb.remote_dom = %d ",alloUnb.dom, alloUnb.remote_dom);
//why there is a big problem there,may be the argv is wrong
	rc = HYPERVISOR_event_channel_op(EVTCHNOP_alloc_unbound, &alloUnb);
	check(rc==0, "EVTCHNOP_alloc_unbound failed.");
	
	info.gref = gref;
	info.port = alloUnb.port;

	log_info("\tinterrupt = %d, local port = %d", info.irq, info.port);

	irq=bind_evtchn_to_irqhandler(info.port, handle_evt, 0, "grant-dom", &info);
	log_info("\tinterrupt = %d, local port = %d", irq, info.port);
	check(irq>=0,"bind_evtchn_to_irqhandler failed.");
	info.irq=irq;
	log_info("\tinterrupt = %d, local port = %d", info.irq, info.port);
	printk(".......\n");

	rc = xenbus_transaction_start(&trans);
         check(rc==0, "transaction start failed");

        xenbus_printf(trans, "yinwuzhe", "gref", "%d",info.gref);
	xenbus_printf(trans, "yinwuzhe", "port", "%d",info.port);
        xenbus_transaction_end(trans, 0);

	return 0;
error:
	//if I do this,then a big bug!
	free_page((unsigned long)page); 
	return -1;
	
}

void cleanup_module(void){
	printk("Cleanup the gref\n");
	
	if (gnttab_query_foreign_access(gref) == 0) { 
		log_info("No map with the gref %d\n",gref);
		gnttab_end_foreign_access(gref, 0, page);
	}
	else{
		log_info("The gref %d has been mapped.\n",gref);
		gnttab_end_foreign_access(gref,0, page);
	}
	//free_page((unsigned long)page);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wuzhe Yin");
MODULE_DESCRIPTION("the module grant an page to the other doamin");
