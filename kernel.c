#include "kernel.h"
#include "vga.h"  // Include VGA functionality

void kernel_main(void) {
    vga_show_welcome();
    vga_set_cursor(0, 0);
    vga_print("Arc/>");
}
