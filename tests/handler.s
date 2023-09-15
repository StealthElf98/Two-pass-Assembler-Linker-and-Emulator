.extern isr_timer, isr_terminal, isr_software

.global handler
.section my_handler
handler:
    push %r1
    push %r2
    csrrd %cause, %r1
    ld $2, %r2
    beq %r1, %r2, handle_timer
    ld $3, %r2
    beq %r1, %r2, handle_terminal
    ld $4, %r2
    beq %r1, %r2, handle_software
finish:
    pop %r2
    pop %r1
    iret
# obrada prekida od tajmera
handle_timer:
    call isr_timer
    jmp finish
# obrada prekida od terminala
handle_terminal:
    call isr_terminal
    jmp finish
# obrada softverskog prekida
handle_software:
    call isr_software
    jmp finish
.section math
    push %r2
    ld [%sp + 0x08], %r1
    ld [%sp + 0x0C], %r2
fake_math:
    add %r2, %r1 # r1 used for the result
    pop %r2
    ret

.end
