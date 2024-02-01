#include "load_r3.h"
#include "processes.h"
#include "pcb.h"
#include "context.h"
#include <string.h>
#include <stdint.h>
#include <sys_req.h>

#define COM1 0x3F8
// stores names of our choosing
const char* process_names[] = {"Process 1", "Process 2", "Process 3", "Process 4", "Process 5"};
// stores priorites of our choosing
const int priorities[] = {1, 2, 3, 4, 5};

void (*process_funcs[])(void) = {proc1, proc2, proc3, proc4, proc5};

void load_r3(void) {
    for (int i = 0; i < 5; i++) {
        struct pcb *new_pcb = pcb_allocate();
        if (new_pcb == NULL) {
            char error_msg[] = "Error: Failed to allocate PCB for R3 process.\n";
            sys_req(WRITE, COM1, error_msg, strlen(error_msg));
            return;
        }

        // copies process_names[i] (the source) into new_pcb->name (destination)
        strncpy(new_pcb->name, process_names[i], sizeof(new_pcb->name));
        new_pcb->priority = 5;
        new_pcb->exec_state = READY;
        new_pcb->disp_state = NOT_SUSPENDED;
        //new_pcb->stack_pointer = (struct context *)new_pcb->stack_pointer;
        struct context* new_context = (struct context*)new_pcb->stack_pointer;
        // Initialize the context / segment registers
        new_context->cs = 0x08;
        new_context->ds = 0x10;
        new_context->es = 0x10;
        new_context->fs = 0x10;
        new_context->gs = 0x10;
        new_context->ss = 0x10;
        // Adjusted these lines to use the correct fields
        //set esp gen register to stack_pointer
        // change to memory address of the stack
        // now doing gen purpose and status control
        new_context->ebp = (int)(new_pcb->stack);
        new_context->esp = (int)new_pcb->stack_pointer;
        // change to memory address of the stack
        //new_context->ebp = (uint32_t)(new_pcb->stack);
        new_context->eip = (int)process_funcs[i];
        new_context->eflags = 0x0202;
        new_context->eax = 0;
        new_context->ebx = 0;
        new_context->ecx = 0;
        new_context->edx = 0;
        new_context->esi = 0;
        new_context->edi = 0;

        // Add the new PCB to the queue
        pcb_insert(new_pcb);
    }
    char success_msg[] = "\nAll R3 processes have been successfully loaded.\n";
    sys_req(WRITE, COM1, success_msg, strlen(success_msg));
}
