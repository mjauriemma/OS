#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
//#include "threads/synch.h"

typedef int tid_t;

struct wait_info {
	char *cmd_line;
	struct list_elem *elem;
};


tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
