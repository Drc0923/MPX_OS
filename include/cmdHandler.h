//
// Created by Dylan Caldwell on 8/18/23.
//

#ifndef CMDHANDLER_H
#define CMDHANDLER_H

#include <stddef.h>  // For size_t



/**
 * Command handler function that processes user commands from the serial interface.
 * This function continually reads commands from the serial port, processes them,
 * and then responds back through the same serial port.
 */
void comhand(void);


#endif //CMDHANDLER_H
