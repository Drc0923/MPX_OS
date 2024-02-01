//
// Created by Dylan Caldwell on 10/13/23.
//

#ifndef FIJI_CONTEXT_H
#define FIJI_CONTEXT_H


struct context {
    int ds;     // Data segment selector
    int es;     // Extra segment selector
    int fs;     // Additional segment selector used for string operations and others
    int gs;     // Additional segment selector used for string operations and others
    int ss;     // Stack segment selector
    int eax;    // Accumulator register, used for arithmetic operations
    int ebx;    // Base register, used for base pointer to data in the data segment
    int ecx;    // Count register, used for loop counters and string manipulation
    int edx;    // Data register, used for I/O operations, arithmetic operations and more
    int esi;    // Source Index register, used for string operations
    int edi;    // Destination Index register, used for string operations
    int ebp;    // Base Pointer register, used for stack frame base pointer
    int esp;    // Stack Pointer register, points to the top of the stack
    int eip;    // Instruction Pointer register, points to the next instruction to execute
    int cs;     // Code segment selector, used for addressing a location in the code segment
    int eflags; // Flags register, contains processor flags and control bits
};



#endif //FIJI_CONTEXT_H
