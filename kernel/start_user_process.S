    .globl start_user_process
# Structure de la pile en entree :
#   %esp + 4 : EIP
#   %esp + 8 : CS
#   %esp + 12 : EFLAGS
#   %esp + 16 : ESP
#   %esp + 20 : SS
# la structure en entrée est  déjà la bonne pour le iret, il suffit donc de mettre les registres segments à la bonne valeur
start_user_process:
#on s'assure que eax est nul
    xor   %eax, %eax

    movl 20(%esp), %eax
    
    movl %eax, %ds
    movl %eax, %es
    movl %eax, %fs
    movl %eax, %gs
    
    pop %eax; # On enleve la return address de la stack avant de retourner
    
    iret