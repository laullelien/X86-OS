#include "time.h"
#include "stdbool.h"
#include "stdint.h"
#include "segment.h"
#include "stdio.h"
#include "cpu.h"
#include "screen.h"
#include "process.h"


#define CLOCK_FREQ 50

uint32_t QUARTZ_FREQ=0x1234DD;
unsigned long QUARTZ_TICKS = 0x1234DD/CLOCK_FREQ;

uint32_t Sec=0;
static unsigned long nb_tic = 0;

void clock_settings(unsigned long *quartz, unsigned long *ticks) {
    *quartz = QUARTZ_FREQ;
    *ticks = QUARTZ_TICKS;
}

unsigned long current_clock() {
    return nb_tic;
}

static void print_time(char *str_time,uint32_t len){
    uint32_t col=79-len;
    uint32_t lig=0;
    for (uint32_t i=0; i<len; i++){
        ecrit_car(lig, col, *str_time);
        str_time++;
        col++;
    }
}

void tic_PIT(void){
    outb(0x20,0x20);

    char buffer[50];
    nb_tic++;
    
    if (nb_tic % CLOCK_FREQ == 0) {
        int S=0;
        int M=0;
        int H=0;
        Sec++;
        H = (Sec/3600); 
	    M = (Sec -(3600*H))/60;
	    S = (Sec -(3600*H)-(M*60));
        
        sprintf(buffer,"%02i:%02i:%02i",H,M,S);
        print_time(buffer,8);
    }
}


static void init_traitant_IT(uint32_t num_IT, void (*traitant)(void)){
    uint32_t M1;
    uint32_t M2;
    uint32_t adr=0x1000+(8*num_IT);
    uint32_t adr_trait= (uint32_t )traitant;
    M1=(KERNEL_CS<<16);
    M1=M1|(adr_trait&0x0000FFFF);
    M2=(adr_trait&0xFFFF0000)|(0x00008E00);
    uint32_t *ptr=(uint32_t *) adr;
    *ptr=M1;
    ptr+=1;
    *ptr=M2;
}

static void set_freq(){
    

    outb(0x34,0x43);
    outb(QUARTZ_TICKS & 0xFF, 0x40);
    outb((QUARTZ_TICKS>>8), 0x40);
}

static void mask_IRQ(uint32_t num_IRQ, bool maskPar){
    uint8_t mask =  inb(0x21);
    if (maskPar) {
        mask |= (1 << num_IRQ);
    } else {
        mask &= ~(1 << num_IRQ);
    }
    outb(mask, 0x21);
}

void init_clock(void){
	init_traitant_IT(32, traitant_IT_32);
    set_freq();
    mask_IRQ(0,0);
}