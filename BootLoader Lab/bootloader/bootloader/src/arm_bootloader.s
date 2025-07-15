.global _start



@trap table vector (instruction executed on trap)
_start:
_trap_table:
    @ Each trap instruction loads the address of its trap handler.
    ldr pc, $handlers + 0	            @ Reset
    ldr pc, $handlers + 4	            @ Supervisor Call
    ldr pc, $handlers + 8	            @ Supervisor Call
    ldr pc, $handlers + 12	            @ Prefetch Abort
    ldr pc, $handlers + 16	            @ Data Abort
    nop				                    @ Reserved; no-op.
    ldr pc, $handlers + 20	            @ Regular Interrupt
    ldr pc, $handlers + 24	            @ Fast Interrupt

@pointers to trap handler routines
handlers:
    .int reset, reset, reset, reset, reset, reset, reset

@reset routine
reset:
    bic r0, r0, #0xff	             @ Clear status register bits 0-7
    orr r0, r0, #0xd3	             @ Mask with 0x11010011
    msr cpsr_fc, r0	                 @ Load value into status register
    b _uart_start		             @ Jump to looping routine

_uart_start:
    ldr r4, =0xA3F00000              @ Point to Start of massage
_uart_tx:	    
    ldrb r5, [r4]                    @ Dereference Value (Character in msg) (al = the lower byte of bx, i.e the char)
    cmp r5, #0                       @ Checks for Null Terminator '\0'
    beq done                         @ If '\0' go to the end of assembly

    ldr  r6, =0x40100000
    strb r5, [r6]                   @ Fill the Tx regester to Send Character Out 
_uart_tx_poll:
    ldr r6, =0x40100014             @ Load in the LSR's Address's byte
    tst r6, #0x20                   @ Check if THR bit is empty
    bne  _uart_tx_poll              @ If not keep polling
    add r4, r4, #1                  @ els increment to the next character
    b _uart_tx                      @ Go back to UART_TX
done:
    b done

