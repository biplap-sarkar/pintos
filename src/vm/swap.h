#ifndef VM_SWAP_H
#define VM_SWAP_H 1

#include <stdbool.h>
#define FREE 0
#define IN_USE 1

struct page;
void swap_init (void);
void swap_in (struct page *);
bool swap_out (struct page *);

#endif /* vm/swap.h */
