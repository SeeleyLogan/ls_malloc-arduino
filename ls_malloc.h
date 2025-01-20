#ifndef LS_MALLOC_H
#define LS_MALLOC_H

#define MALLOC(...) ls_malloc(__VA_ARGS__)
#define FREE(...)   ls_free(__VA_ARGS__)

void* ls_malloc(unsigned short size);
void  ls_free(void *ptr);

#ifdef LS_MALLOC_IMPL

#include <Arduino.h>

extern short __heap_start;
extern void *__brkval;

#define magic_byte __heap_start

unsigned char *page_table;
unsigned short page_table_size;

bool ls_alloc_init = false;
void ls_init_page_table();
unsigned short find_first_free_pages(unsigned short pages);

#endif  // LS_MALLOC_IMPL

#endif  // LS_MALLOC_H
