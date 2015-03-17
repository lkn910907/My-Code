#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "xenstore.h"

unsigned long int num_old[16];  //暂定虚拟机中最多分配16个VCPU.记录更新前，每个CPU的中断负载
unsigned long int num_new[16];	//记录更新后，每个CPU的中断负载
unsigned long int num[16]; 	//记录sleep时间内，即更新前后每个CPU的实时中断负载
unsigned int pick[16];		//保存被选中与网卡中断建立亲和性的CPU编号
char cpu_affinity[][5]={"0001","0002","0004","0008","0010","0020","0040","0080","0100","0200","0400","0800","1000","2000","4000","8000"};
char *pick_affinity[5];
int interrupt[5];	//暂取虚拟机里最多5个网口,保存每个网口的中断号
char *affinity_old[5];  //保留未开启中断调节机制之前中断的CPU亲和度信息
char *affinity_new[5]; 	//开启中断调节机制之后中断的CPU亲和度信息

int M;		//获取虚拟机中实际分配的VCPU数目
int N;		//记录虚拟机中使用的网口数目

/*统计系统中实际使用的物理CPU数目*/
int count_CPU()
{
	FILE *fp;
	char buf[256]={0};
	char *p,*s;
	int num=0;

	if (!(fp = fopen("/proc/cpuinfo", "r")))
        {
                printf("fopen failed.\n");
                return -1;
        }

	while( fgets(buf, 255, fp)!= NULL )
	{
		//printf("%s",buf);
		if( strstr(buf, "processor") != 0)
			num++;
	}
	//printf("虚拟机中的逻辑处理器数目为%d!\n",num);
	return num;

}

/*统计虚拟机中各个CPU上的中断负载，网卡中断除外，因为CPU上的其它中断负载轻重决定网卡中断绑定在哪个CPU上*/
int statistics(void)
{
	char *p,*s;
	FILE *fp;
        int i=0,j=0;
	char buf[256]={0};
	if (!(fp = fopen("/proc/interrupts", "r")))
        {
                printf("fopen failed.\n");
                return -1;
        }
        fgets(buf, 255, fp);    //过滤掉第一行的CPU栏信息

	while( fgets(buf, 255, fp)!= NULL )
        {
               	//printf("%s",buf);
		p=buf;
		/*不统计网卡中断的负载*/
		if( strstr(p, "eth") != 0)
			continue;

                /*跳过空格*/
                while (isspace((int) *p))
                        p++;
                /*只对有中断号的中断统计，对于NMI以及LOC等忽略*/
                s=p;
		if (!(*s>='0' && *s<='9'))
			break;

                while (*p && *p != ':')
                        p++;
                *p = '\0';
                p++;
		//printf("%s",p);
		/*获取每个CPU上的中断次数*/
		for(i=0;i<M;i++)
		{
			while (isspace((int) *p))
                        	p++;
			s=p;
			while (*p && *p != ' ')
                        	p++;
                	*p = '\0';
                	p++;
			num_old[i] += atoi(s);
		}

	}
//	for(i=0;i<M;i++)
//		printf("CPU %d上的中断次数为:%ld\n",i,num_old[i]);
	fclose(fp);
}

/*更新这10S内（sleep了10s）各个CPU的实时中断负载，网卡中断除外，因为CPU上的其它中断负载轻重决定网卡中断绑定在哪个CPU上*/
int calculate(void)
{
	FILE *fp;
	char *p,*s;
        int i=0,j=0;	//j用于调试计数，检查是否统计了系统中所有的中断
        char buf[256]={0};
	if (!(fp = fopen("/proc/interrupts", "r")))
        {
                printf("fopen failed.\n");
                return -1;
        }
        fgets(buf, 255, fp);    //过滤掉第一行的CPU栏信息

	while( fgets(buf, 255, fp)!= NULL )
        {
                //printf("%s",buf);
                 p=buf;

		/*不考虑网卡中断*/
		if( strstr(p, "eth") != 0)
			continue;

                /*跳过空格*/
                while (isspace((int) *p))
                        p++;
                /*只对有中断号的中断统计，对于NMI以及LOC等忽略*/
                s=p;
                if (!(*s>='0' && *s<='9'))
                        break;

                while (*p && *p != ':')
                        p++;
                *p = '\0';
                p++;
                //printf("%s",p);
                /*获取每个CPU上的中断次数,减去之前的统计，以获得10s之内的实时中断负载*/
                for(i=0;i<M;i++)
                {
                        while (isspace((int) *p))
                                p++;
                        s=p;
                        while (*p && *p != ' ')
                                p++;
                        *p = '\0';
                        p++;
                        num_new[i] += atoi(s);
		}
		j++;

        }
	printf("统计了%d个中断\n",j);
	j=0;

	for(i=0;i<M;i++)
	{
		num[i] =  num_new[i] -  num_old[i];
		printf("CPU %d原有中断负载为%ld，此时中断负载为%ld，实时中断负载为%ld\n",i,num_old[i],num_new[i],num[i]);
		num_old[i] = num_new[i];
		num_new[i]=0;
	}
	fclose(fp);
}

/*判断是否要重新设置网卡中断的亲和度，如果每个CPU上中断负载差不多，便省略此次的调节*/
int judge(void)
{
	unsigned long int min,max;	//获取最大实时CPU中断负载与最小实时CPU中断负载
	int i=0;

	min=num[0];
	max=num[0];
	for(i=1; i<M; i++)
	{
		if(min > num[i])
			min= num[i];
		if(max < num[i])
			max= num[i];
	}
	printf("最大实时中断负载为%ld,最小实时中断负载为%ld\n",max,min);
	if( (max-min) >50 )
		return 1;
	else
		return 0;
}

/*根据CPU上的实时中断负载，从中选取负载最小的N个CPU建立其与网卡中断的亲和性*/
int pick_CPU(int N)
{
	int i,j,a;
	/*数组默认先按顺序保存每一个CPU的编号*/
	for(i=0; i<M; i++)
		pick[i]=i;
	for(j=0; j<N; j++)
	{
		for(i=M-1; i>j; i--)
		{
			if(num[i] < num[i-1])
			{
				a = pick[i];
				pick[i] = pick[i-1];
				pick[i-1] = a;
			}
		}
	}
	for(i=0; i<N; i++)
		printf("找到迁移网卡中断的目标CPU%d\n",pick[i]);
}

/*根据pick数组中的目标CPU序号找到合适的CPU位图并建立网口中断与目标CPU的亲和性*/
int set_affinity(int N)
{
	int i,j;
	char cmd[200];  //终端命令
        char path[5][100];
	for(i=0; i<N; i++)
	{
		j = pick[i];
		pick_affinity[i] = cpu_affinity[j];
		printf("新的亲和性位图为%s\n",pick_affinity[i]);
		sprintf( path[i],"/proc/irq/%d/smp_affinity",interrupt[i]);
                sprintf(cmd,"echo \"%s\"> %s",pick_affinity[i],path[i]);
                printf("%s\n",cmd);
                system(cmd);
	}
	
}

/*获取实际使用的网口数目以及每一个网口的中断号*/
int get_eth_CPU_number(void )
{
	FILE *fp;
        int i=0;
	char buf[256];
	char *p,*s;
	if (!(fp = fopen("/proc/interrupts", "r")))
        {
                printf("fopen failed.\n");
                return -1;
        }
        fgets(buf, 255, fp);    //过滤掉第一行的CPU栏信息
        while( fgets(buf, 255, fp)!= NULL )
        {
                fgets(buf, 255, fp);
                //printf("%s",buf);
                p=buf;
                /*跳过空格*/
                while (isspace((int) *p))
                        p++;
                /*获取中断号，保存到s中*/
                s=p;
                while (*p && *p != ':')
                        p++;
                *p = '\0';
                p++;
                //printf("%s\n",s);
                //printf("%s",p);
                /*找到网卡的中断号*/
                if( strstr(p, "eth") == 0)
                        continue;
                printf("第%d个网口的中断号为%s\n",i+1,s);
                //printf("%s",p);
                /*字符串形式转换成整型*/
                interrupt[i]= atoi(s);
                //printf("使用的网口中断号有%d\n",interrupt[i]);
		i++;
        }
        fclose(fp);
       	return i;
}

/*获取开启中断调节机制之前，每一个网卡中断绑定的CPU位图*/
int get_old_affinity()
{
	FILE *fp;
        int j=0;
        char buf[256];
        char *p,*s;
	char path[5][100];
	
	for(j=0;j<N;j++)
        {
                sprintf( path[j],"/proc/irq/%d/smp_affinity",interrupt[j]);
                //printf("%s\n",path[j]);
                if (!(fp = fopen(path[j], "r")))
                {
                        printf("fopen failed.\n");
                        return -1;
                }
                fgets(buf, 255, fp);
                //printf("%s\n",buf);
                p=buf;
                /*跳过空格*/
                while (isspace((int) *p))
                        p++;
                s=p;
                /*该行有换行符，要过滤掉*/
                while (*p && *p!='\n')
                        p++;
                *p = '\0';
                //printf("%s\n",s);
                affinity_old[j]=s;
                printf("第%d个网口的CPU亲和度为%s\n",j+1,affinity_old[j]);
        }
        fclose(fp);
}

int main()
{
	FILE *fp;
	int i=0,j=0,len;
	char* rd;	//读取中断调节机制开关键值
	char buf[256]={0};
	char cmd[200];	//终端命令
	char path[5][100];
	const char *pathn="interrupt/Switch";
	char *p=NULL,*s=NULL;
	xs_transaction_t xt;
    	struct xs_handle *xh = xs_domain_open();

	/*获取虚拟机中的逻辑处理器数目*/
	M = count_CPU();
	for(j=0;j<M;j++)
		num_old[j]=0;
	printf("虚拟机中的逻辑处理器数目为%d\n",M);

	if(!xh)
    	{
    		/*无法连接到XenStore*/
		printf("error\n");
		return ;
    	}
	
	/*将网口中断保存至interrupt数组中，并获取使用的中断数目*/
	N = get_eth_CPU_number();
	printf("使用的网口中断数目为%d\n",N);	

	/*获取开启中断调节机制之前，每一个网卡中断绑定的CPU位图*/
	get_old_affinity();

	/*在开始根据CPU的中断负载进行调节前，要先获得每个CPU上的历史中断负载，以便后续比较*/
	statistics();

	/*因为statistics函数与calculate函数执行间隔很短，所以第一次的中断负载更新很小*/

	/*监控中断调节机制开关键值*/
	while(1)
	{
		xt=xs_transaction_start(xh);
        	if(!xt)
       		{
            		printf("error\n");
            		return -1;
        	}
		rd = xs_read(xh,xt,pathn,&len);
		xs_transaction_end(xh,xt,false);
		//printf("%s",rd);
		if( !strcmp(rd,"1"))
		{
			printf("中断调节机制已开启\n");
			/*开启时，实时更新CPU上的中断负载记录，并根据负载轻重迁移网卡中断*/
			calculate();
			i=judge();
			/*只有在CPU中断负载有一定差距时才考虑重新建立网卡中断的亲和性*/
			if(i==1)
			{
				pick_CPU(N);
				set_affinity(N);
			}
			else
				printf("各个CPU上的实时中断负载相近，跳过此次调节\n");

		}
		else
		{
			/*检测到关闭中断调节机制，恢复中断原始的CPU位图*/
         		for(j=0;j<N;j++)
		        {
		                 //affinity_old[j]="0001";       为了检查是否有效的测试
	             		sprintf( path[j],"/proc/irq/%d/smp_affinity",interrupt[j]);
		                sprintf(cmd,"echo \"%s\"> %s",affinity_old[j],path[j]);
                 		//printf("%s\n",cmd);
                 		system(cmd);
	    		 }
			printf("中断调节机制已关闭\n");
		}
		sleep(10);
	}

	return 0;
}
