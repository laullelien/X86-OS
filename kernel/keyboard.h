#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include "debug.h"
#include "cpu.h"

void handle_scancode();
void cons_echo(int on);
int cons_read(char *string, unsigned long length);

#endif