ATTENTION : Les commandes de compilation et de lancement doivent être effectués impérativement depuis le répertoire QuietMessengers (premier répertoire de l'application).

------------------------------------------------
                  COMPILATION
------------------------------------------------

Avant de lancer l'application, vous devez tout d'abord effectuer la commandes suivante afin de compiler :
gcc -o mainc Client/Sources/*.c -pthread


------------------------------------------------
                  LANCEMENT
------------------------------------------------

Pour lancer chaque nouveau client, vous devez lancer le programme client dans un nouveau terminal avec la commande :
./mainc [IPadress] 8000

Pour trouver votre adresse IP faites la commande ifconfig
(Exemple d'adresse IP : 162.45.32.81)


------------------------------------------------
                  CONNEXION
------------------------------------------------

Lors de la première connexion, on vous demande d'entrer un pseudo. Vous garderez ce dernier tout au long de votre utilisation. Ecrivez votre pseudo puis appuyez sur la touche Entrée.
Il vous est également demander de définir un mot de passe. De la même façon entrez un mot de passe et appuyez sur Entrée.

Aux prochaines connexions, vous n 'aurez plus qu'à indiquer votre pseudo et votre mot de passe pour vous connecter.

Une fois que cela est fait vous pouvez utiliser l'application.

----------------------------------------------------------------
                      UTILISATION
----------------------------------------------------------------

RAPPEL : Entrez la commande /man pour afficher 
les commandes que vous pouvez utiliser. 
Ces commandes sont également rappelées ci-dessous.


________________________________________________________________________
                         Commandes de communication                             
________________________________________________________________________

[message]                       Send a public message (to all)  
/mp [recipient] [message]       Send a private message
/all [message]                  Send a message to everyone in the server

_________________________________________________________________________
                         Commandes de salon
_________________________________________________________________________

/changePseudo                   Change your pseudo for a new one
/file                           Upload a file
/filerequest                    Download a file
/jroom                          Join a room
/croom                          Create a room
/rroom                          Rename a room
/droom                          Delete a room
/rooms                          Display all rooms
/eroom                          Exit a room
/whoishere                      Display the connected clients
/end                            End the conversion (disconnect)

_________________________________________________________________________
              Commandes de gestion des salons et du serveur
_________________________________________________________________________

/promoteToAdmin                 Promote a client to administrator
/promoteToMod                   Promote a client to moderator
/demote                         Demote a client
/kick                           Kick a client from the server
/kickFromRoom                   Kick a client from a room
/ban                            Ban a client from the server
/unban                          Unban a client from the server
/move                           Move a client into a room
/setAdminServ                   Promote a client to server administrator

_________________________________________________________________________
                    Commandes de gestion du client
_________________________________________________________________________

/changePassword                 Change your password for a new one
/changePseudo                   Change your pseudo for a new one
/deleteAccount                  Delete your account 

_________________________________________________________________________










----------------------------------------------------------------
        Merci d'avoir été attentif et bonne utilisation !
----------------------------------------------------------------