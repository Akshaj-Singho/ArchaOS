#include "vga.h"

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static int cursor_x = 0, cursor_y = 0;

// Prevent compiler from optimizing the delay away
void delay(volatile unsigned int count) {
    while (count--) {
        asm volatile("nop");
    }
}


// Write a character at position (x,y)
static void putchar_at(char c, int x, int y) {
    volatile unsigned short *vga = (unsigned short *)VGA_ADDRESS;
    vga[y * VGA_WIDTH + x] = (0x07 << 8) | c;
}

// Clear the screen
void vga_clear() {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            putchar_at(' ', x, y);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    vga_set_cursor(cursor_x, cursor_y);
}

// Set hardware cursor
static inline void outb(unsigned short port, unsigned char val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

void vga_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    unsigned short pos = y * VGA_WIDTH + x;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

// Print text at current cursor
void vga_print(const char *str) {
    while (*str) {
        putchar_at(*str++, cursor_x, cursor_y);
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    vga_set_cursor(cursor_x, cursor_y);
}

// Print centered text
// Centered print with vertical offset
void vga_print_center(const char *str, int y_offset) {
    int len = 0;
    const char *s = str;
    while (*s++) len++;

    int x = (VGA_WIDTH - len) / 2;
    int y = (VGA_HEIGHT / 2) + y_offset;

    for (int i = 0; i < len; i++) {
        putchar_at(str[i], x + i, y);
    }
}

// Show welcome message with delay, then clear
void vga_show_welcome() {
    vga_clear();
    vga_print_center("Welcome to ArchaOS", -1);   // one line above center
    vga_print_center("V0.1 \"Choatic\"", +1);    // one line below center
    delay(500000000);  // ~2s
    vga_clear();

    // Reset cursor for Arc/> prompt
    cursor_x = 0;
    cursor_y = 0;
    vga_set_cursor(cursor_x, cursor_y);
}

