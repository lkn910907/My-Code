/**
 * This is the module that get the gran gref from another
 * domain, and this domian will do a mappage Hypercall. 
 * After this,the doamin can have access to the 
 * mapped page.
 *
 * Insmod the module using "insmod get_grant.ko gref=<vaue>
 */

#include <linux/module.h>
 #include <linux/moduleparam.h>
 #include <linux/kernel.h>
 #include <xen/interface/grant_table.h>
 #include <xen/interface/io/blkif.h>  // for definition of blkif_sring_t  
 #include <xen/grant_table.h>
 #include <linux/vmalloc.h>
//this is really not exist
 //#include <asm-x86/xen/hypervisor.h>   
#include <xen/evtchn.h>
#include <xen/events.h>
#include <xen/interface/io/ring.h>
//
//#define GNTTABOP_map_grant_ref        0
//
//struct gnttab_map_grant_ref {
//    /* IN parameters. */
//        uint64_t host_addr;
//            uint32_t flags;               /* GNTMAP_* */
//                grant_ref_t ref;
//                    domid_t  dom;
//                        /* OUT parameters. */
//                            int16_t  status;              /* GNTST_* */
//                                grant_handle_t handle;
//                                    uint64_t dev_bus_addr;
//                   };
//      DEFINE_GUEST_HANDLE_STRUCT(gnttab_map_grant_ref);
struct gnttab_map_grant_ref ops;
struct gnttab_unmap_grant_ref unmap_ops;

int remoteDomain=0;
int gref;
module_param( gref, int , S_IRUGO);

int init_module(void)
{
	//a pointer to a vm_struct<>
	//include .addr,.size .page .nr_pages .phys_addr
	struct vm_struct * v_start;
	int ret;

	printk("xen:init_module with gref = %d\n",gref);
	//this func reserves a range of kernel address space
	//and allocates pagetables to map that area, but no actual mapping here.
	v_start = alloc_vm_area(PAGE_SIZE , NULL);
	if(v_start==0){
		free_vm_area(v_start);
		printk("xen:could not allocate page\n");
		return -1;
	}
	//static inline void
	//gnttab_set_map_op(struct gnttab_map_grant_ref *map, phys_addr_t addr,
	//                  uint32_t flags, grant_ref_t ref, domid_t domid)
	gnttab_set_map_op(&ops, (unsigned long)v_start->addr, GNTMAP_host_map,
				 gref , remoteDomain);
	//return 0,if success the hypercall
	ret = HYPERVISOR_grant_table_op(GNTTABOP_map_grant_ref, &ops, 1);
	if(ret){ 
		printk("xen: HYPERSIOR map grant ref failed.\n");
		return -1;
	}
	//check the staus from the out param of the ops
	if(ops.status != 0){
		printk("xen: HYPERSIOR map grant ref failed with status %d.\n",ops.status);
		return -1;
	}
	//printk handle n the dmesg
	printk("xen:map grant success:\n\tshared_page=%x, handle=%d\n",(unsigned long)v_start->addr,ops.handle);
	//if mapped,init the unmap ops with the handle and host_addr
	unmap_ops.host_addr=(unsigned long)v_start->addr;
	unmap_ops.handle=ops.handle;
	
	return 0;	
}

void cleanup_module(void)
{
	int ret;
	printk("xen:cleanup_module\n");
	// unmap_ops.handle points to the pages that were mapped
	// unmap_ops->host_addr points to the the heap where pages were mapped
	ret = HYPERVISOR_grant_table_op(GNTTABOP_unmap_grant_ref, &unmap_ops, 1);  
	if(ret==0){
		printk("cleanup: unmapped shared page\n");
	}
	else{
		printk("cleanup:failed to unmap the shared page\n");
	}
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Wuzhe Yin");
MODULE_DESCRIPTION("the module grant an page to the other doamin");
