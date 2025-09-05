#include "vga.h"
#include <stdint.h>   // <-- add this for uint32_t, uint8_t

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define CMD_BUFFER_SIZE 128

static char command_buffer[CMD_BUFFER_SIZE];
static int cmd_index = 0;
static unsigned short *vga_buffer = (unsigned short *)VGA_ADDRESS;
static int cursor_x = 0, cursor_y = 0;
static int prompt_x = 0, prompt_y = 0;  // protect prompt position
static int shift_pressed = 0;           // track shift state

// -------------------- Utility --------------------
static int strlen(const char *str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

static inline unsigned char inb(unsigned short port) {
    unsigned char result;
    __asm__ volatile ("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(unsigned short port, unsigned char data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

static void update_cursor() {
    unsigned short pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

// -------------------- VGA Text --------------------
void vga_set_cursor(int x, int y) {
    cursor_x = x;
    cursor_y = y;
    update_cursor();
}

void vga_clear() {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_buffer[y * VGA_WIDTH + x] = (unsigned char)' ' | (0x07 << 8);
        }
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void vga_print_char(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\b') {
        // prevent deleting past the prompt
        if (!(cursor_y == prompt_y && cursor_x <= prompt_x)) {
            if (cursor_x > 0) {
                cursor_x--;
            } else if (cursor_y > 0) {
                cursor_y--;
                cursor_x = VGA_WIDTH - 1;
            }
            vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (unsigned char)' ' | (0x07 << 8);
        }
    } else {
        vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = (unsigned char)c | (0x07 << 8);
        cursor_x++;
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
    }
    update_cursor();
}

void vga_print(const char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_print_char(str[i]);
    }
}

void vga_print_center(const char *str) {
    int len = strlen(str);
    int x = (VGA_WIDTH - len) / 2;
    int y = VGA_HEIGHT / 2;
    vga_set_cursor(x, y);
    vga_print(str);
}

static void delay(volatile unsigned int count) {
    while (count--) { __asm__ volatile("nop"); }
}

void vga_show_welcome() {
    vga_clear();
    vga_print_center("Welcome to ArchaOS v0.1.5 \"Choatic\"");
    delay(500000000);
    vga_clear();
}

// -------------------- Keyboard --------------------

void vga_prompt() {
    vga_print("Arc/> ");
    prompt_x = cursor_x;
    prompt_y = cursor_y;
    update_cursor();

    static char command_buffer[CMD_BUFFER_SIZE];
    static int cmd_len = 0;
    static int cmd_cursor = 0;
    static unsigned char last_scancode = 0;
    static int shift_pressed = 0;
    static int extended = 0;

    const char map_lower[128] = {
        0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
        '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
        'a','s','d','f','g','h','j','k','l',';','\'','`', 0, '\\',
        'z','x','c','v','b','n','m',',','.','/', 0,   '*', 0,  ' ',
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

    const char map_upper[128] = {
        0,  27, '!','@','#','$','%','^','&','*','(',')','_','+', '\b',
        '\t','Q','W','E','R','T','Y','U','I','O','P','{','}','\n', 0,
        'A','S','D','F','G','H','J','K','L',':','"','~', 0,  '|',
        'Z','X','C','V','B','N','M','<','>','?', 0,   '*', 0,  ' ',
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };

    // Helper to redraw line
    auto void redraw_line() {
        // Move cursor back to prompt
        cursor_x = prompt_x;
        cursor_y = prompt_y;

        // Clear rest of line
        for (int i = 0; i < VGA_WIDTH - prompt_x; i++)
            vga_print_char(' ');

        // Reset cursor & reprint buffer
        cursor_x = prompt_x;
        cursor_y = prompt_y;
        for (int i = 0; i < cmd_len; i++)
            vga_print_char(command_buffer[i]);

        // Set cursor at logical position
        cursor_x = prompt_x + cmd_cursor;
        update_cursor();
    }

    while (1) {
        unsigned char scancode = inb(0x60);

        if (scancode != last_scancode) {
            last_scancode = scancode;

            if (scancode == 0xE0) {
                extended = 1;
                continue;
            }

            if (!(scancode & 0x80)) { // key press
                if (extended) {
                    switch (scancode) {
                        case 0x4B: // Left
                            if (cmd_cursor > 0) cmd_cursor--;
                            redraw_line();
                            break;
                        case 0x4D: // Right
                            if (cmd_cursor < cmd_len) cmd_cursor++;
                            redraw_line();
                            break;
                        case 0x53: // Delete
                            if (cmd_cursor < cmd_len) {
                                for (int i = cmd_cursor; i < cmd_len - 1; i++)
                                    command_buffer[i] = command_buffer[i + 1];
                                cmd_len--;
                                redraw_line();
                            }
                            break;
                    }
                    extended = 0;
                } else {
                    char c = shift_pressed ? map_upper[scancode] : map_lower[scancode];
                    switch (scancode) {
                        case 0x2A: // LShift down
                        case 0x36: // RShift down
                            shift_pressed = 1;
                            break;
                        case 0x0E: // Backspace
                            if (cmd_cursor > 0) {
                                for (int i = cmd_cursor - 1; i < cmd_len - 1; i++)
                                    command_buffer[i] = command_buffer[i + 1];
                                cmd_len--;
                                cmd_cursor--;
                                redraw_line();
                            }
                            break;
                        case 0x1C: // Enter
                            command_buffer[cmd_len] = '\0';
                            vga_print("\n");
                            kernel_execute_command(command_buffer);
                            cmd_len = 0;
                            cmd_cursor = 0;
                            vga_print("Arc/> ");
                            prompt_x = cursor_x;
                            prompt_y = cursor_y;
                            update_cursor();
                            break;
                        default:
                            if (c && cmd_len < CMD_BUFFER_SIZE - 1) {
                                for (int i = cmd_len; i > cmd_cursor; i--)
                                    command_buffer[i] = command_buffer[i - 1];
                                command_buffer[cmd_cursor] = c;
                                cmd_len++;
                                cmd_cursor++;
                                redraw_line();
                            }
                            break;
                    }
                }
            } else { // key release
                if ((scancode & 0x7F) == 0x2A || (scancode & 0x7F) == 0x36)
                    shift_pressed = 0;
                last_scancode = 0;
            }
        }
    }
}

// -------------------- Beeper --------------------

// --- PC speaker sound ---
static void play_sound(uint32_t nFrequence) {
    uint32_t Div;
    uint8_t tmp;

    // Set PIT to desired frequency
    Div = 1193180 / nFrequence;
    outb(0x43, 0xb6);
    outb(0x42, (uint8_t) (Div) );
    outb(0x42, (uint8_t) (Div >> 8));

    // And play the sound using the PC speaker
    tmp = inb(0x61);
    if (tmp != (tmp | 3)) {
        outb(0x61, tmp | 3);
    }
}

static void nosound() {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

// --- Public beep() command ---
void beep() {
    // Try actual hardware PC speaker
    play_sound(1000);

    // crude delay loop (since no timer IRQ yet)
    for (volatile int i = 0; i < 0xFFFFF; i++);

    nosound();

    // Fallback visual beep
    vga_print("[BEEP!]\n");
}

// -------------------- Parser --------------------

void vga_execute_command(const char *cmd) {
    if (cmd[0] != '\0') {
        kernel_execute_command(cmd);  // hand over to kernel
    }
}

