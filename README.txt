ATTENTION : Les commandes de compilation et de lancement doivent être effectués depuis le répertoire QuietMessengers (premier répertoire de l'application).

------------------------------------------------
                  COMPILATION
------------------------------------------------

Avant de lancer l'application, vous devez tout d'abord effectuer les commandes suivantes afin de compiler :
gcc -o mainc Client/Sources/*.c -pthread
gcc -o mains Server/Sources/*.c -pthread

------------------------------------------------
                  LANCEMENT
------------------------------------------------

Pour lancer l'application, vous devez lancer le programme serveur :
./mains [port] [nbClientMax]

Pour lancer chaque nouveau client, vous devez lancer le programme client dans un nouveau terminal :
./mainc [IPadress] [port]

Pour trouver votre adresse IP faites la commande ifconfig
(Exemple d'adresse IP : 162.45.32.81)

Le port doit être le même pour le serveur et pour les clients.