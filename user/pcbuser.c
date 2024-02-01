#include "pcbuser.h"
#include "pcb.h"
#include "time.h"
#include <string.h>
#include <sys_req.h>
#include <stdlib.h>
#include <mpx/serial.h>



void create_pcb(void) {
    char name[50] = {0};
    int pcbClass = 0;
    int priority = 0;
    struct pcb* userPCB = NULL;

    // NAME LOOP
    while (1) {
        char askForName[] = "\nPlease enter the PCB name: ";
        sys_req(WRITE, COM1, askForName, strlen(askForName));

        int userIn = sys_req(READ, COM1, name, sizeof(name) - 1);
        name[userIn] = '\0'; // Null-terminate
        while (userIn > 0 && (name[userIn-1] == '\n' || name[userIn-1] == '\r')) {
            name[--userIn] = '\0';
        }

        if (strlen(name) > 10) {
            char nameMsg[] = "\033[0;31mName entered is greater than 10 characters.\n";
            sys_req(WRITE, COM1, nameMsg, strlen(nameMsg));
            continue;
        }

        if (pcb_find(name) != NULL) {
            char nameMsg[] = "\033[0;31mName entered is already in use.\n";
            sys_req(WRITE, COM1, nameMsg, strlen(nameMsg));
            continue;
        }

        break; // If we make it here name is fine, exit the loop
    }

    // CLASS LOOP
    while (1) {
        char classMsg[] = "\nPlease enter the class of the PCB: User - 0, System - 1. ";
        sys_req(WRITE, COM1, classMsg, strlen(classMsg));

        char classIn[50] = {0};
        int userIn = sys_req(READ, COM1, classIn, sizeof(classIn) - 1);
        classIn[userIn] = '\0'; // Null-terminate
        while (userIn > 0 && (classIn[userIn-1] == '\n' || classIn[userIn-1] == '\r')) {
            classIn[--userIn] = '\0';
        }

        if (strcmp(classIn, "0") == 0 || strcmp(classIn, "1") == 0) {
            pcbClass = atoi(classIn);
            break;
        }

        char errorMsg[] = "\033[0;31mUser input doesn't match 0 (user) or 1 (system), please try again.\n";
        sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
    }

    // PRIORITY LOOP
    while (1) {
        char askForPriority[] = "\nPlease enter the priority level: User - 0, System - 1. ";
        sys_req(WRITE, COM1, askForPriority, strlen(askForPriority));

        char priorityIn[50] = {0};
        int userIn = sys_req(READ, COM1, priorityIn, sizeof(priorityIn) - 1);
        priorityIn[userIn] = '\0'; // Null-terminate
        while (userIn > 0 && (priorityIn[userIn-1] == '\n' || priorityIn[userIn-1] == '\r')) {
            priorityIn[--userIn] = '\0';
        }

        priority = atoi(priorityIn);

        if (priority >= 0 && priority <= 9) {
            break;
        }

        char errorMsg[] = "\033[0;31mUser input is not between 0 and 9, please try again.\n";
        sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
    }

    userPCB = pcb_setup(name, pcbClass, priority);
    if (userPCB == NULL) {
        char errorMsg[] = "\033[0;31mError in creating PCB.\n";
        sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
        return; // exit function if pcb creation failed
    }

    pcb_insert(userPCB);

    // Notify user of the creation
    char successMsg[100];
    sprintf(successMsg, "\nPCB %s created successfully.\n", name);
    sys_req(WRITE, COM1, successMsg, strlen(successMsg));
}



void delete_pcb(void) {
    char name[50] = {0};

    // Prompts user for PCB name
    char askForName[] = "\nPlease enter the PCB name: ";
    sys_req(WRITE, COM1, askForName, strlen(askForName));

    // Reads in user response
    int userIn = sys_req(READ, COM1, name, sizeof(name) - 1);
    name[userIn] = '\0'; // Null-terminate
    while (userIn > 0 && (name[userIn-1] == '\n' || name[userIn-1] == '\r')) {
        name[--userIn] = '\0';
    }

    struct pcb *targetPCB = pcb_find(name);

    // Checks to see if the name exists
    if (!targetPCB) {
        char nameMsg[] = "\033[0;31mName entered does not exist.\n";
        sys_req(WRITE, COM1, nameMsg, strlen(nameMsg));
        return;
    }

    // Checks to see if the name is a system process
    if (targetPCB->class == 1) {
        char nameMsg[] = "\033[0;31mName entered is a system process and can't be deleted.\n";
        sys_req(WRITE, COM1, nameMsg, strlen(nameMsg));
        return;
    }

    // Remove the PCB and free associated memory
    pcb_remove(targetPCB);
    pcb_free(targetPCB);

    // Notify user of the deletion
    char successMsg[100];
    sprintf(successMsg, "\nPCB %s deleted successfully.\n", name);
    sys_req(WRITE, COM1, successMsg, strlen(successMsg));
}



//void block_pcb(void) {
//    char name[50] = {0};
//
//    char askForName[] = "\nPlease enter the PCB name: ";
//    sys_req(WRITE, COM1, askForName, strlen(askForName));
//
//    int userIn = sys_req(READ, COM1, name, sizeof(name) - 1);
//    name[userIn] = '\0';
//    while (userIn > 0 && (name[userIn-1] == '\n' || name[userIn-1] == '\r')) {
//        name[--userIn] = '\0';
//    }
//
//    struct pcb* block_pcb = pcb_find(name);
//    if (block_pcb == NULL) {
//        char nameMsg[] = "\033[0;31mName entered does not exist.\n";
//        sys_req(WRITE, COM1, nameMsg, strlen(nameMsg));
//        return;
//    }
//
//    if (block_pcb->class == SYSTEM_PROCESS) {
//        char errorMsg[] = "\033[0;31mThe entered name corresponds to a system process and cannot be blocked.\n";
//        sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
//        return;
//    }
//
//    if (block_pcb->exec_state == BLOCKED) {
//        char errorMsg[] = "\033[0;31mThe process is already in the blocked state.\n";
//        sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
//        return;
//    }
//
//    pcb_remove(block_pcb);
//    block_pcb->exec_state = BLOCKED;
//    pcb_insert(block_pcb);
//
//    char successMsg[100];
//    sprintf(successMsg, "\nPCB %s has been blocked successfully.\n", name);
//    sys_req(WRITE, COM1, successMsg, strlen(successMsg));
//}
//
//void unblock_pcb(void) {
//    char name[50] = {0};
//
//    char askForName[] = "\nPlease enter the PCB name: ";
//    sys_req(WRITE, COM1, askForName, strlen(askForName));
//
//    int userIn = sys_req(READ, COM1, name, sizeof(name) - 1);
//    name[userIn] = '\0';
//    while (userIn > 0 && (name[userIn-1] == '\n' || name[userIn-1] == '\r')) {
//        name[--userIn] = '\0';
//    }
//
//    struct pcb* unblock_pcb = pcb_find(name);
//    if (unblock_pcb == NULL) {
//        char nameMsg[] = "\033[0;31mName entered does not exist.\n";
//        sys_req(WRITE, COM1, nameMsg, strlen(nameMsg));
//        return;
//    }
//
//    if (unblock_pcb->exec_state == READY) {
//        char errorMsg[] = "\033[0;31mThe process is already in the unblocked (or ready) state.\n";
//        sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
//        return;
//    }
//
//    pcb_remove(unblock_pcb);
//    unblock_pcb->exec_state = READY;
//    pcb_insert(unblock_pcb);
//
//    char successMsg[100];
//    sprintf(successMsg, "\nPCB %s has been unblocked successfully.\n", name);
//    sys_req(WRITE, COM1, successMsg, strlen(successMsg));
//}


void suspend_pcb(void) {
    char name[50] = {0};

    // Prompts user for PCB name
    char askForName[] = "\nPlease enter the PCB name: ";
    sys_req(WRITE, COM1, askForName, strlen(askForName));

    // Reads in user response
    int userIn = sys_req(READ, COM1, name, sizeof(name) - 1);
    name[userIn] = '\0'; // Null-terminate
    while (userIn > 0 && (name[userIn-1] == '\n' || name[userIn-1] == '\r')) {
        name[--userIn] = '\0';
    }

    // Checks if the PCB exists
    if(pcb_find(name) == NULL) {
        char nameMsg[] = "\033[0;31mName entered does not exist.\n";
        sys_req(WRITE, COM1, nameMsg, strlen(nameMsg));
        return;
    }
        // Check if the process is a system process
    else if(pcb_find(name)->class == 1) {
        char sysMsg[] = "\033[0;31mA system process can't be suspended.\n";
        sys_req(WRITE, COM1, sysMsg, strlen(sysMsg));
        return;
    }
    else {
        // Find the PCB and remove it from current queue
        struct pcb* suspend_pcb = pcb_find(name);
        pcb_remove(suspend_pcb);

        // Changes disp_state to suspended
        suspend_pcb->disp_state = 1;

        // Re-inserts the PCB to appropriate queue
        pcb_insert(suspend_pcb);
    }

    // Notify user of the suspension
    char successMsg[100];
    sprintf(successMsg, "\nPCB %s has been suspended successfully.\n", name);
    sys_req(WRITE, COM1, successMsg, strlen(successMsg));
}


void resume_pcb(void) {
    char name[50] = {0};

    // Prompts user for PCB name
    char askForRName[] = "\nPlease enter the PCB name you wish to resume: ";
    sys_req(WRITE, COM1, askForRName, strlen(askForRName));

    // Reads in user response
    int userIn = sys_req(READ, COM1, name, sizeof(name) - 1);
    name[userIn] = '\0'; // Null-terminate
    while (userIn > 0 && (name[userIn-1] == '\n' || name[userIn-1] == '\r')) {
        name[--userIn] = '\0';
    }

    // Checks if the PCB exists
    struct pcb* resume_pcb = pcb_find(name);
    if (resume_pcb == NULL) {
        char nameMsg[] = "\033[0;31mName entered does not exist.\n";
        sys_req(WRITE, COM1, nameMsg, strlen(nameMsg));
        return;
    }

    // Check if the process is already in the not suspended state
    if (resume_pcb->disp_state == 0) {
        char errorMsg[] = "\033[0;31mThe process is already in the not suspended state.\n";
        sys_req(WRITE, COM1, errorMsg, strlen(errorMsg));
        return;
    }

    // Remove the PCB from its current queue
    pcb_remove(resume_pcb);

    // Changes disp_state to not suspended
    resume_pcb->disp_state = 0;

    // Re-inserts the PCB to the appropriate queue
    pcb_insert(resume_pcb);

    // Notify user of the resuming
    char successMsg[100];
    sprintf(successMsg, "\nPCB %s has been resumed successfully.\n", name);
    sys_req(WRITE, COM1, successMsg, strlen(successMsg));
}


void set_priority(void) {
    char name[50] = {0};
    char priorityInput[10] = {0};
    int priority = 0;

    // Prompts user for PCB name
    char askForName[] = "Please enter the PCB you wish to set priority for: ";
    sys_req(WRITE, COM1, askForName, strlen(askForName));

    // Reads in user response for name
    int userIn = sys_req(READ, COM1, name, sizeof(name) - 1);
    sys_req(WRITE, COM1, name, userIn);

    // Checks if the PCB exists
    struct pcb* targetPCB = pcb_find(name);
    if (targetPCB == NULL) {
        char nameMsg[] = "\033[0;31mName entered does not exist.\n";
        sys_req(WRITE, COM1, nameMsg, strlen(nameMsg));
        return;
    }

    // Prompts user for the new priority
    char askForPriority[] = "Please enter the new priority (0-9): ";
    sys_req(WRITE, COM1, askForPriority, strlen(askForPriority));

    // Reads in user response for priority
    userIn = sys_req(READ, COM1, priorityInput, sizeof(priorityInput) - 1);

    // Convert priority input to integer and validate
    priority = atoi(priorityInput);
    if (priority < 0 || priority > 9) {
        char invalidMsg[] = "\033[0;31mInvalid priority. Priority must be between 0 and 9.\n";
        sys_req(WRITE, COM1, invalidMsg, strlen(invalidMsg));
        return;
    }

    // Remove the PCB from its current queue
    if (pcb_remove(targetPCB) == -1) {
        char errMsg[] = "\033[0;31mError removing PCB from queue.\n";
        sys_req(WRITE, COM1, errMsg, strlen(errMsg));
        return;
    }

    // Set its new priority
    targetPCB->priority = priority;

    // Re-insert it based on the new priority
    pcb_insert(targetPCB);

    // Updated success message to display the PCB name and its priority
    char successMsg[100];
    sprintf(successMsg, "Priority for PCB '%s' successfully set to %d.\n", name, priority);
    sys_req(WRITE, COM1, successMsg, strlen(successMsg));
}


void show_pcb(void) {
    char name[50] = {0};

    // Prompts user for PCB name
    char askForName[] = "Please enter the PCB name: ";
    sys_req(WRITE, COM1, askForName, strlen(askForName));

    // Reads in user response
    int userIn = sys_req(READ, COM1, name, sizeof(name) - 1);
    sys_req(WRITE, COM1, name, userIn);

    struct pcb *showtarget = pcb_find(name);

    // Checks if the PCB exists
    if(showtarget == NULL) {
        char nameMsg[] = "\033[0;31mName entered does not exist.\n";
        sys_req(WRITE, COM1, nameMsg, strlen(nameMsg));
        return;
    }

    // Convert class to string for display
    char class_string[20] = {0};
    if (showtarget->class == 0) {
        strcpy(class_string, "System");
    } else {
        strcpy(class_string, "User");
    }

    // Convert exec_state to string for display
    char exestate_string[20] = {0};
    if (showtarget->exec_state == READY) {
        strcpy(exestate_string, "READY");
    } else {
        strcpy(exestate_string, "BLOCKED");
    }

    // Convert priority to string
    char priority_string[20] = {0};
    sprintf(priority_string, "%d", showtarget->priority);

    // Convert disp_state to string for display
    char suspension_string[20] = {0};
    if (showtarget->disp_state == NOT_SUSPENDED) {
        strcpy(suspension_string, "NOT_SUSPENDED");
    } else {
        strcpy(suspension_string, "SUSPENDED");
    }

    char output[500];
    sprintf(output, "Name: %s\nClass: %s\nState: %s\nSuspended Status: %s\nPriority: %s\n",
            name, class_string, exestate_string, suspension_string, priority_string);
    sys_req(WRITE, COM1, output, strlen(output));
}

void display_pcb(struct pcb *target) {
    if(!target) return;

    char class_string[20] = {0};
    char exestate_string[20] = {0};
    char priority_string[20] = {0};
    char suspension_string[20] = {0};

    // Convert class to string for display
    strcpy(class_string, target->class == 0 ? "System" : "User");

    // Convert exec_state to string for display
    strcpy(exestate_string, target->exec_state == READY ? "READY" : "BLOCKED");

    // Convert priority to string
    sprintf(priority_string, "%d", target->priority);

    // Convert disp_state to string for display
    strcpy(suspension_string, target->disp_state == NOT_SUSPENDED ? "NOT_SUSPENDED" : "SUSPENDED");

    char output[500];
    sprintf(output, "Name: %s\nClass: %s\nState: %s\nSuspended Status: %s\nPriority: %s\n",
            target->name, class_string, exestate_string, suspension_string, priority_string);
    sys_req(WRITE, COM1, output, strlen(output));
}

// Shows all PCBs in the Ready state
void show_ready(void) {
    char header[] = "\n====== READY PROCESSES ======\n";
    sys_req(WRITE, COM1, header, strlen(header));

    struct pcb *current = ReadyQueue;

    if (!current) {
        char msg[] = "No processes in READY state.\n";
        sys_req(WRITE, COM1, msg, strlen(msg));
    }

    while (current) {
        display_pcb(current);
        current = current->next;
    }

    char footer[] = "=============================\n\n";
    sys_req(WRITE, COM1, footer, strlen(footer));
}

// Shows all PCBs in the Blocked state
void show_blocked(void) {
    char header[] = "\n====== BLOCKED PROCESSES ======\n";
    sys_req(WRITE, COM1, header, strlen(header));

    struct pcb *current = BlockedQueue;

    if (!current) {
        char msg[] = "No processes in BLOCKED state.\n";
        sys_req(WRITE, COM1, msg, strlen(msg));
    }

    while (current) {
        display_pcb(current);
        current = current->next;
    }

    char footer[] = "==============================\n\n";
    sys_req(WRITE, COM1, footer, strlen(footer));
}

// Shows all PCBs irrespective of their state
void show_all(void) {
    // Display Ready processes
    show_ready();

    // Display Blocked processes
    show_blocked();
}



