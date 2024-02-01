// This is the header file for alarm.c

#include <pcb.h>
#include <processes.h>

struct time {
	int hours;
	int minutes;
	int seconds;
};

void alarm(struct time, char*);
void get_alarm(void);
