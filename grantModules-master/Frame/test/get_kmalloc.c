#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "dbg_kernel.h"

char *p= NULL;
int init_module(void)
{
	int s=100;
	p = kmalloc(sizeof(char)*100, GFP_KERNEL);
	check(p!=NULL, "kmalloc sizeof(char)*100 failed");

	log_info("p is point with 100 chars.");
	return 0;

error:
	return 1;
}

void cleanup_module(void)
{
	log_info("going to cleanup the module");
	if(p)
		kfree(p);
}
