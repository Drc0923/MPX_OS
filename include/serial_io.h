#ifndef FIJI_SERIAL_IO_H
#define FIJI_SERIAL_IO_H

#include <stddef.h> // Include standard definitions
#include "pcb.h"    // Include necessary custom headers
#include "mpx/device.h"

// Constants representing the status of a serial port
#define PORT_CLOSED 0  // Closed or inactive serial port
#define PORT_OPEN   1  // Open or active serial port

// IOCB structure for I/O control block
typedef struct {
    struct pcb* process;     // The process performing the I/O
    char* buffer;            // Buffer for data transfer
    size_t length;           // Length of the buffer
    size_t transferred;      // Bytes transferred so far
    int operation;           // Operation type: READ or WRITE
} IOCB;

// DeviceControlBlock structure for managing device I/O
typedef struct dcb {
    device name;          // Name of the device control block
    int status;              // Status (0 for Closed, 1 for Open)
    int event;               // Event flag
    int start;               // Starting index of buffer
    int end;                 // Ending index of buffer
    int operation;           // Current operation (IDLE, READ, WRITE)
    int buf_size;            // Size of the buffer
    int buf_index;           // Current index in the buffer
    char* ring_buf;          // Ring buffer for data
    IOCB iocb;               // I/O control block instance
} DeviceControlBlock;

// Function prototypes for serial port operations
int serial_open(device dev, int baud_rate);
int serial_close(device dev);
int serial_read(device dev, char *buf, size_t len);
int serial_write(device dev, char *buf, size_t len);
void serial_interrupt(void);
void serial_input_interrupt(struct dcb *dcb);
void serial_output_interrupt(struct dcb *dcb);
int serial_device_status(device dev);
struct dcb* get_device(device dev);

#endif // FIJI_SERIAL_IO_H
