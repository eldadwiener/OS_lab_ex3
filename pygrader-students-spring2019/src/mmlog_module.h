#ifndef _MMLOG_MODULE_H_
#define _MMLOG_MODULE_H_

#include <linux/ioctl.h>

#define MMLOG_MAGIC 'r'
#define MMLOG_SET_LOG_LEVEL _IOW(MMLOG_MAGIC, 0, int)


//
// Function prototypes
//
int mmlog_open(struct inode *, struct file *);

int mmlog_release(struct inode *, struct file *);

ssize_t mmlog_read(struct file *, char *, size_t, loff_t *);

ssize_t mmlog_write(struct file *, const char *, size_t, loff_t *);

int mmlog_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);

#endif
