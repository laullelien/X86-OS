    .text
# cette directive sert a rendre l'etiquette publique
    .globl exit_asm
# debut du exit_asm 
exit_asm:
    # bouger la valeur de retour dans la pile
    pushl %eax
    # appeler exit
    call exit
