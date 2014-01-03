#include "vm/page.h"
#include <stdio.h>
#include <string.h>
#include "vm/frame.h"
#include "vm/swap.h"
#include "filesys/file.h"
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "userprog/process.h"
#include "threads/vaddr.h"
#include "userprog/syscall.h"



/* Destroys a page, which must be in the current process's
   page table.  Used as a callback for hash_destroy(). */
static void destroy_page (struct hash_elem *p_, void *aux UNUSED){
	struct page *p = hash_entry(p_, struct page, hash_elem);
	if(p->isloaded == true ){
		frame_free(p->frame);
	}
	free(p);
}

/* Destroys the current process's page table. */
void page_exit (void) {
	hash_destroy(&thread_current()->spt, destroy_page);
}

/* Returns the page containing the given virtual ADDRESS,
   or a null pointer if no such page exists.
   Allocates stack pages as necessary. */
struct page *
page_for_addr (const void *address) {
	const void *base = pg_round_down(address);
	struct page p;
	struct hash_elem *e;
	p.addr = base;
	e = hash_find (&thread_current()->spt, &p.hash_elem);
	return e != NULL ? hash_entry (e, struct page, hash_elem) : NULL;
}

/* Locks a frame for page P and pages it in.
   Returns true if successful, false on failure. */
static bool
do_page_in (struct page *p)
{
}

/* Faults in the page containing FAULT_ADDR.
   Returns true if successful, false on failure. */
bool
page_in (void *fault_addr) {
	struct page *p = page_for_addr(fault_addr);
	if(p==NULL)
		return false;
	switch(p->map_type){
		case FILE:{
			return page_in_from_file(p);
		}
		case SWAP:{
			return page_in_from_swap(p);
		}
	}
}

/* Faults in the page containing FAULT_ADDR from file.
   Returns true if successful, false on failure. */
bool
page_in_from_file (struct page *p) {
	/* Get a page of memory. */
	 struct frame *newframe = frame_alloc(p);
	 if(newframe == NULL)
		return false;
	 p->frame = newframe;
     /* Load this page. */
     lock_acquire(&fs_lock);
     if (file_read_at(p->file, newframe->base, p->read_bytes, p->ofs) != (int) p->read_bytes){
		lock_release(&fs_lock);
		palloc_free_page (newframe->base);
        return false; 
     }
     lock_release(&fs_lock);
     memset (newframe->base + p->read_bytes, 0, p->zero_bytes);
     

     /* Add the page to the process's address space. */
     if (!install_page (p->addr, newframe->base, p->writable)) {
		palloc_free_page (newframe->base);
		return false; 
     }
     p->isloaded = true;
     return true;
}

/* Faults in the page containing FAULT_ADDR from swap.
   Returns true if successful, false on failure. */
bool
page_in_from_swap (struct page *p){
	//printf("page_in_from_swap().............\n");
	p->frame = frame_alloc(p);
	swap_in(p);
	if(!install_page(p->addr, p->frame->base, p->writable)){
		frame_free(p->frame);
		return false;
	} 
	p->isloaded = true;
	return true;
}

/* Grows the stack. Fails if virtual address is already mapped */
bool page_grow_stack(void *vaddr, void *esp){
	if(vaddr < esp-32)
		return false;
	//printf("page_grow_stack().................\n");
	struct page *newpage = page_allocate(pg_round_down(vaddr), true);
	if(newpage == NULL)
		return false;
	// allocate a frame
	newpage->frame = frame_alloc(newpage);
	
	if(!install_page(newpage->addr, newpage->frame->base, newpage->writable)){
		//printf("install error............\n");
		frame_free(newpage->frame);
		free(newpage);
		return false;
	}
	//printf("stack page installed...........\n");
	newpage->isloaded = true;
	return true;
}

/* Evicts page P.
   P must have a locked frame.
   Return true if successful, false on failure. */
bool
page_out (struct page *p) 
{
}

/* Returns true if page P's data has been accessed recently,
   false otherwise.
   P must have a frame locked into memory. */
bool
page_accessed_recently (struct page *p) 
{
}

/* Adds a mapping for user virtual address VADDR to the page hash
   table.  Fails if VADDR is already mapped or if memory
   allocation fails. */
struct page *
page_allocate (void *vaddr, bool writable) {
	struct page *newpage = (struct page *)malloc(sizeof(struct page));
	newpage->addr = vaddr;
	newpage->writable = writable;
	newpage->frame = NULL;
	newpage->isloaded = false;
	if(hash_find(&thread_current()->spt, &newpage->hash_elem)!=NULL)
		return NULL;
	hash_insert(&thread_current()->spt, &newpage->hash_elem); 
	return newpage;	
}

/* Adds a file mapping for user virtual address VADDR to the page hash
   table.  Fails if VADDR is already mapped or if memory
   allocation fails. */
bool page_allocate_file (void *vaddr, struct file *file, off_t ofs,
	uint32_t read_bytes, uint32_t zero_bytes, bool writable){
	struct page *newpage = page_allocate(vaddr, writable);
	if(newpage == NULL)
		return false;
	newpage->map_type = FILE;
	newpage->file = file;
	newpage->ofs = ofs;
	newpage->read_bytes = read_bytes;
	newpage->zero_bytes = zero_bytes;	
	return true;
}

/* Evicts the page containing address VADDR
   and removes it from the page table. */
void
page_deallocate (void *vaddr) {
	//printf("deallocating page %x........\n",vaddr);
	struct page *p = page_for_addr(vaddr);
	struct hash_elem *e = hash_delete(&thread_current()->spt, &p->hash_elem);
	if(e != NULL){
		if(p->isloaded == true){
			palloc_free_page(p->frame->base);
		}
		if(pagedir_get_page(thread_current()->pagedir, p->addr) != NULL)
			pagedir_clear_page(thread_current()->pagedir, p->addr);
	}
	free(p);
}

/* Returns a hash value for the page that E refers to. */
unsigned
page_hash (const struct hash_elem *e, void *aux UNUSED) {
	const struct page *p = hash_entry (e, struct page, hash_elem);
	return hash_bytes (&p->addr, sizeof p->addr);
}

/* Returns true if page A precedes page B. */
bool
page_less (const struct hash_elem *a_, const struct hash_elem *b_,
           void *aux UNUSED) {
	const struct page *a = hash_entry (a_, struct page, hash_elem);
	const struct page *b = hash_entry (b_, struct page, hash_elem);
	return a->addr < b->addr;
}

/* Tries to lock the page containing ADDR into physical memory.
   If WILL_WRITE is true, the page must be writeable;
   otherwise it may be read-only.
   Returns true if successful, false on failure. */
bool
page_lock (const void *addr, bool will_write) 
{
}

/* Unlocks a page locked with page_lock(). */
void
page_unlock (const void *addr) 
{
}
 
/* Initializes supplementary page table */
void page_table_init (struct hash *spt) {
  hash_init (spt, page_hash, page_less, NULL);
}
