#ifndef __ECRAN_H_INCLUDED__
#define __ECRAN_H_INCLUDED__

#include "stdint.h"

#define COLONS 80
#define LINES 25

#define LINE_START 1

#define CUSOR_COMMAND_PORT 0x3D4
#define CUSOR_DATA_PORT 0x3D5

#define SCREEN_ADDRESS 0xB8000

extern uint8_t TEXT_COLOR;

uint16_t *ptr_mem(uint32_t lig, uint32_t col);

void ecrit_car(uint32_t lig, uint32_t col, char c);

void efface_ecran(void);

void place_curseur(uint32_t lig, uint32_t col);

void traite_car(char c);

void defilement(void);

void console_putbytes(const char *s, int len);

void cons_write(const char *str, long size);

#endif /* __ECRAN_H_INCLUDED__ */