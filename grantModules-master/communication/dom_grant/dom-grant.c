/*this is a module in DomU to create a Grant Reference,
 * and this will pass manaully to the Dom0,then dom0  
 * can access the page in DomU by the GR
 * */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>

#include <asm/pgtable.h>
#include <xen/page.h>
#include <asm/sync_bitops.h>
#include <xen/interface/grant_table.h>
#include <xen/interface/xen.h>
#include <xen/grant_table.h>
#include "dbg_kernel.h"

unsigned long page;

grant_ref_t gref;
//grant to which domain
//
#define DOMO_ID 0

int init_module(void){

	int mfn;
	page = __get_free_pages(GFP_KERNEL,1);
	check(page , "xen: could not grant foreign access\n");

	mfn = virt_to_mfn(page);
	gref = gnttab_grant_foreign_access(DOMO_ID , mfn,0);
	check(gref, "xen: could not grant foreign access\n");

	printk("\n page = %d, mfn=%d\n", page,mfn);
	printk("the grant table ref is %d.\n",gref);
	printk("....\n..."); 
	
	return 0;
	error:
		if(page)
			free_page((unsigned long)page); 
		return -1;
	
}

void cleanup_module(void){
	printk("Cleanup the gref\n");
	
	if (gnttab_query_foreign_access(gref) == 0) { 
		printk("No map with the gref %d\n",gref);
		gnttab_end_foreign_access(gref, 0, page);
	}
	else{
		printk("The gref %d has been mapped.\n",gref);
		gnttab_end_foreign_access(gref,0, page);
	}

}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wuzhe Yin");
MODULE_DESCRIPTION("the module grant an page to the other doamin");
