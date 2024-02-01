#include "pcb.h"
#include "memory.h"
#include <context.h>
#include <string.h>
#include <sys_req.h>

#define COM1 0x3F8


struct pcb *ReadyQueue = NULL;   // Pointer to the head of the Ready Queue
struct pcb *BlockedQueue = NULL; // Pointer to the head of the Blocked Queue

// Writes detailed error messages to a serial port
void detailed_error(const char *message, const char *variable_name, int value) {
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s %s = %d\n", message, variable_name ? variable_name : "", value);
    sys_req(WRITE, COM1, buffer, strlen(buffer)); // Send error message to COM1
}

// Allocates memory for a new PCB and initializes its stack
struct pcb* pcb_allocate(void) {
    struct pcb *new_pcb = (struct pcb*) sys_alloc_mem(sizeof(struct pcb)); // Allocate memory for new PCB
    if (new_pcb == NULL) {
        detailed_error("Error: Failed to allocate memory for new PCB.", NULL, 0);
        return NULL;
    }
    memset(new_pcb->stack, 0, 6700); // Initialize stack with zeros
    new_pcb->stack_pointer = (void*)(new_pcb->stack + 6700 - sizeof(struct context)); // Set stack pointer
    return new_pcb;
}

// Frees memory allocated for a PCB
int pcb_free(struct pcb *pcb_to_free) {
    if (pcb_to_free == NULL) {
        detailed_error("Error: Attempted to free a NULL PCB.", NULL, 0);
        return -1;
    }
    sys_free_mem(pcb_to_free->stack); // Free the stack memory
    sys_free_mem(pcb_to_free);        // Free the PCB structure
    return 0;
}

// Sets up a new PCB with specified name, class, and priority
struct pcb* pcb_setup(const char *name, int class, int priority) {
    if (name == NULL || strlen(name) < 1 || strlen(name) > 15 || priority < 0 || priority > 9) {
        detailed_error("Error: Invalid PCB setup parameters.", "Priority", priority);
        return NULL;
    }
    struct pcb *new_pcb = pcb_allocate(); // Allocate a new PCB
    if (new_pcb == NULL) {
        return NULL;
    }
    strncpy(new_pcb->name, name, 15); // Copy the name to the PCB
    new_pcb->name[15] = '\0';         // Ensure null termination
    new_pcb->class = class;           // Set class
    new_pcb->priority = priority;     // Set priority
    new_pcb->exec_state = READY;      // Set execution state to READY
    new_pcb->disp_state = NOT_SUSPENDED; // Set dispatch state to NOT_SUSPENDED
    new_pcb->stack_pointer = &new_pcb->stack[6700] - sizeof(struct context); // Adjust stack pointer
    new_pcb->stack_pointer = (struct context *)new_pcb->stack_pointer;       // Cast to context pointer
    memset(new_pcb->stack_pointer, 0, sizeof(struct context)); // Initialize context to zeros
    return new_pcb;
}

// Finds a PCB in either the Ready or Blocked queue based on its name
struct pcb* pcb_find(const char *name) {
    if (!name) {
        detailed_error("Error: Attempted to find PCB with NULL name.", NULL, 0);
        return NULL;
    }
    struct pcb *current = ReadyQueue; // Start searching in ReadyQueue
    while (current) {
        if (strcmp(current->name, name) == 0) return current; // Return PCB if name matches
        current = current->next; // Move to next PCB in queue
    }
    current = BlockedQueue; // Search in BlockedQueue
    while (current) {
        if (strcmp(current->name, name) == 0) return current; // Return PCB if name matches
        current = current->next; // Move to next PCB in queue
    }
    detailed_error("Error: PCB not found in any queue. Searched for:", "Name", (int) *name);
    return NULL;
}

// Inserts a PCB into the appropriate queue based on its execution state
void pcb_insert(struct pcb* inserted) {
    if (!inserted) {
        detailed_error("Error: Attempted to insert NULL PCB into queue.", NULL, 0);
        return;
    }
    struct pcb **queue; // Pointer to the appropriate queue
    struct pcb *current; // Current PCB in the traversal
    struct pcb *prev = NULL; // Previous PCB in the traversal

    if (inserted->exec_state == BLOCKED) {
        queue = &BlockedQueue; // Select BlockedQueue for BLOCKED state
        inserted->next = *queue; // Insert at the beginning of the queue
        *queue = inserted;
    } else if (inserted->exec_state == READY) {
        queue = &ReadyQueue; // Select ReadyQueue for READY state
        current = *queue;
        while (current && current->priority <= inserted->priority) { // Traverse to find insertion point
            prev = current;
            current = current->next;
        }
        if (prev) {
            prev->next = inserted; // Insert after the previous PCB
        } else {
            *queue = inserted; // Insert at the beginning of the queue
        }
        inserted->next = current; // Set next pointer to the current PCB
    } else {
        detailed_error("Error: PCB has invalid exec_state for insertion.", "Exec_state", inserted->exec_state);
    }
}

// Removes a PCB from its respective queue
int pcb_remove(struct pcb *target) {
    if (!target) {
        detailed_error("Error: Attempted to remove NULL PCB from queue.", NULL, 0);
        return -1;
    }
    struct pcb **queue; // Pointer to the appropriate queue

    if (target->exec_state == READY) {
        queue = &ReadyQueue; // Select ReadyQueue for READY state
    } else if (target->exec_state == BLOCKED) {
        queue = &BlockedQueue; // Select BlockedQueue for BLOCKED state
    } else {
        detailed_error("Error: PCB has invalid exec_state for removal.", "Exec_state", target->exec_state);
        return -1;
    }
    struct pcb *prev = NULL; // Previous PCB in the traversal
    struct pcb *current = *queue; // Current PCB in the traversal

    while (current) {
        if (current == target) {
            if (prev) {
                prev->next = current->next; // Bypass the target PCB in the queue
            } else {
                *queue = current->next; // Remove the target PCB from the beginning of the queue
            }
            return 0;
        }
        prev = current;
        current = current->next; // Move to the next PCB in the queue
    }
    detailed_error("Error: Target PCB not found in its respective queue.", "Target Name", (int) *target->name);
    return -1;
}

// Function to move a PCB from the blocked queue to the ready queue
static void move_pcb_to_ready_queue(struct pcb *pcb_to_move) {
    if (pcb_to_move == NULL) {
        detailed_error("Error: Attempted to move a NULL PCB to the ready queue.", NULL, 0);
        return;
    }

    // Remove PCB from the blocked queue
    if (pcb_remove(pcb_to_move) != 0) {
        detailed_error("Error: Failed to remove PCB from the blocked queue.", "PCB Name", (int)*pcb_to_move->name);
        return;
    }

    // Change the execution state to READY
    pcb_to_move->exec_state = READY;

    // Insert the PCB into the ready queue
    pcb_insert(pcb_to_move);
}

// Function to resume a PCB from a suspended state
void resume_pcb_kernel(struct pcb *pcb_to_resume) {
    if (pcb_to_resume == NULL) {
        detailed_error("Error: Attempted to resume a NULL PCB.", NULL, 0);
        return;
    }
    if (pcb_to_resume->disp_state == SUSPENDED) {
        pcb_to_resume->disp_state = NOT_SUSPENDED;

        // If the PCB is also in the blocked state, move it to the ready queue
        if (pcb_to_resume->exec_state == BLOCKED) {
            move_pcb_to_ready_queue(pcb_to_resume);
        }
    } else {
        detailed_error("Error: PCB is not suspended and cannot be resumed.", "PCB Name", (int)*pcb_to_resume->name);
    }
}
