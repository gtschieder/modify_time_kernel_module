#include<linux/init.h>
#include<linux/module.h>
#include<linux/cdev.h>
#include<linux/fs.h>
#include<linux/errno.h>
#include<linux/uaccess.h>
#include<linux/namei.h>
#include<linux/path.h>
#include<linux/mount.h>
#include<linux/version.h>
#include<linux/limits.h>
#include<linux/slab.h>

#define IOCONTROL_CMD _IOW(0xCD, 0x19, char*) // IO write

/* 
Prevents error: 
Warning: loading xxxxxx.ko will 
taint the kernel: no license
*/
MODULE_LICENSE("GPL"); 

// struct to store the path data
struct path_data {
	char* pathname;
	size_t pathsize;
	struct timespec time;
	char* command;
}; 

// Change the last modified time, last date accessed, and last date changed of file
static long change_time(struct inode* inode, struct path* path, struct timespec* time, char* command){
	if(inode->i_flags & S_NOCMTIME)
	{
		return -ENOTSUPP;
	}
	
	#ifdef HAS_UNMAPPED_ID
	// Return error if inode has unmapped ID
	if(HAS_UNMAPPED_ID(inode))
	{
		return -ENOTSUPP;
	}
	#endif
	
	// Return error if inode doesn't have created, modified, or accessed  time properties
	if(IS_NOCMTIME(inode) || IS_NOATIME(inode))
	{
		return -ENOTSUPP;
	}
	
	if(mnt_want_write(path->mnt) != 0)
	{
		return -ECANCELED;
	}
	
    // Change Last Modified time
	if(strcmp(command, "LM") == 0) 
	{
		printk(KERN_INFO "Kernel modifying LAST MODIFIED\n");
		inode->i_mtime = *time; // Write time to inode
	} 
    // Change last Date Accessed time
	else if (strcmp(command, "DA") == 0) 
	{
		printk(KERN_INFO "Kernel modifying DATE ACCESSED\n");
		inode->i_atime = *time; // Write time to inode
	} 
    // Change last Changed Date time
	else if (strcmp(command, "DC") == 0) 
	{
		printk(KERN_INFO "Kernel modifying DATE CHANGED\n");
		inode->i_ctime = *time;	// Write time to inode
	}

	mark_inode_dirty_sync(inode);
	mnt_drop_write(path->mnt);
	
	return 0;
}

long iocontrol(struct file* filep, unsigned int cmd, unsigned long ptr){
	char* pathname;
	struct path path;
	struct inode* inode;
	struct path_data data;	
	
	// Check that the command is a write
	if(cmd != IOCONTROL_CMD)
	{
		return -EINVAL;
	}
	
	// Copy the time data from the user space
	if(copy_from_user(&data, (const void*) ptr, sizeof(struct path_data)))
	{
		return -EINVAL;
	}
	
	// Check that the path isn't too long
	if(data.pathsize > PATH_MAX)
	{
		return -ENAMETOOLONG;
	}
	
	// Create space in memory that's smaller than a page size
	pathname = kmalloc(data.pathsize, GFP_KERNEL);
	
	// Copy the path name from the user space
	if(copy_from_user(pathname, (const char*) data.pathname, data.pathsize))
	{
		return -EFAULT;
	}
	
	// Store pathname into path and get inode information
	if(kern_path(pathname, LOOKUP_FOLLOW, &path) < 0)
	{
		return -ENOENT;
	}
	
	inode = path.dentry->d_inode;
	return change_time(inode, &path, &data.time, data.command);
}

struct {
	struct file_operations operations;
	dev_t dev;
	struct cdev chardev;
} time_device = {
	.operations = {
		.owner = THIS_MODULE,
		.unlocked_ioctl = iocontrol
	}
};

// Initialize time device
static int __init time_init(void){
	int error;
	error = alloc_chrdev_region(&time_device.dev, 0, 1, "timem");
	
	if(error)		// If there's an error, go to the fail catch
	{
		goto fail;
	}
	
	cdev_init(&time_device.chardev, &time_device.operations);
	time_device.chardev.owner = THIS_MODULE;
	error = cdev_add(&time_device.chardev,time_device.dev, 1);
	
	if(error)		// If there's an error, go to the fail catch
	{
		goto fail;
	}
	
	printk(KERN_INFO "Time modifier device initialized\n");
	return 0;
	
	// error fail catch
	fail:
	printk(KERN_INFO "Cannot initialize time modifier device\n");
	return -1;
}

// Exit time modifier (this will happen on unload action)
static void __exit time_exit(void){
	printk(KERN_INFO "Time modifier device unmounted\n");
	cdev_del(&time_device.chardev);
	unregister_chrdev_region(time_device.dev, 1);
}

module_init(time_init);
module_exit(time_exit);
