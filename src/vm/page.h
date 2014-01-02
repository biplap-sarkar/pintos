#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "devices/block.h"
#include "filesys/off_t.h"
#include "threads/synch.h"

/* Maximum size of process stack, in bytes. */
#define STACK_MAX (1024 * 1024)

/* Virtual page. */
enum mapping{
	FILE,
	SWAP,
	MMAP
};
struct page 
  {
     void *addr;                 /* User virtual address. */
	 struct hash_elem hash_elem; 
     struct frame *frame;        /* Page frame. */
	
     /* ...............          other struct members as necessary */ 
     bool isloaded; 
     enum mapping map_type;
     // members for keeping file information
     struct file *file;
     off_t ofs;
     uint32_t read_bytes;
     uint32_t zero_bytes;
     bool writable;
     
     // members for keeping swap information 
	 block_sector_t page_sector; 

  };

void page_exit (void);

struct page *page_allocate (void *, bool read_only);
bool page_allocate_file (void *vaddr, struct file *file, off_t ofs,
	uint32_t read_bytes, uint32_t zero_bytes, bool writable);
void page_deallocate (void *vaddr);

bool page_grow_stack(void *vaddr);
struct page *page_for_addr (const void *address);
void page_table_init (struct hash *spt);
bool page_in (void *fault_addr);
bool page_in_from_file (struct page *p);
bool page_in_from_swap (struct page *p);
bool page_out (struct page *);
bool page_accessed_recently (struct page *);

bool page_lock (const void *, bool will_write);
void page_unlock (const void *);

hash_hash_func page_hash;
hash_less_func page_less;

#endif /* vm/page.h */
