    .text
    .globl traitant_IT_49


# traitant qui récupère les interruptions user
# eax contient le numéro de la fonction à appeler (dans l'ordre défini plus bas), les paramètres sont dans les autres registres %ebp, %edi, %esi, %edx, %ecx, %ebx
traitant_IT_49:

#sauvegarder eax temporairement
    pushl %eax
# mettre les registres segments à la bonne valeur
    movl $0x18, %eax
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %fs
    movl %eax, %gs

    popl %eax

# on place les paramètres sur la pile
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    
# on calcule "4*eax + function_table", dans eax. cette case mémoire contient l'adresse de la fonction à appeler.
    mov $function_table, %ebx
    shl $0x2, %eax
    add %ebx, %eax
# on récupère le contenu de cette case mémoire dans eax
    movl (%eax), %eax
# maintenant eax contient l'adresse où se trouve la fonction
    call *%eax

# on dépile ce qu'on a rajouté avant
    popl %ebx
    popl %ebx
    popl %ebx
    popl %ebx
    popl %ebx
    popl %ebx

    
# On remet les registres segments pour le user

    movl $0x4b, %ebx
    movl %ebx, %ds
    movl %ebx, %es
    movl %ebx, %fs
    movl %ebx, %gs

# fin du traitant
# acquitment de l'interruption
    mov %eax, %ebx
    mov $0x20, %al
    outb %al, $0x20
    mov %ebx, %eax
        
    iret

# définition les l'ordre des fonctions
function_table:
    .long start
    .long getpid
    .long getprio
    .long chprio
    .long kill
    .long waitpid
    .long exit
    .long cons_write
    .long cons_read
    .long cons_echo
    .long 0 #scount
    .long 0 #screate
    .long 0 #sdelete
    .long 0 #signal
    .long 0 #signaln
    .long 0 #sreset
    .long 0 #try_wait
    .long 0 #wait
    .long pcount
    .long pcreate
    .long pdelete
    .long preceive
    .long preset
    .long psend
    .long clock_settings
    .long current_clock
    .long wait_clock
    .long sys_info
    .long shm_create
    .long shm_acquire
    .long shm_release
    
    
