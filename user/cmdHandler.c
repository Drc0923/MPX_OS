// Created by Dylan Caldwell on 8/18/23.
// ./mpx.sh
#include "cmdHandler.h"
#include "version.h"
#include "help.h"
#include "time.h"
#include "pcb.h"
#include "pcbuser.h"
#include <alarm.h>
#include <yield.h>
#include "load_r3.h"
#include <string.h> // for string manipulation functions
#include <sys_req.h>
#include <mpx/serial.h>
#include <stdlib.h>
#include "yield.h"

#define COM1 0x3F8
#define MAX_WELCOME_SIZE 1024

void detailed_error1(const char *message, const char *variable_name, int value) {
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "%s %s = %d\n", message, variable_name ? variable_name : "", value);
    sys_req(WRITE, COM1, buffer, strlen(buffer));
}

typedef struct {
    const char *command_str;
    void (*command_func)(void);
    const char *output_message;
    int subMenuIndex;  // -1 if no submenu
} command_map_t;

typedef struct {
    const char* menu_name;
    command_map_t* commands;
} menu_t;

#include <stdbool.h>

#define HISTORY_SIZE 5

// Command history structure
typedef struct {
    char history[HISTORY_SIZE][MAX_CMD_SIZE];
    int current;  // The current position in history for recall
    int count;    // Number of commands in history
} CommandHistory;

static CommandHistory cmd_history = {.current = -1, .count = 0};

// Function to add command to history
void add_to_history(const char *cmd) {
    if (cmd && cmd[0]) {
        // Shift history if necessary
        if (cmd_history.count < HISTORY_SIZE) {
            cmd_history.count++;
        } else {
            for (int i = 1; i < HISTORY_SIZE; i++) {
                strcpy(cmd_history.history[i - 1], cmd_history.history[i]);
            }
        }

        // Add new command to history
        strncpy(cmd_history.history[cmd_history.count - 1], cmd, MAX_CMD_SIZE - 1);
        cmd_history.history[cmd_history.count - 1][MAX_CMD_SIZE - 1] = '\0';
        cmd_history.current = cmd_history.count;  // Reset current position
    }
}

// Function to get the previous command from history
const char* get_previous_history() {
    if (cmd_history.current <= 0) {
        return NULL;  // No more history
    }
    cmd_history.current--;
    return cmd_history.history[cmd_history.current];
}

// Function to get the next command from history
const char* get_next_history() {
    if (cmd_history.current == -1 || cmd_history.current >= cmd_history.count - 1) {
        return NULL;  // No more history
    }
    cmd_history.current++;
    return cmd_history.history[cmd_history.current];
}

static command_map_t main_commands[] = {

        {"Help", help, "Displaying available commands...\n", -1},
        {"Time/Date Functions", NULL, "Navigating to Time/Date Functions...\n", 1},
        {"PCB Functions", NULL, "Navigating to PCB Functions...\n", 2},
        {"Version", version, "Displaying Version...\n", -1},
//        {"Yield",yield, "Yielding current process...\n", -1},
        {"Alarm", get_alarm, "Setting Alarm...\n", -1},
        {NULL, NULL, NULL, -1}
};

static command_map_t time_date_commands[] = {
        {"Set Time", setTime, "Setting Time...\n", -1},
        {"Get Time", get_time, "Current Time is...\n", -1},
        {"Set Date", setDate, "Setting Date...\n", -1},
        {"Get Date", getDate, "Getting Date...\n", -1},
        {"Return to Main Menu", NULL, "Returning...\n", 0},
        {NULL, NULL, NULL, -1}
};

static command_map_t pcb_commands[] = {
        {"Load R3", load_r3, "Loading R3 test processes...\n", -1},
//        {"Create PCB", create_pcb, "Creating PCB...\n", -1}, REMOVED FOR M3
        {"Delete PCB", delete_pcb, "Deleting PCB...\n", -1},
        //{"Block PCB", block_pcb, "Blocking PCB...\n", -1},
        //{"Unblock PCB", unblock_pcb, "Unblocking PCB...\n", -1},
        {"Suspend PCB", suspend_pcb, "Suspending PCB...\n", -1},
        {"Resume PCB", resume_pcb, "Resuming PCB...\n", -1},
        {"Set Priority", set_priority, "Setting Priority...\n", -1},
        {"Show PCB", show_pcb, "Showing PCB...\n", -1},
        {"Show Ready", show_ready, "Displaying processes in READY state...\n", -1},
        {"Show Blocked", show_blocked, "Displaying processes in BLOCKED state...\n", -1},
        {"Show All", show_all, "Displaying all processes...\n", -1},
        {"Return to Main Menu", NULL, "Returning...\n", 0},
        {NULL, NULL, NULL, -1}
};

static menu_t menus[] = {
        {"\033[0;34mMain Menu\033[0;37m", main_commands},
        {"Time/Date Menu", time_date_commands},
        {"PCB Menu", pcb_commands}
};

static int current_menu = 0;

void generate_welcome_message(char *buf) {
    if (!buf) {
        detailed_error1("Error: generate_welcome_message received a NULL buffer.", NULL, 0);
        return;
    }

    menu_t currentMenu = menus[current_menu];

    if (!currentMenu.menu_name || !currentMenu.commands) {
        detailed_error1("Error: Current menu or commands are not initialized properly.", NULL, 0);
        return;
    }

    strcpy(buf, "\n\033[0;34mWelcome to MPX.\033[0;37m ");
    strcat(buf, currentMenu.menu_name);
    strcat(buf, "\n");

    int index = 1;
    for (int i = 0; currentMenu.commands[i].command_str; i++) {
        if (!currentMenu.commands[i].command_str) {
            detailed_error1("\033[0;31mError: Command string at index is NULL.", "Index", i);
            continue;
        }
        char option[100];
        sprintf(option, "%d) %s\n", index, currentMenu.commands[i].command_str);
        strcat(buf, option);
        index++;
    }

    strcat(buf, "\033[0;36mEnter choice: \033[0;37m");
}

static int shutdown_requested = 0;  // 0 for false, 1 for true

static void process_command(const char *command) {

    // Shutdown command
    if (strcmp(command, "shutdown") == 0) {
        char confirm_msg[] = "Are you sure you want to shut down? (y/n): ";
        sys_req(WRITE, COM1, confirm_msg, strlen(confirm_msg));

        char confirm[5] = {0};
        sys_req(READ, COM1, confirm, sizeof(confirm) - 1);
        confirm[4] = '\0';

        int confirmRead = strlen(confirm);
        if (confirm[confirmRead - 1] == '\n' || confirm[confirmRead - 1] == '\r') {
            confirm[confirmRead - 1] = '\0';
        }
        if (strcmp(confirm, "y") == 0) {
            shutdown_requested = 1;  // set the flag true
            // r4
            sys_req(EXIT);
            //return;
        }
    } 
    // Clear Command
    else if(strcmp(command, "clear") == 0) {
    	const char *clearCode = "\x1b[2J\x1b[H\x1b[3J";
    	sys_req(WRITE, COM1, clearCode, strlen(clearCode));
    	current_menu = 0;
    	
    } else {
        // Parse the entered command into an integer
        int choice = atoi(command);
        if (choice <= 0) {
            char error_msg[] = "\033[0;31mInvalid choice. Please enter a valid number.\n";
            sys_req(WRITE, COM1, error_msg, strlen(error_msg));
            return;
        }

        menu_t currentMenu = menus[current_menu];
        if (currentMenu.commands[choice - 1].command_str == NULL) {
            char error_msg[] = "\033[0;31mInvalid choice. Please enter a valid number.\n";
            sys_req(WRITE, COM1, error_msg, strlen(error_msg));
            return;
        }

        // Check if the command has a function to execute
        if (currentMenu.commands[choice - 1].command_func) {
            currentMenu.commands[choice - 1].command_func();
            return;
        }
        // Check if the command is meant to navigate to another menu
        if (currentMenu.commands[choice - 1].subMenuIndex >= 0) {
            current_menu = currentMenu.commands[choice - 1].subMenuIndex;
            return;
        }
    }
}


void comhand(void) {
    char buf[101] = {0};

    char welcome_msg[MAX_WELCOME_SIZE];
    generate_welcome_message(welcome_msg);
    if (strlen(welcome_msg) == 0) {
        detailed_error1("\033[0;31mError: Failed to generate welcome message.", NULL, 0);
        return;
    }

    sys_req(WRITE, COM1, welcome_msg, strlen(welcome_msg));

    for (;;) {
        if (shutdown_requested) {
            break;
        }

        memset(buf, 0, sizeof(buf));

        int nread = serial_poll(COM1, buf, sizeof(buf) - 1);

        if (nread < 0) {
            detailed_error1("\033[0;31mError: serial_poll failed to read input.", "Bytes read", nread);
            continue;
        }


        buf[nread] = '\0';

        if (buf[nread - 1] == '\n' || buf[nread - 1] == '\r') {
            buf[nread - 1] = '\0';
        }

        process_command(buf);
        add_to_history(buf);
        generate_welcome_message(welcome_msg);
        sys_req(WRITE, COM1, welcome_msg, strlen(welcome_msg));
    }
}
