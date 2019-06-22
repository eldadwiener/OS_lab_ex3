/* mmlog_module.c: Example char device module.
 *
 */
/* Kernel Programming */
#define MODULE
#define LINUX
#define __KERNEL__

#include <linux/kernel.h>  	
#include <linux/module.h>
#include <linux/fs.h>       		
#include <asm/uaccess.h>
#include <linux/errno.h>  
#include <linux/slab.h>  
#include <linux/sched.h>  

#include "mmlog_module.h"

#define MMLOG_DEVICE "mmlog_device"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Anonymous");

/* globals */
int mmlog_major = 0; /* will hold the major # of my device driver */

struct file_operations mmlog_fops = {
    .open = mmlog_open,
    .release = mmlog_release,
    .read = mmlog_read,
    .write = mmlog_write,
    .ioctl = mmlog_ioctl,
    .owner = THIS_MODULE,
};


int init_module( void )
{
    mmlog_major = register_chrdev( mmlog_major, MMLOG_DEVICE, &mmlog_fops );

    if( mmlog_major < 0 )
    {
	printk( KERN_WARNING "can't get dynamic major\n" );
	return mmlog_major;
    }

    return 0;
}


void cleanup_module( void ) {
	
    unregister_chrdev( mmlog_major, MMLOG_DEVICE);

    //
    // do clean_up();
    //
    return;
}


int mmlog_open( struct inode *inode, struct file *filp )
{
    return 0;
}


int mmlog_release( struct inode *inode, struct file *filp )
{
	return 0;
}


ssize_t mmlog_read( struct file *filp, char *buf, size_t count, loff_t *f_pos )
{
    return 0;
}


ssize_t mmlog_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
    return 0;
}


int mmlog_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch( cmd )
    {
    case MMLOG_SET_LOG_LEVEL:
	//
	// handle restart of the pointer.
	//
        set_malloc_free_log_level((int)arg);
	break;

    default:
	return -ENOTTY;
    }

    return 0;
}
