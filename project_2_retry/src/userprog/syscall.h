#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <list.h>

struct file_node {
	int fd;
	struct list_elem elem;
	struct file *file;
};

void syscall_init (void);

#endif /* userprog/syscall.h */
