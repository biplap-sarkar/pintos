#include "vm/swap.h"
#include <bitmap.h>
#include <debug.h>
#include <stdio.h>
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/synch.h"
#include "threads/vaddr.h"

/* The swap device. */
static struct block *swap_device;

/* Used swap pages. */
static struct bitmap *swap_bitmap;

/* Protects swap_bitmap. */
static struct lock swap_lock;

/* Number of sectors per page. */
#define PAGE_SECTORS (PGSIZE / BLOCK_SECTOR_SIZE)

/* Sets up swap. */
void
swap_init (void) 
{
}

/* Swaps in page P, which must have a locked frame
   (and be swapped out). */
void
swap_in (struct page *p) 
{
}

/* Swaps out page P, which must have a locked frame. */
bool
swap_out (struct page *p) 
{
}
