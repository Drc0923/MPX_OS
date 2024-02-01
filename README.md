# Fiji
Steps for QEMU debug

WINDOW 1

0. Go to file directory using: cd /Users/dylancaldwell/CLionProjects/Fiji

1. Run Make 
2.  Start QEMU  (Only if you don’t want to start the program): qemu-system-i386 -s -S -kernel kernel.bin

3. Start MPX (If you want to start the program)(Probably do this one): ./mpx.sh
WINDOW 2 3.  Open new Terminal Window  . Go to file directory, using: cd /Users/dylancaldwell/CLionProjects/Fiji

5. Start GDB:   i386-elf-gdb
 6. Run this: file kernel.bin

7. Target remote: target remote localhost:1234

USING DEBUGGER

Breakpoints: “break file:line”

Step (goes into function) : step or s

Next (does not go into function: next or s

Continue (to next breakpoint) : continue

UI : tui enable
