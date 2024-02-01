.POSIX:

kernel/serial.o: kernel/serial.c include/mpx/io.h include/mpx/serial.h \
  include/mpx/device.h include/sys_req.h

kernel/kmain.o: kernel/kmain.c include/mpx/gdt.h include/mpx/interrupts.h \
  include/mpx/serial.h include/mpx/device.h include/mpx/vm.h \
  include/sys_req.h include/string.h include/memory.h

kernel/core-c.o: kernel/core-c.c include/mpx/gdt.h include/mpx/panic.h \
  include/mpx/interrupts.h include/mpx/io.h include/mpx/serial.h \
  include/mpx/device.h include/sys_req.h include/string.h \
  include/mpx/vm.h

kernel/R3_Context/syscall.o: kernel/R3_Context/syscall.c include/context.h include/pcb.h

kernel/serial_io.o: kernel/serial_io.c include/serial_io.h include/mpx/io.h include/mpx/interrupts.h include/mpx/device.h

kernel/sys_call_isr.o: kernel/sys_call_isr.s
	nasm -f elf -o kernel/sys_call_isr.o kernel/sys_call_isr.s

kernel/serial_isr_asm.o: kernel/serial_isr_asm.s
	nasm -f elf -o kernel/serial_isr_asm.o kernel/serial_isr_asm.s

KERNEL_OBJECTS = \
  kernel/core-asm.o \
  kernel/sys_call_isr.o \
  kernel/serial.o \
  kernel/kmain.o \
  kernel/core-c.o \
  kernel/pcb.o \
  kernel/R3_Context/syscall.o \
  kernel/serial_io.o \
  kernel/serial_isr_asm.o
