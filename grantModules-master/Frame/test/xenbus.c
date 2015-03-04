#include <xen/xenbus.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include<linux/slab.h>
#include "dbg_kernel.h"

static char *result = NULL;
int init_module(void)
{
	int rc;

	struct xenbus_transaction trans;  

	log_info("init the module");
	rc = xenbus_transaction_start(&trans);  
	 check(rc==0, "transaction start failed");

	xenbus_write(trans, "example2", "Alice", "Hello, nice to meet you,again");  
	xenbus_transaction_end(trans, 0);  
	return 0;

error:
        return 1;
}

void cleanup_module(void)
{
	struct xenbus_transaction trans;
	int len;
	int rc;

	log_info("going cleanup the module");

        rc = xenbus_transaction_start(&trans);
	check(rc==0, "transaction start failed");

        result =xenbus_read(trans, "example2", "Alice", &len);
	check(result, "read failed");

        xenbus_transaction_end(trans, 0);

	log_info("read from the xenstore is %s",result);
	kfree(result);
error:
	return;
}

MODULE_LICENSE("GPL");
