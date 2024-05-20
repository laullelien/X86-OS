    .globl cr3_sw
# Structure de la pile en entree :
#   %esp + 4 : adresse du page directory
# On stocke la page directory dans cr3 est dans la tss
cr3_sw:
    xor   %eax, %eax
    movl 4(%esp), %eax
    
    movl %eax, %cr3 
    movl %eax, tss+28
    
    ret