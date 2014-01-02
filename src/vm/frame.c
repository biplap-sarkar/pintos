#include "vm/frame.h"
#include <stdio.h>
#include "vm/page.h"
#include "devices/timer.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include <list.h>

static struct list frames;			/* Single instance of Frame list */
static struct lock frames_lock;

/* Initialize the frame manager. */
void frame_init (void) {
	lock_init(&frames_lock);
	list_init(&frames);
}

/* Find the victim frame to be evicted */
void frame_evict(){
	//printf("find_frame_to_evict()..........\n");
	struct list_elem *e;
	struct frame *fr;
	lock_acquire(&frames_lock);
	e = list_begin(&frames);
	/* Use Second Chance to find the frame to be evicted. Find the frame
	 * which has accessed flat off. If it is on then turn it off and check
	 * the next frame
	 */
	 //printf("searching for victim.........\n");
	 //printf("evicting frame...........\n");
	while(true){
		if(e == list_end(&frames)){
			e = list_begin(&frames);
			continue;
		}
		fr = list_entry(e, struct frame, list_elem);
		if(pagedir_is_accessed(fr->th->pagedir, fr->page->addr) == true)
			pagedir_set_accessed(fr->th->pagedir, fr->page->addr, false);
		else{
			//printf("found victim frame %x........\n",fr->base);
			list_remove(e);
			swap_out(fr->page);
			fr->page->map_type = SWAP;
			fr->page->isloaded = false;
			pagedir_clear_page(fr->th->pagedir, fr->page->addr);
			//printf("frame %x evicted........\n",fr->base);
			palloc_free_page(fr->base);
			free(fr);
			lock_release(&frames_lock);
			return;
		}
		e = list_next(e);
	}
	
}

struct frame *frame_alloc(struct page *page){
	uint8_t *base = palloc_get_page (PAL_USER);
	//static int count=0;
	//count++;
	while(base == NULL){
		//printf("evicting frame.....3.....\n");
		frame_evict();
		base = palloc_get_page(PAL_USER);
		//count--;
	}
	//printf("got a frame %x......\n",base);
	struct frame *newframe = (struct frame *)malloc(sizeof(struct frame));
	newframe->base = base; 
	newframe->page = page;
	newframe->th = thread_current();
	lock_acquire(&frames_lock);
	list_push_back(&frames, &newframe->list_elem);
	lock_release(&frames_lock);
	return newframe;
}

/* Tries to allocate and lock a frame for PAGE.
   Returns the frame if successful, false on failure. */
/*
struct frame *
frame_alloc_and_lock (struct page *page) 
{
}
*/

/* Locks P's frame into memory, if it has one.
   Upon return, p->frame will not change until P is unlocked. */
/*
void
frame_lock (struct page *p) 
{
}
*/

/* Releases frame F for use by another page.
   F must be locked for use by the current process.
   Any data in F is lost. */
void frame_free (struct frame *f){
	//printf("frame_free().............\n");
	
	struct list_elem *e;
	lock_acquire(&frames_lock);
	for (e = list_begin(&frames); e != list_end(&frames); e = list_next(e)){
		struct frame *fte = list_entry(e, struct frame, list_elem);
		if (fte == f){
			list_remove(e);
			pagedir_clear_page(fte->th->pagedir, fte->page->addr);
			palloc_free_page(fte->base);
			free(fte);
			break;
		}
    }
	lock_release(&frames_lock);
}


/* Unlocks frame F, allowing it to be evicted.
   F must be locked for use by the current process. */
/*
void
frame_unlock (struct frame *f) 
{
}
*/
