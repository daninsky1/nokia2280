.syntax unified     /* pseudo-ops are GAS keywords, directives */
.arch armv4t
.arm


.global _start
.type _start, %function
_start:
    mov     r0, #5
    mov     r1, #7
    add     r2, r0, r1      @ r2 = 12
    adds    r3, r2, #1      @ r3 = 13, updates CPSR flags

.LHalt:
    b .LHalt

.size _start, . - _start

