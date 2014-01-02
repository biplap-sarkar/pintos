#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <stdbool.h>
#include "threads/synch.h"
#include "threads/thread.h"

/* A physical frame. */
struct frame 
{
    void *base;                 /* Kernel virtual base address. */
    struct page *page;          /* Mapped process page, if any. */
    struct thread *th;			/* Thread owning the frame */
    /* ...............          other struct members as necessary */
    struct list_elem list_elem;
};

void frame_init (void);

struct frame *frame_alloc(struct page *);
struct frame *frame_alloc_and_lock (struct page *);
void frame_lock (struct page *);

void frame_free (struct frame *);
void frame_unlock (struct frame *);

#endif /* vm/frame.h */
