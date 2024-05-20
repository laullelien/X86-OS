.globl start
.globl getpid
.globl getprio
.globl chprio
.globl kill
.globl waitpid
.globl exit
.globl cons_write
.globl cons_read
.globl cons_echo
.globl scount
.globl screate
.globl sdelete
.globl signal
.globl signaln
.globl sreset
.globl try_wait
.globl wait
.globl pcount
.globl pcreate
.globl pdelete
.globl preceive
.globl preset
.globl psend
.globl clock_settings
.globl current_clock
.globl wait_clock
.globl sys_info
.globl shm_create
.globl shm_acquire
.globl shm_release

syscall:
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %esi
    pushl %edi
    pushl %ebp

    movl (6*4+4)(%esp), %ebx
    movl (6*4+8)(%esp), %ecx
    movl (6*4+12)(%esp), %edx
    movl (6*4+16)(%esp), %esi
    movl (6*4+20)(%esp), %edi
    movl (6*4+24)(%esp), %ebp

    int $49

    # restauration des registres importants
    popl %ebp
    popl %edi
    popl %esi
    popl %edx
    popl %ecx
    popl %ebx

    ret


start:
    movl $0x0, %eax
    jmp syscall

getpid:
    movl $0x1, %eax
    jmp syscall

getprio:
    movl $0x2, %eax
    jmp syscall

chprio:
    movl $0x3, %eax
    jmp syscall

kill:
    movl $0x4, %eax
    jmp syscall

waitpid:
    movl $0x5, %eax
    jmp syscall

exit:
    movl $0x6, %eax
    jmp syscall

cons_write:
    movl $0x7, %eax
    jmp syscall

cons_read:
    movl $0x8, %eax
    jmp syscall

cons_echo:
    movl $0x9, %eax
    jmp syscall

scount:
    movl $0xa, %eax
    jmp syscall

screate:
    movl $0xb, %eax
    jmp syscall

sdelete:
    movl $0xc, %eax
    jmp syscall

signal:
    movl $0xd, %eax
    jmp syscall

signaln:
    movl $0xe, %eax
    jmp syscall

sreset:
    movl $0xf, %eax
    jmp syscall

try_wait:
    movl $0x10, %eax
    jmp syscall

wait:
    movl $0x11, %eax
    jmp syscall

pcount:
    movl $0x12, %eax
    jmp syscall

pcreate:
    movl $0x13, %eax
    jmp syscall

pdelete:
    movl $0x14, %eax
    jmp syscall

preceive:
    movl $0x15, %eax
    jmp syscall

preset:
    movl $0x16, %eax
    jmp syscall

psend:
    movl $0x17, %eax
    jmp syscall

clock_settings:
    movl $0x18, %eax
    jmp syscall

current_clock:
    movl $0x19, %eax
    jmp syscall

wait_clock:
    movl $0x1a, %eax
    jmp syscall

sys_info:
    movl $0x1b, %eax
    jmp syscall

shm_create:
    movl $0x1c, %eax
    jmp syscall

shm_acquire:
    movl $0x1d, %eax
    jmp syscall

shm_release:
    movl $0x1e, %eax
    jmp syscall
