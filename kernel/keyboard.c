
#include "debug.h"
#include "cpu.h"
#include "kbd.h"
#include "screen.h"
#include "queue.h"
#include "process.h"

static int ECHO = 1;
#define BUFFER_MAX_SIZE 20
static char BUFFER[BUFFER_MAX_SIZE];
static unsigned int BUFFER_START = 0;
static unsigned int BUFFER_SIZE = 0;

static link IO_QUEUE = LIST_HEAD_INIT(IO_QUEUE);

static void wake_io() {
    while (!queue_empty(&IO_QUEUE)) {
        Process *process = queue_out(&IO_QUEUE, Process, listfield);
        make_process_activable(process);
    }
    ordonnance();
}

static void echo_char(char c) {
    if (c == 8) {
        if (BUFFER_SIZE != 0) {
            BUFFER_SIZE -= 1;
            if (ECHO == 1) {
                char space = ' ';
                console_putbytes(&c, 1);
                console_putbytes(&space, 1);
                console_putbytes(&c, 1);
            }
        }
    } else {
        if (BUFFER_SIZE < BUFFER_MAX_SIZE) {
            if (ECHO == 1) {
                console_putbytes(&c, 1);
            }
            BUFFER[(BUFFER_START + BUFFER_SIZE)%BUFFER_MAX_SIZE] = c;
            BUFFER_SIZE += 1;
            if (c == 10) {
                wake_io();
            }
        }
    }
    
}

void keyboard_data(char *str) {
    int i=0;
    while (str[i] != 0) {
        char c = str[i];
        i ++;
        if (32 <= c && c <= 126) {
            echo_char(c);
            continue;
        }

        switch (c)
        {
        case 127:
            echo_char(8);
            break;
        case 9:
            echo_char(c);
            break;
        
        case 13:
            echo_char(10);
            break;
        
        default:
            if (c < 32) {
                echo_char('^');
                echo_char(c+64);
            }
            break;
        }
    }
}

void kbd_leds(unsigned char leds) {
    outb(0xed, 0x60);
    outb(leds, 0x60);
}

void cons_echo(int on) {
    if (on == 0) {
        ECHO = 0;
    } else {
        ECHO = 1;
    }
}

int cons_read(char *string, unsigned long length) {
    if (check_user_pointer(string)) {
        return 0;
    }
    if (length == 0) {
        return 0;
    }
    while (1) {
        for(unsigned int i=0;i<BUFFER_SIZE;i++) {
            if (BUFFER[(BUFFER_START + i)%BUFFER_MAX_SIZE] == 10) {
                // on a une ligne à renvoyer
                unsigned int j;
                for (j=0;(j<i && j<length);j++) {
                    string[j] = BUFFER[(BUFFER_START + j)%BUFFER_MAX_SIZE];
                }
                
                BUFFER_START += j;
                BUFFER_SIZE -= j;
                if (j != length) { // on est arreté par le \n et pas pas la taille
                    BUFFER_START += 1;
                    BUFFER_SIZE -=1;
                }
                
                return j;
            }
        }

        Process *process = getprocess(getpid());
        process->queue_head = &IO_QUEUE;
        queue_add(process, &IO_QUEUE, Process, listfield, priority);
        process->state = WAIT_IO;

        ordonnance();
    }
    return 0;
}

void handle_scancode() {
    
    int scancode = inb(0x60);
    do_scancode(scancode);
    outb(0x20, 0x20);
    
}