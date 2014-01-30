#ifndef _PROC_MISC_H
#define _PROC_MISC_H
#include <linux/proc_fs.h>


struct proc_dir_entry* procm_create(const char * proc_name);
void procm_delete(const char * proc_name);
int procm_write(struct file *file, const char __user *buffer, unsigned long count, void *data);

#endif //_PROC_MISC_H
