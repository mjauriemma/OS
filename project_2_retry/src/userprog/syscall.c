#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);

/* Define sys_func for array and allow it to have 3 args */
typedef int sys_func (uint32_t, uint32_t, uint32_t);
/* List of syscalls - have 13 so array holds 13 */
static sys_func *sys_calls[13];

void exit (int status);
void halt (void);
int write (int fd, const void *buffer, unsigned length);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  sys_calls[SYS_HALT] = (sys_func *) halt;
  sys_calls[SYS_EXIT] = (sys_func *) exit;
  //sys_calls[SYS_EXEC] = (sys_func *) exec;
  //sys_calls[SYS_WAIT] = (sys_func *) wait;
  //sys_calls[SYS_CREATE] = (sys_func *) create;
  //sys_calls[SYS_REMOVE] = (sys_func *) remove;
  //sys_calls[SYS_OPEN] = (sys_func *) open;
  //sys_calls[SYS_FILESIZE] = (sys_func *) filesize;
  //sys_calls[SYS_READ] = (sys_func *) read;
  sys_calls[SYS_WRITE] = (sys_func *) write;
  //sys_calls[SYS_SEEK] = (sys_func *) seek;
  //sys_calls[SYS_TELL] = (sys_func *) tell;
  //sys_calls[SYS_CLOSE] = (sys_func *) close;
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{


	//handler system_call;
  char* file_name;
  int *call = f->esp;
  int *call_ret;

  /* Check user address */
  //check_valid_ptr(&call);

  call_ret = sys_calls[*call] (call + 1, call + 2, call + 3);
  f->eax = call_ret;

  return;
  //printf ("system call!\n");
  //thread_exit ();
}

void exit (int status) 
{
  //thread_current()->wait_info->exit_status = status;
  thread_exit ();
}

void halt (void)
{
  shutdown_power_off();
}

struct file * get_file (int fd)
{
	struct file_node * f;
	struct list_elem * e;
	bool success = false;
	for (e = list_begin(&thread_current()->file_list); 
		e != list_end(&thread_current()->file_list); e = list_next(e))
	{
		f = list_entry (e, struct file_node, elem);
		if (f->fd == fd) {
			success = true;
			break;
		}
	}
	if (success)
		return f->file;

	return NULL;
}

int write (int fd, const void *buffer, unsigned length)
{
  if (fd == 1)
    {
      putbuf (buffer, length);
      return length;
    }
  //lock_acquire(&file_sys_lock);
  struct file *f = get_file (fd);
  if (f == NULL)
    {
      //lock_release(&file_sys_lock);
      return -1;
    }
  int written = file_write (f, buffer, length);
  //lock_release(&file_sys_lock);
  return written;

}