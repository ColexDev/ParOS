#include <stdint.h>

#include "vga.h"
#include "tty.h"
#include "../stdlib/util.h"
#include "../io/port_io.h"

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
volatile uint16_t* terminal_buffer;

static const uint16_t* VGA_MEMORY_START = (uint16_t*) 0xB8000;

void
terminal_initialize(void) 
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    terminal_buffer = (volatile uint16_t*) VGA_MEMORY_START;
    clear_screen();
}

void
terminal_setcolor(uint8_t color) 
{
    terminal_color = color;
}

void
terminal_putentryat(char c, uint8_t color, size_t x, size_t y) 
{
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

/* Scrolls the terminal by one line */
void
terminal_scroll(){
    for(size_t y = 0; y < VGA_HEIGHT; y++){
        for (size_t x = 0; x < VGA_WIDTH; x++){
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
}

void
clear_screen()
{
    terminal_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_putentryat(' ', terminal_color, x, y);
        }
    }
}

/* Credit to: https://stackoverflow.com/questions/35601175/adding-support-for-newlines-n-in-c */
void
putch(int c) {
    switch (c) {
        case '\n':
            terminal_column = 0;
            terminal_row++;
            break;
        case '\r':
            terminal_column = 0;
            break;
        case '\t':
            /* Add option to change tab width */
            terminal_column = (terminal_column + 4) / 4 * 4;
            if (terminal_column >= VGA_WIDTH) {
                terminal_column -= VGA_WIDTH;
                terminal_row++;
            }
            break;
        case '\f':
            /* Erase Screen */
            clear_screen();
            terminal_column = terminal_row = 0;
            break;
        case '\a':
            /* Beep */
            break;
        case 0x08: /* Backspace */
            delch();
            break;
        default:
            terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
            if (++terminal_column == VGA_WIDTH) {
                terminal_column = 0;
                ++terminal_row;
            }
            break;
    }
    if (terminal_row == VGA_HEIGHT) {
        terminal_scroll();
        --terminal_row;
    }
    update_cursor();
}

/* TODO: Maybe return the character deleted? */
void
delch(void)
{
    if (terminal_column != 0) {
        move_cursor(terminal_column - 1, terminal_row);
    } else {
        move_cursor(VGA_WIDTH - 1, terminal_row - 1);
    }
    terminal_putentryat(' ', terminal_color, terminal_column, terminal_row);
}

void
delrow(void)
{
    size_t old_column = terminal_column;

    if (old_column == 0 && terminal_row != 0) {
        /* TODO: Find where end of text is on each line and make the x position that */
        move_cursor(terminal_column, terminal_row - 1);
    }

    for (size_t i = 0; i < old_column; i++) {
        delch();
    }
}

void
puts(const char* data) 
{
    for (size_t i = 0; i < strlen(data); i++)
        putch(data[i]);
}

void
update_cursor(void)
{
    size_t index = terminal_row * VGA_WIDTH + terminal_column;

    outb(0x3D4, 14);
    outb(0x3D5, index >> 8);
    outb(0x3D4, 15);
    outb(0x3D5, index);
}

void
move_cursor(int x, int y)
{
    terminal_row = y;
    terminal_column = x;
    update_cursor();
}
