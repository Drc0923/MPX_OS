global serial_interrupt_entry

extern serial_interrupt  ; C function

section .text
serial_interrupt_entry:
    pusha                      ; Push all general-purpose registers
    call serial_interrupt  ; Call the C function to handle the interrupt
    popa                       ; Pop all general-purpose registers
    iret                       ; Return from interrupt
