#define LS_MALLOC_IMPL
#include "ls_malloc.h"

inline void ls_init_page_table()
{
  // TODO: fix the math
  unsigned int dynamic_memory_size = RAMEND - (int) &__heap_start - 2;  // subtract 2 for magical byte at start of heap

  unsigned long sum_of_all_pages = (dynamic_memory_size * 32L / 33L) & ~3L;               // round down to nearest mult of 4
  page_table_size  = (unsigned short) (dynamic_memory_size - sum_of_all_pages + 3) & ~3;  // round up

  page_table = 2 + (char*) &__heap_start;

  memset(page_table, 0, page_table_size);   // page table must be all 0
  __brkval = page_table_size + page_table;  // __brkval is end of last used dynamic memory

  magic_byte = page_table_size;  // TODO: resize the magic byte 
  ls_alloc_init = true;
}

void* ls_malloc(unsigned short size)
{
  if (!ls_alloc_init)
    ls_init_page_table();

  unsigned short pages;

  // size to pages (minimum of 2 pages)
  if (size <= 8)
    pages = 2;
  else
    pages = ((size + 3) & ~3) / 4;

  unsigned short free_pages = find_first_free_pages(pages);

  // ensure we aren't corrupting the stack
  if ((free_pages + pages) * 4 + page_table_size + 2 > SP)
    return NULL;

  // set the starting and ending bits
  page_table[free_pages / 8] ^= 0x80 >> (free_pages & 7);
  page_table[(free_pages + pages) / 8] ^= 0x80 >> (((free_pages + pages) & 7) - 1);

  return (void*) (free_pages * 4 + page_table_size + 2);
}

// TODO: move magic_byte
void ls_free(void *ptr)
{
  unsigned short page = (((unsigned short) ptr) - 2 - page_table_size) / 4;

  page_table[page / 8] ^= 0x80 >> (page & 7);

  for (unsigned short byte = page / 8;; byte++)
    for (unsigned char bit = (page & 7) + 1; bit < 8; bit++)
      if (page_table[byte] & (0x80 >> bit))
      {
        page_table[byte] ^= 0x80 >> bit;
        return;
      }
}

inline unsigned short find_first_free_pages(unsigned short pages)
{
  bool allocated_memory = false;
  unsigned short free_continuous_pages = 0;

  for (unsigned short byte = 0; byte < magic_byte; byte++)
    for (unsigned char bit = 0; bit < 8; bit++)
    {
      bool bit_on = page_table[byte] & (0x80 >> bit);

      // find first instance of continuous free pages equal to pages
      if (bit_on && !allocated_memory && !free_continuous_pages)
        allocated_memory = !allocated_memory;
      else if (bit_on && allocated_memory)
        allocated_memory = !allocated_memory; 
      else if (!bit_on && !allocated_memory && free_continuous_pages < pages)
        free_continuous_pages++;
      else if (bit_on && free_continuous_pages > 0 && free_continuous_pages < pages)
      {
        free_continuous_pages = 0;
        allocated_memory = !allocated_memory;
      }
      else if (free_continuous_pages == pages)
        return byte * 8 + bit - free_continuous_pages;
    }

    // since no allocations can exist after where the magic byte tells us,
    // simply assume all pages after are free (because they are)
    return magic_byte * 8 - free_continuous_pages;
}
