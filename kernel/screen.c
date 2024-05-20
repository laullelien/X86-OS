#include "stdint.h"
#include "string.h"
#include "stddef.h"

#include "screen.h"
#include "cpu.h"
#include "process.h"
#include <stdio.h>

uint16_t CURSOR_LINE = LINE_START;
uint16_t CURSOR_COL = 0;

uint8_t TEXT_CLIGNOTE = 0;
uint8_t TEXT_BG_COLOR = 0;
uint8_t TEXT_COLOR = 10;


uint16_t *ptr_mem(uint32_t lig, uint32_t col){
    return (uint16_t *)(SCREEN_ADDRESS + 2 * (lig * COLONS + col));
}


void ecrit_car(uint32_t lig, uint32_t col, char c){
    uint16_t *address = ptr_mem(lig, col);
    uint8_t text = (TEXT_CLIGNOTE << 7) | (TEXT_BG_COLOR << 4) | (TEXT_COLOR);
    
    *address = (text << 8) | c;
}

void efface_ecran(){
    for (uint8_t line=0; line < LINES; line ++){
        for (uint8_t col=0; col < COLONS; col ++){
            ecrit_car(line, col, ' ');
        }
    }
}


void place_curseur(uint32_t lig, uint32_t col){
    CURSOR_LINE = lig;
    CURSOR_COL = col;

    uint16_t index = CURSOR_COL + CURSOR_LINE * COLONS;

    outb(0x0f, CUSOR_COMMAND_PORT);
    outb(index & 0xff, CUSOR_DATA_PORT);
    outb(0x0e, CUSOR_COMMAND_PORT);
    outb((index >> 8) & 0xff, CUSOR_DATA_PORT);

}

void defilement(){
    memmove(ptr_mem(LINE_START, 0), ptr_mem(LINE_START+1, 0), (LINES-LINE_START)*COLONS * sizeof(uint16_t));
    for (uint8_t col=0; col < COLONS; col ++){
        ecrit_car(LINES-1, col, ' ');
    }
}

void move_cursor_vertical(uint8_t down){
    if (down){
        CURSOR_LINE ++;
        if (CURSOR_LINE >= LINES){
            CURSOR_LINE --;
            defilement();
        }
    } else {
        if (CURSOR_LINE >= LINE_START+1){
            CURSOR_LINE --;
        }
    }
}

void move_cursor_horizontal(uint8_t right){
    if (right){
        CURSOR_COL ++;
        if (CURSOR_COL >= COLONS){
            CURSOR_COL = 0;
            move_cursor_vertical(1);

        }
    } else {
        if (CURSOR_COL > 0){
            CURSOR_COL --;
        }
    }

}

void traite_car(char c){
    switch (c){
    case 8: // BS
        move_cursor_horizontal(0);
        place_curseur(CURSOR_LINE, CURSOR_COL);

        break;
    case 9: // HT
        CURSOR_COL = 8*(CURSOR_COL/8 + 1);
        if (CURSOR_COL >= COLONS){
            CURSOR_COL = COLONS-1;
        }
        place_curseur(CURSOR_LINE, CURSOR_COL);

        break;
    case 10: // LF
        move_cursor_vertical(1);
        place_curseur(CURSOR_LINE, 0);

        break;
    case 12: // FF
        efface_ecran();
        place_curseur(LINE_START, 0);

        break;
    case 13: // CR
        place_curseur(CURSOR_LINE, 0);
        
        break;
    default:
        ecrit_car(CURSOR_LINE, CURSOR_COL, c);
        move_cursor_horizontal(1);
        place_curseur(CURSOR_LINE, CURSOR_COL);
        
        break;
    }
}

void console_putbytes(const char *s, int len){
    for (int i=0; i < len; i++){
        traite_car(s[i]);
    }
}

void cons_write(const char *str, long size) {
    if (check_user_pointer(str)) {return;}
    console_putbytes(str, (int)size);
}
