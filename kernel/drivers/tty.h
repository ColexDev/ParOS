#ifndef TTY_H
#define TTY_H

#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

void terminal_initialize(void);
void putch(int c);
void puts(const char* data);
void clear_screen();
void move_cursor(int x, int y);
void update_cursor(void);
void delch(void);
void delrow(void);
void terminal_setcolor(uint8_t color);
void move_cursor_up(int num);
void move_cursor_down(int num);
void move_cursor_left(int num);
void move_cursor_right(int num);
void print_header();
void switch_tty(uint8_t tty);
void terminal_putentryat(char c, uint8_t color, size_t x, size_t y);
uint8_t get_current_tty();
uint8_t get_cursor_x();
uint8_t get_cursor_y();

#endif
