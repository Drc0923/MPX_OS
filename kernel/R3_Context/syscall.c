#include "context.h"
#include "pcb.h"
#include <stddef.h>

#define IDLE 1      // Define IDLE as 1, used in sys_call to represent idle system call
#define EXIT 0      // Define EXIT as 0, used in sys_call for exit system call

// Global variables
struct pcb *current_pcb = NULL;             // Pointer to the current running PCB
static struct context *initial_context = NULL; // Initial context stored during the first IDLE call

// Function prototypes
static void save_context(struct context *ctx); // Saves the context of the current PCB
static struct pcb* select_next_process(void);  // Selects the next process to run from the ready queue
static int sys_req_idle(void);                // Checks if the system is idle
static void terminate_and_free_all_pcbs(struct pcb **queue); // Terminates and frees all PCBs in a queue

// System call implementation
struct context *sys_call(struct context *ctx) {
    switch (ctx->eax) { // Switch based on the system call number stored in eax
        case IDLE: // IDLE system call
            if (initial_context == NULL) {
                initial_context = ctx; // Store the initial context if not already stored
            }
            if (current_pcb != NULL) { // If there is a current PCB
                save_context(ctx); // Save its context
                current_pcb->exec_state = READY; // Set its state to READY
                pcb_insert(current_pcb); // Insert it back into the ready queue
            }
            break;

        case EXIT: // EXIT system call
            // Terminate and free all PCBs in both ready and blocked queues
            terminate_and_free_all_pcbs(&ReadyQueue);
            terminate_and_free_all_pcbs(&BlockedQueue);

            if (current_pcb) { // If there is a current PCB
                pcb_free(current_pcb); // Free it
                current_pcb = NULL; // Set the current PCB pointer to NULL
            }
            break;

        default: // Default case for unknown system call
            ctx->eax = -1; // Set eax to -1 indicating error
            return ctx; // Return the context
    }

    // If there are ready, non-suspended PCBs
    if (sys_req_idle() == 0) {
        struct pcb *next_pcb = select_next_process(); // Select the next PCB
        current_pcb = next_pcb; // Update the current PCB
        current_pcb->exec_state = READY; // Set its state to READY
        return (struct context *) current_pcb->stack_pointer; // Return its context
    } else { // If the system is idle
        ctx = initial_context; // Use the initial context
        initial_context = NULL; // Reset the initial context
        return ctx; // Return the initial context
    }
}

// Saves the context by updating the stack pointer of the current PCB
static void save_context(struct context *ctx) {
    if (current_pcb) {
        current_pcb->stack_pointer = ctx; // Update the stack pointer in the current PCB
    }
}

// Checks if there are any ready, non-suspended PCBs in the ready queue
static int sys_req_idle(void) {
    struct pcb *current = ReadyQueue; // Start from the head of the ready queue
    while (current != NULL) {
        if (current->exec_state == READY && current->disp_state == NOT_SUSPENDED) {
            return 0; // Return 0 if a ready, non-suspended PCB is found
        }
        current = current->next; // Move to the next PCB in the queue
    }
    return -1; // Return -1 if no ready, non-suspended PCBs are found
}

// Selects the next process from the ready queue if there are any ready, non-suspended PCBs
static struct pcb* select_next_process(void) {
    if (ReadyQueue && ReadyQueue->exec_state == READY && ReadyQueue->disp_state == NOT_SUSPENDED) {
        struct pcb *selected_pcb = ReadyQueue; // Select the first PCB in the ready queue
        pcb_remove(ReadyQueue); // Remove it from the queue
        return selected_pcb; // Return the selected PCB
    }
    return NULL; // Return NULL if no suitable PCB is found
}

// Terminates and frees all PCBs in a given queue
static void terminate_and_free_all_pcbs(struct pcb **queue) {
    struct pcb *current;
    while ((current = *queue) != NULL) {
        *queue = current->next; // Move to the next PCB in the queue
        pcb_free(current); // Free the current PCB
    }
}
