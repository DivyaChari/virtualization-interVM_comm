#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>

DEFINE_MUTEX(my_mutex);
DEFINE_MUTEX(my_mutex_shmem);

u8 data;
static int flag=0;


ssize_t read_proc_r(struct file *filp,char *buf,size_t count,loff_t *offp)
{
	static int finished=0;
		
	if(finished){
		printk(KERN_INFO "procfs_read: END\n");
		finished=0;
		//concats=0;
		return 0;		
	}
	
	finished =1;
	
	//if (mutex_lock_interruptible(&my_mutex)) {
	//mutex_lock(&my_mutex);
		
		//return -ERESTARTSYS;
	//}
	
	
	sprintf(buf, "%c\n" , data);	
	printk("(%lu)process %i (%s) reads value : %u \n", count, current->pid,current->comm, data);
	
	mutex_unlock(&my_mutex);		
	
	return count;
}

ssize_t write_proc_w(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	
	if (mutex_lock_interruptible(&my_mutex)) {
		return -ERESTARTSYS;
	}
	//mutex_lock(&my_mutex);

	sscanf(buf, "%c", &data);
	printk("process %i (%s) writes value : %u \n", current->pid,current->comm, data, buf);	

	//mutex_unlock(&my_mutex);
	
	return count;		
} 

ssize_t read_proc_r_shmem(struct file *filp,char *buf,size_t count,loff_t *offp)
{
	static int finished_shmem=0;
		
	if(finished_shmem){
		printk(KERN_INFO "procfs_read: END\n");
		finished_shmem=0;
		//concats=0;
		return 0;		
	}
	
	finished_shmem =1;
	

	sprintf(buf, "%c\n" , flag);	
	//printk("(%lu)process %i (%s) reads value : %u \n", count, current->pid,current->comm, flag);
	
	mutex_unlock(&my_mutex_shmem);		
	
	return count;
	
}

ssize_t write_proc_w_shmem(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	
	if (mutex_lock_interruptible(&my_mutex_shmem)) {
		return -ERESTARTSYS;
	}
	
	sscanf(buf, "%c", &flag);
	//printk("process %i (%s) writes value : %u \n", current->pid,current->comm, flag, buf);	
	
	return count;		
	
}


struct file_operations proc_fops = {
read: read_proc_r,
write: write_proc_w
};

struct file_operations proc_fops_shmem = {
read: read_proc_r_shmem,
write: write_proc_w_shmem
};

static int __init bridge_comm_init(void)
{
	printk("Setting up intercomm_vm");
	proc_create("bridge_comm_VM",0777, NULL, &proc_fops); //0644
	proc_create("shmem_comm_VM",0777, NULL, &proc_fops_shmem); //0644
	return 0;
}

static void __exit bridge_comm_exit(void)
{	
	remove_proc_entry("bridge_comm_VM",NULL);
	remove_proc_entry("shmem_comm_VM",NULL);	
	
	printk("Goodbye \n");
}

module_init(bridge_comm_init);
module_exit(bridge_comm_exit);

//MODULE_AUTHOR("Anway");
//MODULE_DESCRIPTION("VM  communication bridge");
//MODULE_VERSION(VERSION);
MODULE_LICENSE("GPL");
