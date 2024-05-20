    .text

    .globl traitant_IT_32
    .globl traitant_IT_14
    .globl traitant_IT_1

# Traitant it32: clock
traitant_IT_32:
# sauvegarde des registres importants
    pushl %eax
    pushl %ebx
    pushl %edx
    pushl %ecx
    
# appel a la fonction C realisant le traitant
    call tic_PIT
# restauration des registres importants

    popl %ecx
    popl %edx
    popl %ebx
    popl %eax

# fin du traitant
    iret



# traitant it 33: clavier
traitant_IT_1:
# sauvegarde des registres importants
    pushl %eax
    pushl %ebx
    pushl %edx
    pushl %ecx
    
    call handle_scancode
# restauration des registres importants

    popl %ecx
    popl %edx
    popl %ebx
    popl %eax

# fin du traitant
    iret




# traitant it 14: pagefault
traitant_IT_14:
# sauvegarde des registres importants
    pushl %eax
    pushl %ebx
    pushl %edx
    pushl %ecx

    call handle_user_pagefault

# restauration des registres importants
    popl %ecx
    popl %edx
    popl %ebx
    popl %eax

# fin du traitant
    iret