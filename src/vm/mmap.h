#ifndef VM_MMAP_H
#define VM_MMAP_H

#include <list.h>
#include "vm/page.h"
#define MAP_FAILED ((mapid_t) -1)

typedef int mapid_t;

/* Structure for keeping an mmap entry */
struct mmap_file{
	mapid_t mapping;
	struct page *page;
	struct list_elem list_elem;
};

#endif

