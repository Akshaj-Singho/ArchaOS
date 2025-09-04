; Multiboot header
section .multiboot
    align 4
    dd 0x1BADB002            ; magic
    dd 0x3                  ; flags (align + memory info)
    dd -(0x1BADB002 + 0x3)  ; checksum

section .text
global _start
extern kernel_main          ; <- tell NASM this exists in C

_start:
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang
