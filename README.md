ArchaOS guide

to make the .iso by yourself :- 

Tools needed to download:-
On Debian/Kali/Ubuntu

Install everything with:

sudo apt update
sudo apt install build-essential nasm xorriso qemu-system-i386

For the boot folder:-
i)Create a folder called boot
ii)Create a folder called grub inside boot
iii)Place the grub.cfg

For the build folder:-
i)Only make one folder called build 

For the Iso folder:-
i)Create a folder called iso
ii)Create a folder called boot inside iso
iii)Create a folder called grub inside boot
iv)Place the grub.cfg into grub

For the src folder:-
i)Create a folder called src
ii)Place the boot.asm into src
iii)Place the vga.c into src
iv)Place the kernel.c into src
v)Place the vga.h into src
vi)Place the kernel.h into src
vii)Place the linker.ld into src

Now to compile place makefile in the place where u made all the folder give it appropriate permissions and run "make" in terminal

For using the pre-made .iso file :- 

Install any virtualisation software and run .iso with these specs:- 

CPU:
Any x86 (32-bit or 64-bit CPU that can run 32-bit mode).
– Works fine in QEMU, VirtualBox, VMware, or on bare metal.

RAM:
16 MB minimum (but literally even 8 MB works).
→ QEMU usually defaults to 128 MB, which is way more than enough.

Disk:
Not required (ArchaOS runs from ISO, no filesystem support yet).

Display:
VGA text mode (80x25).
Works on any standard emulator or hardware VGA.
