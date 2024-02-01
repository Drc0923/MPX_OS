//
// Created by Dylan Caldwell on 9/15/23.
//
#ifndef FIJI_PCB_H
#define FIJI_PCB_H

#include "memory.h"

// Process classes
#define USER_PROCESS 0
#define SYSTEM_PROCESS 1

// Execution states
#define READY 0
#define RUNNING 1
#define BLOCKED 2

// Dispatching states
#define NOT_SUSPENDED 0
#define SUSPENDED 1

struct pcb {
    char name[16];            // Unique process name
    int class;                // Class of the process
    int priority;             // Process priority
    int exec_state;           // Execution state
    int disp_state;           // Dispatching state
    char stack[6700];         // Pointer to the process stack
    void *stack_pointer;      // Stack pointer
    struct pcb *next;         // Pointer to the next PCB for building queues
//    struct context *context;
};

// Function prototypes
struct pcb* pcb_allocate(void);
int pcb_free(struct pcb*);
struct pcb* pcb_setup(const char*, int, int);
struct pcb* pcb_find(const char*);
void pcb_insert(struct pcb*);
int pcb_remove(struct pcb*);
void resume_pcb_kernel(struct pcb*);


extern struct pcb *ReadyQueue;
extern struct pcb *BlockedQueue;



#endif //FIJI_PCB_H
