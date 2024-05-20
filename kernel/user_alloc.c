#include "user_alloc.h"
#include "mem.h"
#include "stddef.h"
#include "early_mm.h"
#include "userspace_apps.h"
#include "mem.h"
#include "debug.h"
#include "string.h"

static unsigned long current_index =  1<<26;
static unsigned long page_size = 1<<12;


/* Page directory */
extern unsigned pgdir[];
#define PAGE_DIR_FLAGS     0x00000003u


/* Page tables */
extern unsigned pgtab[];
#define PAGE_TABLE_RO      0x000000001u
#define PAGE_TABLE_RW      0x000000003u
#define PAGE_FLAG_USER     0x000000004u



void *user_alloc() {
    // Un allocateur très basique, on ne libère jamais rien non plus
    current_index += page_size; // ignore address 64M
    return (void *) current_index;

}

void user_free(void *ptr) {
    // Avec un allocateur plus évolué on libère ici le bloc de 4Ko donné
    ptr = ptr;
}

static uint32_t *user_alloc_full_page_table() {
    uint32_t *address = (uint32_t*) (current_index+page_size);
    current_index += 1025*page_size;
    for (uint32_t i = 0; i < 1024; i++) {
        address[i] = (((uint32_t)address) + (i+1) * 0x1000) | PAGE_TABLE_RW| PAGE_FLAG_USER;
    }

    return address;
}


static void init_kernel_pages(uint32_t pagedir[])  {
    for (int i = 0; i < 64; i++) {
        pagedir[i] = (pgdir[i] & 0xfffffff0) | PAGE_TABLE_RW; 
    }

    for (int i = 64; i < 1024; i++) {
        pagedir[i] = 0;
    }
}

static void init_shm_pages(PageDirectory *ptr){
    uint32_t *address = (uint32_t*) user_alloc();
    
    for (uint32_t i = 0; i < 1024; i++) {
        address[i] = 0;
    }

    ptr->shm_page_table = address;
    ptr->address[512] = ((uint32_t)address) | PAGE_DIR_FLAGS | PAGE_FLAG_USER;
}

static uint32_t *init_stack_pages(PageDirectory *pagedir) {
    // retourne l'adresse du haut de la pile
    uint32_t *stack_address = user_alloc_full_page_table();
    pagedir->stack_page_table = stack_address;

    pagedir->address[511] = ((uint32_t) stack_address) | PAGE_DIR_FLAGS| PAGE_FLAG_USER;
    
    return stack_address + ((1025*0x1000)>>2)-1;
}

static void init_code_pages(PageDirectory *ptr) {
    uint32_t *code_address = user_alloc_full_page_table();
    ptr->code_page_table = code_address;
    ptr->address[256] = ((uint32_t)code_address) | PAGE_DIR_FLAGS | PAGE_FLAG_USER;
}

void map_user_code(PageDirectory *pagedir, const struct uapps *uapp) {
    if (uapp == NULL) {
        return;
    }

    assert((uint32_t)uapp->end- (uint32_t)uapp->start < (1<<20));
    
    memcpy(((void*)pagedir->code_page_table)+4096, uapp->start, (int32_t)uapp->end- (int32_t)uapp->start+1); // dest, src, nbytes
}

uint32_t *init_page_directory(PageDirectory *ptr) {
    
    ptr->address = user_alloc();
    
    init_kernel_pages(ptr->address);

    init_code_pages(ptr);
    init_shm_pages(ptr);

    return init_stack_pages(ptr);
}
