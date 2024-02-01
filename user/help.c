#include "help.h"
#include <sys_req.h>
#include <string.h>

// Help Command Structure
typedef struct {
    const char *name;
    const char *description;
    const char *args; // NULL if no arguments
} CommandHelp;

// List of Commands
const CommandHelp commands[] = {
        {"Version", "Displays the current version and update date of MPX", NULL},
        {"Shutdown", "Halts all processes and shuts down the OS", NULL},
        {"Clear", "Clears the text currently inside of the terminal and redisplays the menu", NULL},
        {"SetTime", "Sets the time on the operating system", "Three user inputs of 'hh', 'mm', 'ss'"},
        {"GetTime", "Gets the current time saved on the operating system", NULL},
        {"SetDate", "Sets the date on the operating system", "Three user inputs of 'mm', 'dd', 'yy'"},
        {"GetDate", "Gets the current date saved on the operating system", NULL},
        {"LoadR3", "Loads in processes from processes.h", NULL},
        {"DeletePCB", "Deletes a PCB based on name given by user", "User input of the name of the PCB to be deleted"},
        {"BlockPCB", "Moves a PCB to the blocked state based on name given by user", "User input of the name of the PCB to be moved to blocked"},
        {"UnblockPCB", "Moves a PCB to the unblocked/ready state based on name given by user", "User input of the name of the PCB to be moved to unblocked/ready"},
        {"SuspendPCB", "Moves a PCB to the suspended state based on name given by user", "User input of the name of the PCB to be moved to suspended"},
        {"ResumePCB", "Moves a PCB out of the suspended state based on name given by user", "User input of the name of the PCB to be moved out of suspended"},
        {"SetPriority", "Changes the priority of a PCB given by the user", "User input of the name of the PCB to change and the new priority (0-9)"},
        {"ShowPCB", "Displays the PCB with the given name", "User input of the name of the PCB to display"},
        {"ShowReady", "Displays all the PCBs currently in the ready queue", NULL},
        {"ShowBlocked", "Displays all the PCBs currently in the blocked queue", NULL},
        {"ShowAll", "Displays all the PCBs currently in the system", NULL},
        //  Add any other commands here
        {NULL, NULL, NULL} // Sentinel to mark end of the array
};

void help(void) {
    const char *header = "MPX Project Help Page\n\n";
    sys_req(WRITE, COM1, header, strlen(header));

    char buffer[1024]; // Adjust buffer size as needed
    for (int i = 0; commands[i].name != NULL; i++) {
        // Output name and description
        snprintf(buffer, sizeof(buffer), "\033[0;32mName\n\t%s\033[0;37m - %s\n\n", commands[i].name, commands[i].description);
        sys_req(WRITE, COM1, buffer, strlen(buffer));

        // Output arguments if they exist
        if (commands[i].args != NULL) {
            snprintf(buffer, sizeof(buffer), "\033[0;36mArguments\n\t\033[0;37m%s\n\n", commands[i].args);
            sys_req(WRITE, COM1, buffer, strlen(buffer));
        }
    }
}
