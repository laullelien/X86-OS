
# Vue d'ensemble du projet:

Notre projet implémente la phase 7.
Le mode user/kernel demandé dans la phase 5 est implémenté, tous les tests passent, le clavier fonctionne et un interpreteur de commandes basique est disponible dans 'test_app'. (programme lancé par défaut par l'éxécutable)

Concernant le clavier, nous avons fait le choix de ne prendre en compte aucun nouvel appui sur une touche si le buffer est plein (même pas Entrée). Ainsi si on a écrit trop de caractères, il faut en effacer un pour pouvoir retourner à la ligne. 

`make run` lance le shell.
Les commande disponibles dans le shell sont:
```
exit
echo on/off
start <process_name>
kill <pid>
waitpid <pid>
chprio <prio>
autotest
ps
```

La commande `autotest` lance tous les tests à la suite. 

# Points à améliorer

Notre système d'allocation de mémoire utilisateur est assez basique et devrait être amélioré.

Si un process se termine ou est tué sans relacher un shm, il restera ouvert pour toujours. Un effet de bord est que si on lance deux fois de suite dans le shell le test 7 par exemple, au deuxième essai, le process n'arrive pas à créer un nouveau shm puisque le process précédent ne l'a pas relaché avant de se terminer.
