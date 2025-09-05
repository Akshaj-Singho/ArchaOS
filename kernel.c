#include "kernel.h"
#include "vga.h"  // Include VGA functionality
#include <stdint.h>   // for uint32_t, uint8_t
#include <stddef.h>

// Simple string comparison
static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Compare first n chars
static int strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}


static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(unsigned short port, unsigned char data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

// reboot via keyboard controller
void reboot() {
    unsigned char good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);  // pulse CPU reset line
}

// crude uptime counter (increment in timer IRQ)
volatile unsigned int uptime_seconds = 0;

// integer to string
char* itoa(int value, char* str, int base) {
    char *rc = str, *ptr, *low;
    if (base < 2 || base > 36) { *str = '\0'; return str; }
    ptr = str;
    if (value < 0 && base == 10) { *ptr++ = '-'; value = -value; }
    low = ptr;
    do {
        *ptr++ = "0123456789abcdefghijklmnopqrstuvwxyz"[value % base];
        value /= base;
    } while (value);
    *ptr-- = '\0';
    while (low < ptr) { char tmp = *low; *low++ = *ptr; *ptr-- = tmp; }
    return rc;
}

// Very simple string compare (since libc is not available)
static int str_eq(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 0;
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

void kernel_execute_command(const char *cmd) {
    if (strcmp(cmd, "help") == 0) {
        vga_print("Available commands:\n");
        vga_print("  clear          - Clear the screen\n");
        vga_print("  reboot         - Reboot the machine\n");
        vga_print("  halt           - Shutdown / Halt CPU\n");
        vga_print("  uptime         - Show uptime (seconds)\n");
        vga_print("  echo <text>    - Print text back\n");
        vga_print("  settings about - Info about ArchaOS\n");
        vga_print("  settings time  - Show time (UTC/IST)\n");
        vga_print("  ls             - List files (stub)\n");
        vga_print("  ports          - Show port info (stub)\n");
        vga_print("  beep           - PC speaker beep\n");
        vga_print("  credits        - Show OS credits\n");
    }

    else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
    }

    else if (strcmp(cmd, "reboot") == 0) {
        reboot();
    }

    else if (strcmp(cmd, "halt") == 0) {
        vga_print("System halted.\n");
        for (;;); // hang
    }

    else if (strcmp(cmd, "uptime") == 0) {
        char buf[32];
        itoa(uptime_seconds, buf, 10);
        vga_print("Uptime: ");
        vga_print(buf);
        vga_print(" seconds\n");
    }

    else if (strncmp(cmd, "echo ", 5) == 0) {
        vga_print(cmd + 5);
        vga_print("\n");
    }

    else if (strcmp(cmd, "settings about") == 0) {
        vga_print("ArchaOS v0.2 \"Chaotic\"\n");
        vga_print("Author: AkshajGPT\n");
        vga_print("Kernel base: 32-bit x86, VGA TTY\n");
    }

    else if (strncmp(cmd, "settings time", 13) == 0) {
        // stub until CMOS RTC is wired
        vga_print("Time feature not yet implemented.\n");
    }

    else if (strcmp(cmd, "ls") == 0) {
        vga_print("/ArchaOS\n");
    }

    else if (strcmp(cmd, "ports") == 0) {
        vga_print("I/O Ports: feature stub\n");
    }

    else if (strcmp(cmd, "beep") == 0) {
        beep();
    }

    else if (strcmp(cmd, "credits") == 0) {
        vga_print("Made by AkshajGPT (Pen name)\n");
    }

    else {
        vga_print("Unknown command. Type 'help'.\n");
    }
}

void kernel_main(void) {
    vga_show_welcome();
    vga_prompt();
    vga_set_cursor(0, 0);
    vga_print("Arc/>");
    
}
