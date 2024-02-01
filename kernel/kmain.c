#include <mpx/gdt.h>
#include <mpx/interrupts.h>
#include <mpx/serial.h>
#include <mpx/vm.h>
#include <sys_req.h>
#include <string.h>
#include <memory.h>
#include <pcb.h>
#include <pcbuser.h>
#include <context.h>
#include <processes.h>
#include <stdint.h>
#include <cmdHandler.h>
#include <processes.h>
#include "serial_io.h"


void init_comhand_process(void);       // Function prototype for initializing command handler process
void init_system_idle_process(void);   // Function prototype for initializing system idle process

// Logs a message to the serial port with a prefix
static void klogv(device dev, const char *msg) {
    char prefix[] = "klogv: ";
    serial_out(dev, prefix, strlen(prefix)); // Outputs prefix
    serial_out(dev, msg, strlen(msg));       // Outputs message
    serial_out(dev, "\r\n", 2);              // Outputs newline
}

// Outputs startup logo sequence
void startup_sequence(void) {
    char logo[] = "\n\n  \033[0;34m====\033[0;37m\n /    \\\n/      \\\n|      |\n| \033[0;31mFIJI\033[0;37m |\n|      |\n|      |\n|______|\n\n";
    sys_req(WRITE, COM1, logo, strlen(logo)); // System request to write the logo to COM1
}

// Initializes the command handler process
void init_comhand_process(void) {
    struct pcb* comHand = pcb_setup("comhand", 1, 0); // Set up PCB for command handler
    if (!comHand) {
        klogv(COM1, "Failed to setup comhand process..."); // Log failure message
        return;
    }
    // Configure context for command handler process
    struct context* ctx = comHand->stack_pointer;
    // Setting various segment registers and stack pointers
    ctx->cs = 0x08; ctx->ds = 0x10; ctx->es = 0x10; ctx->fs = 0x10; ctx->gs = 0x10; ctx->ss = 0x10;
    ctx->ebp = (int)comHand->stack;
    ctx->esp = (int)comHand->stack_pointer;
    ctx->eip = (int)comhand; // Set instruction pointer to command handler function
    ctx->eflags = 0x0202; // Set flags
    // Initializing general-purpose registers
    ctx->eax = 0x00; ctx->ebx = 0x00; ctx->ecx = 0x00; ctx->edx = 0x00; ctx->esi = 0x00; ctx->edi = 0x00;
    pcb_insert(comHand); // Insert PCB into the process queue
    klogv(COM1, "Successfully initialized comhand..."); // Log success message
}

void serial_interrupt_wrapper(void *arg) {
    (void)arg; // Cast arg to void to indicate it's intentionally unused
    serial_interrupt();
}


// Initializes the system idle process
void init_system_idle_process(void) {
    struct pcb* systemIdle = pcb_setup("Sys IDLE Proc", 1, 9); // Set up PCB for system idle process
    if (!systemIdle) {
        klogv(COM1, "Failed to setup System IDLE Process..."); // Log failure message
        return;
    }
    // Configure context for system idle process
    struct context* ctx = (struct context *)systemIdle->stack_pointer;
    // Setting various segment registers and stack pointers
    ctx->cs = 0x08; ctx->ds = 0x10; ctx->es = 0x10; ctx->fs = 0x10; ctx->gs = 0x10; ctx->ss = 0x10;
    ctx->ebp = (int)systemIdle->stack;
    ctx->esp = (int)systemIdle->stack_pointer;
    ctx->eip = (int)sys_idle_process; // Set instruction pointer to system idle process function
    ctx->eflags = 0x0202; // Set flags
    // Initializing general-purpose registers
    ctx->eax = 0x00; ctx->ebx = 0x00; ctx->ecx = 0x00; ctx->edx = 0x00; ctx->esi = 0x00; ctx->edi = 0x00;
    pcb_insert(systemIdle); // Insert PCB into the process queue
    klogv(COM1, "Successfully initialized system idle process..."); // Log success message
}



void kmain(void)
{
    // 0) Serial I/O -- <mpx/serial.h>
    // If we don't initialize the serial port, we have no way of
    // performing I/O. So we need to do that before anything else so we
    // can at least get some output on the screen.
    // Note that here, you should call the function *before* the output
    // via klogv(), or the message won't print. In all other cases, the
    // output should come first as it describes what is about to happen.


    serial_init(COM1);
    //serial_out(COM1, buffer, len);
    klogv(COM1, "Initialized serial I/O on COM1 device...");

    // 1) Global Descriptor Table (GDT) -- <mpx/gdt.h>
    // Keeps track of the various memory segments (Code, Data, Stack, etc.)
    // required by the x86 architecture. This needs to be initialized before
    // interrupts can be configured.
    klogv(COM1, "Initializing Global Descriptor Table...");
    gdt_init();

    // 2) Interrupt Descriptor Table (IDT) -- <mpx/interrupts.h>
    // Keeps track of where the various Interrupt Vectors are stored. It
    // needs to be initialized before Interrupt Service Routines (ISRs) can
    // be installed.
    klogv(COM1, "Initializing Interrupt Descriptor Table...");
    idt_init();


    // 3) Disable Interrupts -- <mpx/interrupts.h>
    // You'll be modifying how interrupts work, so disable them to avoid
    // crashing.
    klogv(COM1, "Disabling interrupts...");
    cli();

    // 4) Interrupt Request (IRQ) -- <mpx/interrupts.h>
    // The x86 architecture requires ISRs for at least the first 32
    // Interrupt Request (IRQ) lines.
    klogv(COM1, "Initializing Interrupt Request routines...");
    irq_init();
    idt_install(0x24, serial_interrupt_wrapper);
    idt_install(0x23, serial_interrupt_wrapper);


    // 5) Programmable Interrupt Controller (PIC) -- <mpx/interrupts.h>
    // The x86 architecture uses a Programmable Interrupt Controller (PIC)
    // to map hardware interrupts to software interrupts that the CPU can
    // then handle via the IDT and its list of ISRs.
    klogv(COM1, "Initializing Programmable Interrupt Controller...");
    pic_init();

    // 6) Reenable interrupts -- <mpx/interrupts.h>
    // Now that interrupt routines are set up, allow interrupts to happen
    // again.
    klogv(COM1, "Enabling Interrupts...");
    sti();

    // 7) Virtual Memory (VM) -- <mpx/vm.h>
    // Virtual Memory (VM) allows the CPU to map logical addresses used by
    // programs to physical address in RAM. This allows each process to
    // behave as though it has exclusive access to memory. It also allows,
    // in more advanced systems, the kernel to swap programs between RAM and
    // storage (such as a hard drive or SSD), and to set permissions such as
    // Read, Write, or Execute for pages of memory. VM is managed through
    // Page Tables, data structures that describe the logical-to-physical
    // mapping as well as manage permissions and other metadata.
    klogv(COM1, "Initializing Virtual Memory...");
    vm_init();

    // 8) MPX Modules -- *headers vary*
    // Module specific initialization -- not all modules require this.
    klogv(COM1, "Initializing MPX modules...");
    // R5: sys_set_heap_functions(...);
    // R4: create commhand and idle processes

    // 9) YOUR command handler -- *create and #include an appropriate .h file*
    // Pass execution to your command handler so the user can interact with
    // the system.
    klogv(COM1, "Transferring control to commhand...");
    // i uncommented this for r4

    //__asm__ volatile ("int $0x60" :: "a"(IDLE));
    //__asm__ volatile ("int $0x60" :: "a"(IDLE));
    // Setup Command Handler process

    //comhand();
    init_system_idle_process();
    init_comhand_process();
    
    startup_sequence();
    
    __asm__ volatile ("int $0x60" :: "a"(IDLE));

    // 10) System Shutdown -- *headers to be determined by your design*
    // After your command handler returns, take care of any clean up that
    // is necessary.
    klogv(COM1, "Starting system shutdown procedure...");


    // 11) Halt CPU -- *no headers necessary, no changes necessary*
    // Execution of kmain() will complete and return to where it was called
    // in boot.s, which will then attempt to power off Qemu or halt the CPU.
    klogv(COM1, "Halting CPU...");
}
