#ifndef __dbg_kernel_h__
#define __dbg_kernel_h_
#include <linux/types.h>
#include <linux/kernel.h>

#define debug(M, ...) printk("DEBUG %s:%d:" M "\n",__FILE__, __LINE__, ##__VA_ARGS__)


#define log_err(M,...) printk("[ERROR] %s:%d: " M "\n", __FILE__, __LINE__,  ##__VA_ARGS__)

#define log_warn(M, ...) printk("[WARN] %s:%d: " M "\n", __FILE__, __LINE__,  ##__VA_ARGS__)

#define log_info(M,...) printk("[INFO] %s:%d " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M,...)  if(!(A)){ log_err(M, ##__VA_ARGS__); goto error;}
#endif
