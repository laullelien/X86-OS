#ifndef ___USER_ALLOC_H___
#define ___USER_ALLOC_H___

#include "stdint.h"
#include "userspace_apps.h"

void *user_alloc();
void user_free(void *ptr);

typedef struct 
{
    uint32_t *address;
    uint32_t *code_page_table; // 256
    uint32_t *stack_page_table; // 511
    uint32_t *shm_page_table; // 512

} PageDirectory;


uint32_t *init_page_directory(PageDirectory *ptr);
void map_user_code(PageDirectory *pagedir, const struct uapps *uapp);


extern unsigned pgdir[]; // initial kernel pagedir address

#endif