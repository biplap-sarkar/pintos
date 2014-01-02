#include "vm/swap.h"
#include <bitmap.h>
#include <debug.h>
#include <stdio.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "devices/block.h"

/* The swap device. */
static struct block *swap_device;

/* Used swap pages. */
static struct bitmap *swap_bitmap;

/* Protects swap_bitmap. */
static struct lock swap_lock;

/* Number of sectors per page. */
#define PAGE_SECTORS (PGSIZE / BLOCK_SECTOR_SIZE)

/* Sets up swap. */
void swap_init (void) {
	lock_init(&swap_lock);		// Initialized lock for swap bitmap
	swap_device = block_get_role (BLOCK_SWAP);		// Initialized swap device
	if(swap_device == NULL){
		PANIC("Panicking... no swap device\n");
	}
	swap_bitmap = bitmap_create(block_size(swap_device)/PAGE_SECTORS);
	bitmap_set_all(swap_bitmap, FREE);		//Initialized swap bitmap
}

/* Swaps in page P, which must have a locked frame
   (and be swapped out). */
void swap_in (struct page *p) {
	block_sector_t swap_sector = p->page_sector; 
	lock_acquire(&swap_lock);
	if (bitmap_test(swap_bitmap,swap_sector) != IN_USE){
		lock_release(&swap_lock);
		PANIC ("Panicking... cannot swap in a free page sector");
    }
	int i;
	for (i = 0; i < PAGE_SECTORS; i++){
      block_read(swap_device, swap_sector*PAGE_SECTORS + i, p->frame->base + i * BLOCK_SECTOR_SIZE);
    }
    bitmap_flip(swap_bitmap, swap_sector);
	lock_release(&swap_lock);
}

/* Swaps out page P, which must have a locked frame. */
bool swap_out (struct page *p) {
	lock_acquire(&swap_lock);
	block_sector_t free_page_sector = bitmap_scan_and_flip(swap_bitmap,0,1,FREE);
	
	lock_release(&swap_lock);
	if(free_page_sector == BITMAP_ERROR){
		
		PANIC("Panicking... Out of space in swap :-( ");
	}
	p->page_sector = free_page_sector;
	free_page_sector = free_page_sector * PAGE_SECTORS;
	int i;
	for(i=0;i< PAGE_SECTORS;i++){
		block_write(swap_device, free_page_sector + i, p->frame->base + (i * BLOCK_SECTOR_SIZE));  
	}
	return true;
}
