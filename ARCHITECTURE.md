**Slash, small laudable shell :**

# Introduction et fonctionnalités :

Ce projet, `Slash` consiste en la programmation d’un interpréteur de commandes interactif.

Slash permet :
-L’exécution des commandes `internes` : cd , pwd et exit.
-L’interprétation de l’étoile simple `*`.
-L’interprétation de l’étoile double `**`.
-Les différentes redirections qui peuvent être multiples dans une meme ligne de commande.
-L’utilisation des tubes ou `pipelines`.
-L'exécution de toutes les commandes externes d’un shell classique.


[Notre projet est composé principalement de 4 classes: cd.c, pwd.c, exit.c, et slash.c.]

# Structures de données utilisées :

Notre choix était d’utiliser les pointeurs de pointeurs comme structure de données principale dans notre implementation. Et cela à travers les allocations de mémoires à l’aides de malloc() et calloc().


# Architecture logicielle :

Notre programme passe par les étapes ci dessous: 
-La ligne de commande saisie est récupérée dans un pointeur à l’aide de la bibliothèque `readline`.
-Ce pointeur est ensuite transformé en pointeur de pointeurs grace à la fonction `splitCommand()` qui utilise l'espace comme un délimiteur.
-Par la suite, la ligne de commande est passé à la fonction `interpretCommand()` pour faire l’expansion des jokers `* et **` s'ils y’en a.
-Le traitement des tubes ou les pipelines et des redirections, s'ils existent, grace à la fonction `treatPipes()`.
-L'exécution de la commande.
-A la fin, On affiche le résultat.


# Algorithmes Principaux :

## L'expansion de l’étoile simple et double :
- Cette étape passe par l'appel à la fonction `treatJoker()` qui retourne un pointeur de pointeurs avec les interprétations possibles de chaque joker.
-La fonction `Search()` à un role principal dans cette partie. Elle prend en argument un pointeur qui représente ce qu'on doit chercher et qui peut etre soit `*` soit `*.extension` , et un autre pointeur qui est le path du repértoire dans lequel on doit effectuer le recherche.
-Selon chaque cas, la fonction `treatJoker()` fait appel à `search()` avec les bons arguments.
-Par exemple:
    -Si on est en train de faire l'expansion de `**/`: on commence par allouer un pointeur de pointeurs,
    puis on appelle `search(".","*")` puis `search(".","*/*")` puis `search(".","*/.../*")` un certain nombre de fois (qui est la varibale `maxDepth` déclaré au début du programme et qui représente la pronfondeur maximale de l'arborescence ou on doit chercher) et à chaque on ajoute le résultat à notre pointeurs de pointeurs. A la fin, on parcourt ce dernier et on vérifie quel pointeur répond aux conditions qu'on cherche (dans ce cas, pas de lien symbolique, path valide).
    -Si on est en train de faire l'expansion de " target=`*.extension` " , on retourne directement `search(".",target)`.
    -Si on est en train de faire l'expansion de " target=`directory/*.extension` ", on retourne `search(directory,*.extension)`.
    -etc...


## Traitement des redirections et pipelines : 
-Cette étape commence suite à l'appel à la fonction `treatPipes()`.
-La fonction `treatPipes()` verifie au début si la ligne de commande représentée par un pointeurs de pointeurs contient des tubes:
        - Si oui, elle fait appel à la fonction `treatmultiplesPipes()` qui traite les tubes ou les pipelines en vérifiant s'il y a des redirections à faire au meme temps. 
        -sinon, on verifie directement si la ligne de commande contient des redirections en appelant la fonction `treat()`:
            -si oui, on fait les duplications nécessaire puis on execute la commande
            -sinon, on execute la commande directement.

# Difficultés rencontrées :

De façon générale , quelques difficultés ont été rencontrées dont les plus grandes étaient l'expansion des jokers, les pipelines et la gestion du temps en fonction du travail qui reste à faire.
Mais, on a réussi a surmonté toutes ces difficultés et à finir le projet avec toute ces parties et tout ce qui était demandé.  