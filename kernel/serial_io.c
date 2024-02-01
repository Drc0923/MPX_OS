#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "serial_io.h"
#include <mpx/io.h>
#include <mpx/interrupts.h>
#include <mpx/serial.h>
#include "mpx/device.h"
#include <memory.h>

// Constants
#ifndef IER_THRE
#define IER_THRE 0x02      // Interrupt Enable Register: Transmitter Holding Register Empty
#endif

#ifndef PIC1
#define PIC1 0x20          // Programmable Interrupt Controller 1
#endif

#define READ 3             // Read operation
#define WRITE 2            // Write operation
#define IDLE 1             // Idle operation
#define EXIT 0             // Exit operation

#define RING_BUFFER_SIZE 256  // Size of the ring buffer

#define MAX_DEVICES 4         // Maximum number of devices


// Array of Device Control Blocks
DeviceControlBlock devices[MAX_DEVICES];

// Find an existing DCB for a given device
DeviceControlBlock* get_device(device dev) {
    for (int i = 0; i < MAX_DEVICES; ++i) {
        if (devices[i].name == dev && devices[i].status != PORT_CLOSED) {
            return &devices[i];
        }
    }
    return NULL; // DCB not found for the device
}

// Function to find or create a DCB for a given device
//static DeviceControlBlock* get_device(device dev) {
//    for (int i = 0; i < MAX_DEVICES; ++i) {
//        if (devices[i].name == dev) {
//            return &devices[i];
//        }
//    }
//
//    // If not found, initialize a new DCB if available
//    for (int i = 0; i < MAX_DEVICES; ++i) {
//        if (devices[i].status == PORT_CLOSED) {
//            devices[i].name = dev;
//            devices[i].status = PORT_CLOSED; // Initially closed
//            devices[i].event = 0;
//            devices[i].operation = IDLE;
//            devices[i].buf_size= 150;
//            devices[i].buf_index = 0;
//            devices[i].start = 0;
//            devices[i].end = 149;
//            devices[i].ring_buf = (char*)sys_alloc_mem(RING_BUFFER_SIZE);
//
//            // Initialize other fields as necessary
//            return &devices[i];
//        }
//    }
//
//    return NULL; // No available DCB or not found
//}


// Open and initialize a serial port
//wrong implementation, old using old dcb struct, need guidance
int serial_open(device dev, int baud_rate) {
    // Check if the port is already open by checking the status.
    serial_init(dev);

    DeviceControlBlock* dcb = get_device(dev);

    if (!dcb) {
        // If DCB doesn't exist, find a closed slot and initialize it
        for (int i = 0; i < MAX_DEVICES; ++i) {
            if (devices[i].status == PORT_CLOSED) {
                dcb = &devices[i];
                dcb->name = dev;
                dcb->status = PORT_OPEN;
                dcb->event = 0;
                dcb->operation = IDLE;
                dcb->buf_size = 150;
                dcb->buf_index = 0;
                dcb->start = 0;
                dcb->end = 149;
                dcb->ring_buf = (char *) sys_alloc_mem(RING_BUFFER_SIZE);
                break;
            }
        }
    }

    if (baud_rate <= 0) {
        return -102; // Invalid baud rate
    }

    int div = 115200/(long)baud_rate;
    int msb = (div >> 8) & 0xFF;
    int lsb = div & 0xFF;

    outb(dev + LCR, 0x80);
    outb(dev + DLL, lsb);
    outb(dev + DLM, msb);
    outb(dev + LCR, 0x03);

    cli();
    int mask = inb(0x21);
    mask &= ~(1 << 4);
   // mask |= (0 << 7);
    outb (0x21, mask);
    //sti();

    outb(dev + MCR, 0x08);
    outb(dev + IER, 0x01);
    outb(dev + IER, 0x03);

    return 0;
}




// Close a serial port
int serial_close(device dev) {
    DeviceControlBlock* current_device = get_device(dev); // Retrieve DCB for the given device

    if(current_device->name == dev && current_device->status == PORT_CLOSED) {
        return -201; // Return error if the device is already closed
    }

    current_device->status = PORT_CLOSED; // Set device status to closed
    sys_free_mem(current_device->ring_buf); // Free allocated memory for the ring buffer

    cli(); // Disable interrupts
    int mask = inb(0x21); // Read interrupt mask
    outb(0x21, mask); // Restore the interrupt mask
    sti(); // Enable interrupts

    outb(dev + IER, 0x00); // Disable interrupts for the device
    outb(dev + MSR, 0x00); // Reset Modem Status Register

    return 0; // Return success
}

// Read from a serial port
int serial_read(device dev, char* buf, size_t len) {
    DeviceControlBlock* current_device = get_device(dev); // Retrieve DCB for the given device

    // Validate device and buffer
    if (current_device == NULL || current_device->name != dev || current_device->status != PORT_OPEN) {
        return -300; // Return error if device not open or mismatch
    }
    if (buf == NULL || len == 0) {
        return -302; // Return error for invalid buffer or length
    }

    // Check if device is busy with another operation
    if (current_device->operation != IDLE) {
        return -303; // Return error if device is busy
    }

    current_device->operation = READ; // Set operation to READ

    // Read from the ring buffer
    size_t bytesRead = 0;
    while (bytesRead < len && current_device->buf_index < current_device->buf_size) {
        buf[bytesRead++] = current_device->ring_buf[current_device->buf_index++];
        // Check for end of data or newline
        if (current_device->ring_buf[current_device->buf_index] == '\0' || current_device->ring_buf[current_device->buf_index] == '\n') {
            break;
        }
    }

    // Check if additional data needs to be read
    if (bytesRead < len) {
        // Setup IOCB for ISR to continue reading
        current_device->iocb.buffer = buf + bytesRead;
        current_device->iocb.length = len - bytesRead;
        current_device->iocb.transferred = bytesRead;
    } else {
        current_device->operation = IDLE; // Set operation back to IDLE
        current_device->event = 1; // Set event flag to indicate completion
    }

    return bytesRead; // Return number of bytes read
}

// Write to a serial port
int serial_write(device dev, char *buf, size_t len) {
    DeviceControlBlock* current_device = get_device(dev); // Retrieve DCB for the given device

    // Validate device and buffer
    if (current_device == NULL || current_device->status == PORT_CLOSED || current_device->name != dev) {
        return -401; // Return error if device is closed or invalid
    }
    if (buf == NULL) {
        return -402; // Return error for null buffer
    }
    if (len > 100 || len < 0) {
        return -403; // Return error for invalid length
    }

    // Setup device for writing
    current_device->event = 0;
    current_device->ring_buf = buf;
    current_device->buf_size = len;
    current_device->start = 0;
    current_device->end = len - 1;
    current_device->operation = WRITE; // Set operation to WRITE

    sti(); // Enable interrupts

    // Write the first character to start transmission
    char first_character = buf[0];
    outb(dev + THR, first_character);

    // Update the interrupt register to enable THRE interrupt
    unsigned char prev = inb(dev + IER);
    outb(dev + IER, prev | 0x02); // Enable THRE interrupt

    return 0; // Return success
}

// Serial input interrupt handler
void serial_input_interrupt(struct dcb *dcb) {
    device dev = dcb->name;
    char input_character = inb(dev + IIR); // Read the input character

    if (dcb->operation != READ) {
        // Handle non-read operations
        if (dcb->buf_index <= dcb->end) {
            // Store character in ring buffer if within bounds
            dcb->ring_buf[dcb->buf_index] = input_character;
        }
    } else {
        // Handle read operation
        dcb->iocb.buffer = &input_character;
        if (input_character == '\r') {
            return; // Return if carriage return
        } else {
            // Complete the operation
            dcb->operation = IDLE;
            dcb->event = 1;
        }
    }
}

// Serial output interrupt handler
void serial_output_interrupt(struct dcb *dcb) {
    if (dcb->operation != WRITE) {
        return; // Return if not a write operation
    } else {
        // Complete the write operation
        dcb->operation = IDLE;
        dcb->event = 1;
        inb(dcb->name + IER); // Read the IER to reset
    }
}

// General serial interrupt handler
void serial_interrupt(void) {
    cli(); // Disable interrupts

    // Iterate through each active device
    for (int i = 0; i < MAX_DEVICES; ++i) {
        if (devices[i].status == PORT_OPEN) {
            device dev = devices[i].name;
            unsigned char interrupt_type = inb(dev + IIR); // Read the interrupt type

            // Check and handle interrupt type
            if (!(interrupt_type & 0x01)) { // If interrupt is pending
                if (interrupt_type & 0x04) {
                    serial_output_interrupt(&devices[i]); // THR empty
                } else if (interrupt_type & 0x02) {
                    serial_input_interrupt(&devices[i]); // Received data available
                }
            }
        }
    }

    outb(PIC1, 0x20); // Send EOI to the PIC
    sti(); // Enable interrupts
}





