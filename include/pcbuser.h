#include <stddef.h>
#include <pcb.h>

void create_pcb(void);
void delete_pcb(void);
//void block_pcb(void);
//void unblock_pcb(void);
void suspend_pcb(void);
void resume_pcb(void);
void set_priority(void);
void show_pcb(void);
void show_ready(void);
void show_blocked(void);
void show_all(void);
void display_pcb(struct pcb *target);

