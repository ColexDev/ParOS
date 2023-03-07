#include <stdint.h>

#include "../interrupts/isr.h"
#include "../interrupts/irq.h"
#include "../io/port_io.h"
#include "../stdlib/util.h"
#include "tty.h"

/* Change this to a bitfield or a single
 * uint8_t and have flags */
static uint8_t shift_pressed     = 0;
static uint8_t ctrl_pressed      = 0;
static uint8_t caps_lock_pressed = 0;
static uint8_t esc_pressed       = 0;
static uint8_t alt_pressed       = 0;

static uint8_t last_key_press    = 0;
static uint8_t key_read          = 0;

/* Taken from https://github.com/krisvers/kros/blob/master/kernel/arch/x86/drivers/keyboard.c */
static char keycodes[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9',  '0', '-',  '=',  '\b', '\t',                                                    /* <-- Tab */
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[',  ']', '\n', 0,                                                                   /* <-- control key */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,    '\\', 'z',  'x',  'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, /* Alt */
    ' ',                                                                                                                                    /* Space bar */
    0,                                                                                                                                      /* Caps lock */
    0,                                                                                                                                      /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,   0,                                                                                              /* < ... F10 */
    0,                                                                                                                                      /* 69 - Num lock*/
    0,                                                                                                                                      /* Scroll Lock */
    0,                                                                                                                                      /* Home key */
    0,                                                                                                                                      /* Up Arrow */
    0,                                                                                                                                      /* Page Up */
    '-', 0,                                                                                                                                 /* Left Arrow */
    0,   0,                                                                                                                                 /* Right Arrow */
    '+', 0,                                                                                                                                 /* 79 - End key*/
    0,                                                                                                                                      /* Down Arrow */
    0,                                                                                                                                      /* Page Down */
    0,                                                                                                                                      /* Insert Key */
    0,                                                                                                                                      /* Delete Key */
    0,   0,   0,   0,                                                                                                                       /* F11 Key */
    0,                                                                                                                                      /* F12 Key */
    0,                                                                                                                                      /* All other keys are undefined */
};

static char keycodes_shift[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_',  '+', '\b', '\t',                                                    /* <-- Tab */
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,                                                                  /* <-- control key */
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 0,   0,    '|', 'Z',  'X',  'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, /* Alt */
    ' ',                                                                                                                                  /* Space bar */
    0,                                                                                                                                    /* Caps lock */
    0,                                                                                                                                    /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,   0,                                                                                            /* < ... F10 */
    0,                                                                                                                                    /* 69 - Num lock*/
    0,                                                                                                                                    /* Scroll Lock */
    0,                                                                                                                                    /* Home key */
    0,                                                                                                                                    /* Up Arrow */
    0,                                                                                                                                    /* Page Up */
    '-', 0,                                                                                                                               /* Left Arrow */
    0,   0,                                                                                                                               /* Right Arrow */
    '+', 0,                                                                                                                               /* 79 - End key*/
    0,                                                                                                                                    /* Down Arrow */
    0,                                                                                                                                    /* Page Down */
    0,                                                                                                                                    /* Insert Key */
    0,                                                                                                                                    /* Delete Key */
    0,   0,   0,   0,                                                                                                                     /* F11 Key */
    0,                                                                                                                                    /* F12 Key */
    0,                                                                                                                                    /* All other keys are undefined */
};
/* TODO:
 * Vim Mode
 * Buffer so backspace works correctly
 * Handle ALL keys correctly including shifts */

char
getchar()
{
    if (!key_read) {
        key_read = 1;
        return last_key_press;
    } else {
        return 0;
    }
}

void
handle_keypress(uint8_t scancode)
{
    if (shift_pressed || caps_lock_pressed) {
        // putch(keycodes_shift[scancode]); 
        key_read = 0;
        last_key_press = keycodes_shift[scancode];
    } else if (ctrl_pressed) {
        /* Support ctrl keys for keybindings */
    } else if (alt_pressed) {
        switch(keycodes[scancode]) {
            case '1':
                switch_tty(1);
                break;
            case '2':
                switch_tty(2);
                break;
            case '3':
                switch_tty(3);
                break;
        }
    } else if (esc_pressed) { /* Vim mode baby */
        switch (keycodes[scancode]) {
            case 'j':
                move_cursor_down(1);
                break;
            case 'k':
                move_cursor_up(1);
                break;
            case 'h':
                move_cursor_left(1);
                break;
            case 'l':
                move_cursor_right(1);
                break;
            case 'x':
                /* It moves cursor because delch() deletes character behind the 
                 * cursor not the character under it */
                move_cursor_right(1);
                delch();
                break;
            case 'i':
                esc_pressed = 0;
                break;
            /* FIX: What should default be? */
    }
    } else {
        // putch(keycodes[scancode]);
        key_read = 0;
        last_key_press = keycodes[scancode];
    }
}
/* Scan code list:
 * https://www.scs.stanford.edu/10wi-cs140/pintos/specs/kbd/scancodes-1.html */
void
keyboard_handler(struct registers* regs)
{
    uint8_t scancode = inb(0x60); /* 0x60 is the keyboards data buffer */

    /* See if a key was released */
    if (scancode & 0x80) {
        switch(scancode) {
            case 0x1d | 0x80: /* Right Control */
                ctrl_pressed = 0;
                break;
            case 0x2a | 0x80: /* Left Shift */
            case 0x36 | 0x80: /* Right Shift */
                shift_pressed = 0;
                break;
            case 0x38 | 0x80: /* Left alt */
                alt_pressed = 0;
                break;
        }
    /* See if a key was pressed */
    } else {
        switch (scancode) {
            case 0x01: /* Escape */
                esc_pressed = !esc_pressed;
                break;
            case 0x1d: /* Right Control */
                ctrl_pressed = 1;
                break;
            case 0x2a: /* Left Shift */
            case 0x36: /* Right Shift */
                shift_pressed = 1;
                break;
            case 0x3a: /* Caps Lock */
                caps_lock_pressed = !caps_lock_pressed;
                break;
            case 0x38: /* Left alt */
                alt_pressed = 1;
                break;
            default:
                handle_keypress(scancode);
                break;
        }
    }
}

void
keyboard_install()
{
    irq_register_handler(1, keyboard_handler);
}
