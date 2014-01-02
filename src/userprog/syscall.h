#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

void syscall_init (void);
void syscall_exit (void);

/* Serializes file system operations. */
struct lock fs_lock;

#endif /* userprog/syscall.h */
