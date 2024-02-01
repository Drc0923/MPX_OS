bits 32
global sys_call_isr

;;; System call interrupt handler for Module R3.

extern sys_call         ; Extern directive informs the assembler about the 'sys_call' label which is defined in another file

sys_call_isr:
    ; Push general-purpose and segment registers
    push esp             ; Save stack pointer
    push ebp             ; Save base pointer
    push edi             ; Save destination index
    push esi             ; Save source index
    push edx             ; Save data register
    push ecx             ; Save count register
    push ebx             ; Save base register
    push eax             ; Save accumulator register
    push ss              ; Save stack segment register
    push gs              ; Save additional segment register
    push fs              ; Save additional segment register
    push es              ; Save extra segment register
    push ds              ; Save data segment register

    ; Push ESP onto the stack
    push esp             ; Push the stack pointer onto the stack (the value before pushing general-purpose registers)

    ; Call sys_call C function
    call sys_call        ; Call the sys_call function (a C function). This is where the system call handling happens.

    ; Adjust ESP based on returned value in EAX
    mov esp, eax         ; Restore the stack pointer. The sys_call function returns the new stack pointer in EAX.

    ; Pop segment and general-purpose registers
    pop ds               ; Restore data segment register
    pop es               ; Restore extra segment register
    pop fs               ; Restore additional segment register
    pop gs               ; Restore additional segment register
    pop ss               ; Restore stack segment register
    pop eax              ; Restore accumulator register
    pop ebx              ; Restore base register
    pop ecx              ; Restore count register
    pop edx              ; Restore data register
    pop esi              ; Restore source index
    pop edi              ; Restore destination index
    pop ebp              ; Restore base pointer
    add esp,4            ; Correct stack pointer for the earlier push esp instruction

    ; Return from ISR
    iret                 ; Interrupt return - this restores the CPU flags, IP, CS, and returns to user mode