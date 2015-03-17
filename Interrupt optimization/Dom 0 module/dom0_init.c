#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "xenstore.h"
#include <unistd.h>
#include "libxl.h"
#include <time.h>
#include <assert.h>

int main()
{
	int a,b;	//菜单输入选项
	libxl_ctx * ctx;
	char path[100],cmd[100];
	xs_transaction_t xt,th;
	libxl_dominfo *info=NULL;
	static xentoollog_logger  *logger;
	int len,domid,num_vm,i;
	char *rd;
	struct xs_handle *xh = xs_daemon_open();
        if(!xh)
        {
                printf("error\n");
                return ;
        }

menu:
	printf("   输入数字选项选择对应功能：\n");
	printf("1、开启所有虚拟机的中断调节机制\n");
	printf("2、关闭所有虚拟机的中断调节机制\n");
	printf("3、开启单个虚拟机的中断调节机制\n");
	printf("4、关闭单个虚拟机的中断调节机制\n");
  	printf("5、退出程序\n");
	scanf("%d",&a);
	getchar();	
	
	switch(a)
	{
		case 1:
       			 /*获取当前虚拟机数目，为每个虚拟机都在Xenstore中创建键值*/
			logger =(xentoollog_logger*) xtl_createlogger_stdiostream(stderr, 4,  0);
        		libxl_ctx_alloc(&ctx,0,0,(xentoollog_logger*)logger);
    			info=libxl_list_domain(ctx,&num_vm);
   			libxl_ctx_free(ctx);
    			xtl_logger_destroy((xentoollog_logger*)logger);

        		//printf("%d\n",num_vm);
        		for( domid=1; domid<num_vm; domid++)
			{
				sprintf(path,"/local/domain/%d",domid);
       				strcat(path,"/interrupt/Switch");
        			//printf("%s\n",path);
				/*xenstore在被读取时，写入无效，Dom U中每隔一段周期会读取，为了保证写入有效，要进行检验*/		
				do{
					xt=xs_transaction_start(xh);
    					xs_write(xh,xt,path,"1",strlen("1"));
    					xs_transaction_end(xh,xt,false);		

					xt=xs_transaction_start(xh);
                			rd=xs_read(xh,xt,path,&len);
                			xs_transaction_end(xh,xt,false);
				}while( strcmp(rd,"1") );	

				sprintf(cmd,"xenstore-chmod %s b",path);
        			system(cmd);
				printf("Dom %d 中断调节已开启！\n",domid);
				
			}
			goto menu;
		case 2:
			logger =(xentoollog_logger*) xtl_createlogger_stdiostream(stderr, 4,  0);
                        libxl_ctx_alloc(&ctx,0,0,(xentoollog_logger*)logger);
                        info=libxl_list_domain(ctx,&num_vm);
                        libxl_ctx_free(ctx);
                        xtl_logger_destroy((xentoollog_logger*)logger);

                        //printf("%d\n",num_vm);
                        for( domid=1; domid<num_vm; domid++)
                        {
                                sprintf(path,"/local/domain/%d",domid);
                                strcat(path,"/interrupt/Switch");
                                //printf("%s\n",path);
                                /*xenstore在被读取时，写入无效，Dom U中每隔一段周期会读取，为了保证写入有效，要进行检验*/              
                                do{
                                        xt=xs_transaction_start(xh);
                                        xs_write(xh,xt,path,"0",strlen("0"));
                                        xs_transaction_end(xh,xt,false);

                                        xt=xs_transaction_start(xh);
                                        rd=xs_read(xh,xt,path,&len);
                                        xs_transaction_end(xh,xt,false);
                                }while( strcmp(rd,"0") );
		                printf("Dom %d 中断调节已关闭！\n",domid);

                        }
                        goto menu;
		case 3:
			printf("输入需要开启中端调节机制的虚拟机ID!\n");
        		scanf("%d",&domid);
        		getchar();
        		sprintf(path,"/local/domain/%d",domid);
        		strcat(path,"/interrupt/Switch");
        		//printf("%s\n",path);

	       		do{
                        		xt=xs_transaction_start(xh);
                                        xs_write(xh,xt,path,"1",strlen("1"));
                                        xs_transaction_end(xh,xt,false);
 
                                        xt=xs_transaction_start(xh);
                                        rd=xs_read(xh,xt,path,&len);
                                        xs_transaction_end(xh,xt,false);
	                }while( strcmp(rd,"1") );

        		sprintf(cmd,"xenstore-chmod %s b",path);
       			system(cmd);
			printf("Dom %d 中断调节已开启！\n",domid);
			goto menu;
		case 4:
			printf("输入需要关闭中端调节机制的虚拟机ID!\n");
                        scanf("%d",&domid);
                        getchar();
                        sprintf(path,"/local/domain/%d",domid);
                        strcat(path,"/interrupt/Switch");
                        //printf("%s\n",path);
   
                        do{
                                       xt=xs_transaction_start(xh);
                                       xs_write(xh,xt,path,"0",strlen("0"));
	                               xs_transaction_end(xh,xt,false);

		                       xt=xs_transaction_start(xh);
                                       rd=xs_read(xh,xt,path,&len);
                                       xs_transaction_end(xh,xt,false);
                        }while( strcmp(rd,"0") );
                        printf("Dom %d 中断调节已关闭！\n",domid);
                        goto menu;
		case 5:
			logger =(xentoollog_logger*) xtl_createlogger_stdiostream(stderr, 4,  0);
                        libxl_ctx_alloc(&ctx,0,0,(xentoollog_logger*)logger);
                        info=libxl_list_domain(ctx,&num_vm);
                        libxl_ctx_free(ctx);
                        xtl_logger_destroy((xentoollog_logger*)logger);

                        //printf("%d\n",num_vm);
                        for( domid=1; domid<num_vm; domid++)
                        {
                                sprintf(path,"/local/domain/%d",domid);
                                strcat(path,"/interrupt/Switch");
                                //printf("%s\n",path);
                                /*xenstore在被读取时，写入无效，Dom U中每隔一段周期会读取，为了保证写入有效，要进行检验*/              
                                do{
                                        xt=xs_transaction_start(xh);
                                        xs_write(xh,xt,path,"0",strlen("0"));
                                        xs_transaction_end(xh,xt,false);

                                        xt=xs_transaction_start(xh);
                                        rd=xs_read(xh,xt,path,&len);
                                        xs_transaction_end(xh,xt,false);
                                }while( strcmp(rd,"0") );
                                printf("Dom %d 中断调节已关闭！\n",domid);

                        }
			printf("所有虚拟机中断调节机制全部关闭并且已退出程序！\n");
	}
	return 0;
}
