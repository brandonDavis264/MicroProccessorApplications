# The bootloader will read a message from a memory mapped block device at a
# designated memory address with a null terminator (\0) 

# General Register Usages
    # %AX = Stores a funtion return value (Accumulator [1+2 = Ax])
    # %BX = Base pointer to the data section
    # %CX = Counter for string and loop operations
    # %DX = I/O pointer
    # %SI = source index pointer for string Operations
    # %DI = destination index pointer for string Operations

.code16
.global _start

#Constants
.set UART_TXB_ADDR,     0x3F8
.set UART_LSR_ADDR,     0x3FE
.set UART_LSR_TX_BIT,   0x20
.set TTY_MODE,          0x0E
.set TTY_INT,           0x10
.set MEM_MAP_ADDR,      0x0500

# reset routine
.text
_start:
    jmp _uart_start              # Jump to looping routine

_uart_start:
    mov $0x500, %bx              # Point to Start of _msg
_uart_tx:
    mov (%bx), %al 		        # Dereference Value (Character in _msg) (al = the lower byte of bx, i.e the char)
    cmp $0, %al                 # Checks for Null Terminator '\0'
    je _end                     # If '\0' go to the end of assembly
    mov $UART_TXB_ADDR, %dX     # dx -> $UART_TXB_ADDR
    out %al, %dx                # Fill the Tx regester to Send Character Out (Send al to address specified be dx)
_uart_tx_poll:
    mov $UART_LSR_ADDR, %dx
    in   %dx, %al               # Load in the LSR's Address's byte
    and  UART_LSR_TX_BIT, %al   # Check if THR bit is empty
    jz  _uart_tx_poll           # If not keep polling
    inc %bx                     # els increment to the next character
    jmp _uart_tx                # Go back to UART_TX
_end:
    hlt

_msg:
    .asciz "\nHello World"

. = _start + 510		    # Skip forward to byte 510
.word 0xaa55			    # magic number that tells the BIOS that this device is bootable
