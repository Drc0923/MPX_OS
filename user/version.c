// This file will be for showing the current version, and date.
// Created by David Clark on 8/25/23

#include "version.h"
#include <string.h>
#include <sys_req.h>

void version(void) {
	// Char array to update for each version of MPX
	// Style will be in R#.? where # is the current module being worked on and ? starts at 1 and goes up by one for each
	// big change to the Development branch (Ex. R3.2 would be the 3rd module and 2nd major change to Development branch).
	char version_msg[] = "\n\033[0;34mCurrent Version:\033[0;31m R6.15\n\033[0;37mUpdated on October 3rd, 2023\n\n";

	// Outputing the version
	sys_req(WRITE, COM1, version_msg, strlen(version_msg));
}
