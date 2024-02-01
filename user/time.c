//
// Created by Chris Henry on 9/1/23.
//

#include <mpx/io.h>
#include "mpx/gdt.h"
#include "mpx/interrupts.h"
#include <mpx/device.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys_req.h>
#include <string.h>
#include "time.h"

// starting on
#define RTC_INDEX_PORT 0x70
#define RTC_DATA_PORT 0x71
#define RTC_SECONDS_REG 0x00
#define RTC_MINUTES_REG 0x02
#define RTC_HOURS_REG 0x04
#define DAY 0x07
#define MONTH 0x08
#define YEAR 0x09


// Function to read from RTC registers
int readRTC(int reg) {
    outb(RTC_INDEX_PORT,reg); // Write the index to the RTC index register
    return inb(RTC_DATA_PORT); // Read the value from the RTC data register
}


// Function to convert BCD to decimal
int bcdToDecimal(int bcd) {
    return ((bcd >> 4) & 0x0F) * 10 + (bcd & 0x0F);
}




// Function to get the current time
void get_time(void) {
    int seconds,minutes, hours;

    // Disable interrupts
    cli();

    // Read seconds, minutes, and hours from RTC registers
    seconds = bcdToDecimal(readRTC(RTC_SECONDS_REG));
    minutes = bcdToDecimal(readRTC(RTC_MINUTES_REG));
    hours = bcdToDecimal(readRTC(RTC_HOURS_REG));

    // Enable interrupts
    sti();

    // Display the current time
    char time_msg[] = "\n Current time \nUpdated on September 10th, 2023\n";
    char hours_msg[] = "Hour: ";
    char min_msg[] = " Minute: ";
    char sec_msg[] = " Second: ";
    char UTC_space[] = " UTC Time ";
    char seconds_string[20] = {0};
    char minutes_string[20] = {0};
    char hours_string[20] = {0};



    itoa(seconds, seconds_string, 10);
    itoa(minutes, minutes_string, 10);
    itoa(hours, hours_string,10);


    //sys_req(WRITE, COM1, time_msg, hours_string, minutes_string, seconds_string);
    sys_req(WRITE, COM1, time_msg, strlen(time_msg));
    // break for hours
    sys_req(WRITE, COM1, hours_msg, strlen(hours_msg));
    sys_req(WRITE, COM1, hours_string, strlen(hours_string));
    // break for minutes
    sys_req(WRITE, COM1, min_msg, strlen(min_msg));
    sys_req(WRITE, COM1, minutes_string, strlen(minutes_string));

    // break for seconds
    sys_req(WRITE, COM1, sec_msg, strlen(sec_msg));
    sys_req(WRITE, COM1, seconds_string, strlen(seconds_string));

    sys_req(WRITE, COM1, UTC_space, strlen(UTC_space));
    sys_req(WRITE, COM1, "\n\n", strlen("\n\n"));

}

void setTime(void){
    int hours;
    int mins;
    int secs;

    // getting the hours
    char hourMessage[] = "Please enter the hours. \n";
    sys_req(WRITE, COM1, hourMessage, strlen(hourMessage));

    char userHour[10] = {0};
    int nread = sys_req(READ, COM1, userHour, sizeof(userHour));
    sys_req(WRITE, COM1, userHour, nread);

    // Getting the mins
    char dayMins[] = "Please enter the Mins.  \n";
    sys_req(WRITE, COM1, dayMins, strlen(dayMins));

    char userMin[10] = {0};
    nread = sys_req(READ, COM1, userMin, sizeof(userMin));
    sys_req(WRITE, COM1, userMin, nread);


    // Getting the secs
    char secMessage[] = "Please enter the seconds. \n";
    sys_req(WRITE, COM1, secMessage, strlen(secMessage));

    char userSec[10] = {0};
    nread = sys_req(READ, COM1, userSec, sizeof(userSec));
    sys_req(WRITE, COM1, userSec, nread);

    hours = atoi(userHour);
    mins = atoi(userMin);
    secs = atoi(userSec);

    if(secs < 0 || secs > 59) {
    	char secsError[] = "\033[0;31mError: Seconds is out of the range 00 - 59\n\n";
	sys_req(WRITE, COM1, secsError, strlen(secsError));
	return;
    }
    
    if(mins < 0 || mins > 59) {
    	char minsError[] = "\033[0;31mError: Minutes is out of the range 00 - 59\n\n";
	sys_req(WRITE, COM1, minsError, strlen(minsError));
	return;
    }
    
    if(hours < 0 || hours > 23) {
    	char hourError[] = "\033[0;31mError: Hours is out of the range 00 - 23\n\n";
	sys_req(WRITE, COM1, hourError, strlen(hourError));
	return;
    }

    // Writing the the clock (BCD conversion)
    outb(RTC_INDEX_PORT, RTC_HOURS_REG);
    outb(RTC_DATA_PORT, (((hours / 10) << 4) | (hours % 10)));

    outb(RTC_INDEX_PORT, RTC_MINUTES_REG);
    outb(RTC_DATA_PORT, (((mins / 10) << 4) | (mins % 10)));

    outb(RTC_INDEX_PORT, RTC_SECONDS_REG);
    outb(RTC_DATA_PORT, (((secs / 10) << 4) | (secs % 10)));

    char finaltimeMessage[] = "New time set to: ";
    sys_req(WRITE, COM1, finaltimeMessage, strlen(finaltimeMessage));
    get_time();

}

void getDate(void){

	// Defining the variables for day, month and year
	int day;
	int month;
	int year;
	char* day_s = "";
	char* month_s = "";
	char* year_s = "";

	// Displaying the month
	outb(RTC_INDEX_PORT, MONTH);
	char temp = inb(0x71);
	month = ((temp >> 4) &0x0f) * 10 + (temp &0x0f);

	// Checks if Month is a single digit if so add a leading 0
	if(month < 10){
		sys_req(WRITE, COM1, "0", strlen("0"));
	}

	itoa(month, month_s, 10);
	sys_req(WRITE, COM1, month_s, strlen(month_s));
	sys_req(WRITE, COM1, "/", strlen("/"));

	// Displaying the day
	outb(RTC_INDEX_PORT, DAY);
	temp = inb(0x71);
	day = ((temp >> 4) &0x0f) * 10 + (temp &0x0f);

	// Checks if Date is a single digit if so add a leading 0
	if(day < 10){
		sys_req(WRITE, COM1, "0", strlen("0"));
	}

	itoa(day, day_s, 10);
	sys_req(WRITE, COM1, day_s, strlen(day_s));
	sys_req(WRITE, COM1, "/", strlen("/"));

	// Displaying the year
	outb(RTC_INDEX_PORT, YEAR);
	temp = inb(0x71);
	year = ((temp >> 4) &0x0f) * 10 + (temp &0x0f);

	// Checks if Year is a single digit if so add a leading 0
	if(year < 10){
		sys_req(WRITE, COM1, "0", strlen("0"));
	}

	itoa(year, year_s, 10);
	sys_req(WRITE, COM1, year_s, strlen(year_s));
	sys_req(WRITE, COM1, "\n\n", strlen("\n\n"));

}

void setDate(void){

	int day;
	int month;
	int year;
	int leap = 0;
	int daysInMonth = 0;

	// Getting the month
	char monthMessage[] = "Please enter a the desired month in the form of mm (ex. 04)\n";
	sys_req(WRITE, COM1, monthMessage, strlen(monthMessage));

	char userMonth[10] = {0};
	int nread = sys_req(READ, COM1, userMonth, sizeof(userMonth));
	sys_req(WRITE, COM1, userMonth, nread);
	
	// Getting the day
	char dayMessage[] = "Please enter a the desired day in the form of dd (ex. 09)\n";
	sys_req(WRITE, COM1, dayMessage, strlen(dayMessage));

	char userDay[10] = {0};
	nread = sys_req(READ, COM1, userDay, sizeof(userDay));
	sys_req(WRITE, COM1, userMonth, nread);
	
	// Getting the year
	char yearMessage[] = "Please enter a the desired year in the form of yy (ex. 01)\n";
	sys_req(WRITE, COM1, yearMessage, strlen(yearMessage));

	char userYear[10] = {0};
	nread = sys_req(READ, COM1, userYear, sizeof(userYear));
	sys_req(WRITE, COM1, userYear, nread);
	
	day = atoi(userDay);
	month = atoi(userMonth);
	year = atoi(userYear);

	// Check for if the year is valid
	if(year < 0 || year > 99) {
		char yearError[] = "\033[0;31mError: Year is out of the range 00 - 99\n\n";
		sys_req(WRITE, COM1, yearError, strlen(yearError));
		return;
	}

	// Checks for a leap year
	if(year % 4 == 0) {
		leap = 1;
	}

	// Checks if month is valid
	if(month < 1 || month > 12) {
		char monthError[] = "\033[0;31mError: Month is out of the range 01 - 12\n\n";
		sys_req(WRITE, COM1, monthError, strlen(monthError));
		return;
	}

	// Checks how many days a month can have (based on the month given)
	// If not a leap year
	if(leap == 0) {

		// Months with 30 days
		if(month == 4 || month == 6 || month == 9 || month == 11) {
			daysInMonth = 30;
		}
		// Months with 31 days
		else if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) {
			daysInMonth = 31;
		}
		// February
		else{
			daysInMonth = 28;
		}
	}
	// Else it is a leap year
	else {

		// Months with 30 days
		if(month == 4 || month == 6 || month == 9 || month == 11) {
			daysInMonth = 30;
		}
		// Months with 31 days
		else if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) {
			daysInMonth = 31;
		}
		// February
		else{
			daysInMonth = 29;
		}
	}

	// Check if the day given is valid based on the month
	if(day < 1 || day > daysInMonth) {
		char dayError[] = "\033[0;31mError: Day is out of the range for the month that was given\n\n";
		sys_req(WRITE, COM1, dayError, strlen(dayError));
		return;
	}
	
	// Writing the the clock (BCD conversion)
	outb(RTC_INDEX_PORT, YEAR);
	outb(RTC_DATA_PORT, (((year / 10) << 4) | (year % 10)));
	
	outb(RTC_INDEX_PORT, MONTH);
	outb(RTC_DATA_PORT, (((month / 10) << 4) | (month % 10)));
	
	outb(RTC_INDEX_PORT, DAY);
	outb(RTC_DATA_PORT, (((day / 10) << 4) | (day % 10)));
	
	char finalMessage[] = "New date set to: ";
	sys_req(WRITE, COM1, finalMessage, strlen(finalMessage));
	getDate();
}


