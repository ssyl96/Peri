#Questions
##Question 1
* Q1: Le mot clé volatile indique au compilateur que la variable déclarée est susceptible d'être modifiée entre deux instructions. Cela permet d'éviter que le compilateur d'effectuer des optimisations sur cette variable.
* Q2: Dans `open()`, O_RDWR permet l'autorisation d'écriture et lecture et O_SYNC permet l'écriture synchrone entre le disque et la mémoire.
Dans la fonction `mmap()`, PROT_READ et PROT_WRITE permettent l'écriture et la lecture et MAP_SHARED autorise d'autres processus à utiliser la région de mémoire mappée.
* Q3: Lorsqu'on termine l'exécution, on doit libérer la zone mémoire qui a été mappée, `munmap()` suprrime la projection dans la zone de mémoire spécifiée par `mmap()`.
* Q4:  `LCD_FUNCTIONSET, LCD_FS_4BITMODE, LCD_FS_2LINE,LCD_FS_5x8DOTS` permettent d'indiquer à LCD qu'on initialise la taille des données, le nombre de lignes sur l'écran et la police.
* Q5: TODO

##Question 2



