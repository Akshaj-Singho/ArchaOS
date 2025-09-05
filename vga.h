#ifndef VGA_H
#define VGA_H

void vga_clear();
void vga_set_cursor(int x, int y);
void vga_print(const char *str);
void vga_print_center(const char *str);
void vga_show_welcome();
void vga_print_char(char c);
void vga_prompt();   // New: simple Arc/> prompt loop
void kernel_execute_command(const char *cmd);
void beep();   // <--- add this

#endif
