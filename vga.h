#ifndef VGA_H
#define VGA_H

void vga_clear();
void vga_set_cursor(int x, int y);
void vga_print(const char *str);
void vga_print_center(const char *str, int y_offset);
void vga_show_welcome();
void delay(volatile unsigned int count);   // make this unsigned

#endif
