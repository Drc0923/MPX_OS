// This will be the file for the alarm command.

#include <alarm.h>
#include <pcb.h>
#include <processes.h>
#include <mpx/io.h>
#include <sys_req.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define RTC_INDEX_PORT 0x70
#define RTC_DATA_PORT 0x71
#define RTC_SECONDS_REG 0x00
#define RTC_MINUTES_REG 0x02
#define RTC_HOURS_REG 0x04


void alarm(struct time trigger, char* msg) {

	for(;;) {
		int current_s, current_m, current_h;
	
			// Read seconds, minutes, and hours from RTC registers
		current_s = bcdToDecimal(readRTC(RTC_SECONDS_REG));
		current_m = bcdToDecimal(readRTC(RTC_MINUTES_REG));
		current_h = bcdToDecimal(readRTC(RTC_HOURS_REG));
	
		// If the alarm is past the right time to alert the user
		if((current_h > trigger.hours) || (current_h == trigger.hours && current_m > trigger.minutes) || (current_h == trigger.hours && current_m == trigger.minutes && current_s > trigger.seconds)) {
			sys_req(WRITE, COM1, msg, strlen(msg));
			sys_req(WRITE, COM1, "\n", strlen("\n"));
			break;
		}
		else {
			sys_req(IDLE);
		}
	}
	sys_req(EXIT);
}

void get_alarm(void) {

	// Prompt user to enter the time for the alarm to go off
	char time_msg[] = "Please enter the time to set the alarm for in the form hh:mm:ss\n";
	sys_req(WRITE, COM1, time_msg, strlen(time_msg));
	
	char alarmTime[10] = {0};
	int nread = sys_req(READ, COM1, alarmTime, sizeof(alarmTime));
	sys_req(WRITE, COM1, alarmTime, nread);
	
	// If the time given by the user is not in the correct format, fail out and return
	if((alarmTime[2] != ':') || (alarmTime[5] != ':')){
		char errorMsg[] = "\033[0;31mTime given for the alarm to go off is not in the proper format hh:mm:ss\n\n";
		sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
		return;
	}
	
	// Uses the atoi function to caputere the specific hours, minutes and seconds into corresponding variables
	int hours = atoi(&alarmTime[0]);
	int minutes = atoi(&alarmTime[3]);
	int seconds = atoi(&alarmTime[6]);
	
	// Checks to make sure that the time the user gave is within the range to be valid for an alarm
	if((hours > 23) || (hours < 0)) {
		char errorMsg[] = "\033[0;31mThe hours given is out of the range 0 - 23\n\n";
		sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
		return;
	}
	else if((minutes > 59) || (minutes < 0)) {
		char errorMsg[] = "\033[0;31mThe minutes given is out of the range 0 - 59\n\n";
		sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
		return;
	}
	else if((seconds > 59) || (seconds < 0)) {
		char errorMsg[] = "\033[0;31mThe seconds given is out of the range 0 - 59\n\n";
		sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
		return;
	}
	
	// If we get here the time entered is in the correct format and valid so we can take a message
	char user_msg[] = "Please enter the message you would like to get at that time\n";
	sys_req(WRITE, COM1, user_msg, strlen(user_msg));
	
	char msg[100] = {0};
	int inread = sys_req(READ, COM1, msg, sizeof(msg));
	sys_req(WRITE, COM1, msg, inread);
	
	// Create a struct time to store the info for this alarm
	struct time alarm_time;
	alarm_time.hours = hours;
	alarm_time.minutes = minutes;
	alarm_time.seconds = seconds;
	
	alarm(alarm_time, msg);
}



