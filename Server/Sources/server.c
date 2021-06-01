

#include "../Headers/server.h"

int createServerSocket(int port){

    /* Creating the socket */
    printf("Creation : ");
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(port);
    printf("ok\n");

    /* Naming the socket */
    printf("Naming : ");
    int bindR = bind(dS, (struct sockaddr*)&ad, sizeof(ad));
    if (bindR == -1){
        perror("error\n");
        exit(-1);
    }
    else{
        printf("ok\n");
    }

    /* Making it listen */
    printf("Listening : ");
    int listenR = listen(dS, 7);
    if (listenR == -1){
        perror("error\n");
        exit(-1);
    }
    else{
        printf("ok\n");
    }
    printf("\nSocket ready\n\n\n");
    return dS;
}


int firstIndexEmpty(struct dataClient* tab, int taille){
    int trouve = -1;
    int c = 0;
    while(trouve ==-1 && c<taille){
        if(tab[c].dS==-1){
            return c;
        }
        c = c + 1;
    }
    return trouve; 
}




int findIndexByDS(int dS){

    if(dS != -1){
        for(int i = 0; i<nbClientMax;i++){
            if(tabClient[i].dS == dS){
                return i;
            }
        }
    }
    return -1;
}


int loginUser(int indexClient,char* password){

    int doesPasswordMatch =0 ;
    char* message = "\nVeuillez entrer votre mot de passe : \n\n";
    char* passwordEntered = malloc(SIZEOFPSWD);
    getFeedBackFrom(indexClient,message,passwordEntered);

    /*isPasswordValid ?*/
    if(strcmp(passwordEntered,password)==0){
    
        message = "\nVous vous êtes connecté avec succès !\n\n";
        doesPasswordMatch = 1;
    }
    else{
    message = "\nMauvais mot de passe !\n";
    }
    sendMessageTo(tabClient[indexClient].dS,message);
    return doesPasswordMatch;
}


void registerUser(int indexClient){

    /*ask the client for a password*/
    char* message = "\nEntrez le mot de passe de votre choix : \n\n";
    char* password = malloc(SIZEOFPSWD);
    getFeedBackFrom(indexClient,message,password);

    /*register the client to the DATABASE*/
    registerClientToDB(indexClient,password);
    printf("le client à été enregistré avec succès\n");

    /*gives feedBack to the client*/
    message = "\nVotre compte a été créée avec succès !\n\n";
    sendMessageTo(tabClient[indexClient].dS,message);
}


void selectPseudo(int currentClientDS){

    int pseudoAffected = 0;
    int indexC = findIndexByDS(currentClientDS);

    while(!pseudoAffected){
        
        /*Wait for the client to enter a pseudo*/
        char * pseudo = (char *) malloc(SIZEOFPSEUDO);
        char* updtMsg = (char *) malloc(SIZEOFMESSAGE);
        receiveMessageFrom(currentClientDS,pseudo);

        /*Check if the client is banned*/
        if(isClientBlackListed(pseudo)){
                strcpy(updtMsg,"Le compte est suspendu\n");
        }
        else{
            /* If the pseudo is available, it is assigned*/
            if(isPseudoAvailable(pseudo)){

                /* Checking if the mutex is available and blocking it during the edit */
                pthread_mutex_lock(&mutex_tab);
                /*Update the pseudo in the Server*/
                tabClient[indexC].pseudo = malloc(SIZEOFPSEUDO);
                strcpy(tabClient[indexC].pseudo, pseudo);
                pseudoAffected = 1;
                strcpy(updtMsg,"ok");
            }
            else{
                strcpy(updtMsg,"Le compte est déjà en ligne\n");
                /* Sending the message that the pseudo entered already exists */
            }
        }
        sendMessageTo(currentClientDS,updtMsg);
        free(pseudo);
        pthread_mutex_unlock(&mutex_tab);
    }
    
}


void changePseudo(int index){

    char* message = "\nEntrez votre nouveau pseudo :\n\n";
    char* pseudo = (char*) malloc(SIZEOFPSEUDO);
    char* previousPseudo = (char*) malloc(SIZEOFPSEUDO);

    getFeedBackFrom(index,message,pseudo);

    if(isPseudoAvailable(pseudo)){
    
        
        strcpy(previousPseudo,tabClient[index].pseudo);
        free(tabClient[index].pseudo);
        tabClient[index].pseudo = (char*) malloc(SIZEOFPSEUDO);
        strcpy(tabClient[index].pseudo,pseudo);

        /*Update the DB*/
        char* password = findPasswordByPseudo(previousPseudo);
        removeClientFromDB(previousPseudo);
        registerClientToDB(index,password);

        /*Notify the users of the change of pseudo*/
        char* message = (char*) malloc(SIZEOFMESSAGE);
        strcat(message,"\nLe client ");
        strcat(message,previousPseudo);
        strcat(message," est devenu ");
        strcat(message,pseudo);
        strcat(message,"\n\n");

        sendMessageToAll(tabClient[index].dS,message);
        free(message);

    }
    else{

        char* message = "\nCe pseudo n'est malheureusement pas disponible ! \n\n";
        sendMessageTo(tabClient[index].dS,message);
    }

}

void changePassword(int index){

    char* m = malloc(SIZEOFMESSAGE);
    strcpy(m,"\nEntrez votre ancien mot de passe :\n\n");
    char* password = (char*) malloc(SIZEOFPSWD);
    char* previousPassword = (char*) malloc(SIZEOFPSWD);

    getFeedBackFrom(index,m,password);
    strcpy(previousPassword,findPasswordByPseudo(tabClient[index].pseudo));

    /*If the password matches with the one in the DB*/
    if(strcmp(password,previousPassword)==0){

        /*Update the client password in the DB*/
        char* newPassword = (char*) malloc(SIZEOFPSWD);
        strcpy(m,"\nEntrez votre nouveau mot de passe :\n\n");
        getFeedBackFrom(index,m,newPassword);
        removeClientFromDB(tabClient[index].pseudo);
        registerClientToDB(index,newPassword); 
        strcpy(m,"\nFélicitation votre mot de passe à été actualisé !\n\n");
    }
    else{
        strcpy(m,"\nLe mot de passe ne correspond pas à celui associé à votre compte\n\n");
    }
    sendMessageTo(tabClient[index].dS,m);
}


int findDSClientByPseudo(char* pseudo){

    int boolFound = 0;
    int c = 0;
    pthread_mutex_lock(&mutex_tab);

    /* Crossing the table tabClient */
    while(!boolFound && c<nbClient){

        /*If the pseudo if found return the dS that correspond*/
        if(strcmp(tabClient[c].pseudo,pseudo)==0){
            pthread_mutex_unlock(&mutex_tab);
            return tabClient[c].dS;
        }
        c=c+1;
    }
    pthread_mutex_unlock(&mutex_tab);
    printf("personne détécté avec ce pseudo\n");
    return -1;

}

char* findPasswordByPseudo(char* pseudo){

    FILE* fp = fopen("Server/Data/clientInfo.txt","r");

    if(fp == NULL){
        printf("probleme de fichiers lors de l'initialisation des rooms\n");
        exit(0);
    }

    char* separators = ",";
    char* currentLine = (char *) malloc(sizeof(char)*100);
    char* copyLine = (char *) malloc(sizeof(char)*100);
    int isDone = 0;
    char* strToken = malloc(sizeof(char)*50);

    /*Until it reach EOF*/
    while((fgets(currentLine, sizeof(char)*100, fp) != NULL)&& !isDone)
        {
                strcpy(copyLine,currentLine);
                strToken = strtok(copyLine,separators);

                /*Compares the current pseudo to the one in parameter*/
                if(strcmp(strToken,pseudo)==0){

                        strToken = strtok(NULL,separators);
                        isDone = 1;
                }
            
        }
    fclose(fp);

    return strToken;
}

int findIDRByIndex(int index){
    return tabClient[index].roomId;
}



/* Testing functions ======================================================== */


int isPseudoAvailable(char* pseudo){
    int isDispo = 1;
    int c = 0;

    /*Check if the pseudo is used in the Server*/
    while(isDispo && c<nbClientMax){

        if(strcmp(pseudo,tabClient[c].pseudo)==0){
            printf("The pseudo is : %s\n",pseudo);
            
            isDispo = 0;
        }
        c = c + 1;
    }
    if(isDispo){
        printf("This pseudo free to log-in with \n");
    }
    else{
       printf("This pseudo is already in use in the server\n"); 
    }
    return isDispo;
}


int isCommand(char *msg){

    int boolean = 0;
    
    /* Checking if the first charcter is '/' */
    if((char)msg[0]=='/'){
        boolean = 1;
        printf("C'est une commande\n");
    }
    else{
        printf("Ce n'est pas une commande !\n");
    }
    return boolean;
}


/* Setting functions ======================================================== */

char* extractCommand(char* msg){

    /*Extract the first argument of the line given in parameter*/
    char * copyMsg = (char *) malloc(SIZEOFMESSAGE);
    strcpy(copyMsg, msg);
    char * m = (char *) malloc(SIZEOFMESSAGE);
    m = strtok(copyMsg," ");

    return m;
}


char* extractAttributeN(int n, char* msg,char* separator){

    /* Recovering the attribute from the beginning of the message */
    char * copyMsg = (char *) malloc(SIZEOFMESSAGE);
    strcpy(copyMsg, msg);
    char * attribute = (char *) malloc(sizeof(char)*20);
    attribute = strtok(copyMsg,separator);
    int i = 0;
    while(i < n){
        attribute = strtok(NULL,separator);
        i++; 
    }
    printf("L'attribut extrait est : [%s]\n",attribute);
    return attribute;
}


char* extractPrivateMessage(char* msg){

    char* separator = " ";
    char* attribute = extractAttributeN(1,msg,separator);
    char * copyMsg2 = (char *) malloc(SIZEOFMESSAGE);

    strcpy(copyMsg2, msg);
    copyMsg2 += 4 +strlen(attribute) + 1;
    printf("strlen(attribute) : %d\n",(int)strlen(attribute));
    printf("Le message extrait est : [%s]\n",copyMsg2);
    return copyMsg2;
}


void messageTreatement(char* msg,int i){

    char* c = " : ";
    char* finalMsg = malloc(SIZEOFMESSAGE);

    strcat(finalMsg,tabClient[i].pseudo);
    strcat(finalMsg,c);
    strcat(finalMsg,msg);
    strcat(finalMsg,"\n");
    strcpy(msg,finalMsg);
    free(finalMsg);
}


int hasInsult(char* msgToTreat){

    char *saveptr1, *saveptr2;
    char* insultSeparators =",";
    char* wordsSeparators =" ;,-.";
    char* msg = malloc(SIZEOFMESSAGE);
    char* currentLine = (char*)malloc(sizeof(char)*400);

    strcpy(msg,msgToTreat);
    char* strTokenWords = strtok_r(msg,wordsSeparators,&saveptr1);

    /*Tant qu'il y'a des mots dans le message*/
    while(strTokenWords != NULL){

        /*on compare avec tous les mots*/
        FILE* fp = fopen("Server/Data/banwords.txt","r");

        /*Tant que le fichier n'est pas fini*/
        while(!feof(fp)){

            /*On lis la ligne*/
            if(fgets(currentLine, sizeof(char)*100, fp) == NULL){
                perror("erreur lecture de ligne\n");
                exit(0);
            }

            /*Tant qu'il y'a des mots à lire*/
            char* strTokenInsult = strtok_r(currentLine,insultSeparators,&saveptr2);

            while(strTokenInsult!=NULL){

                if(strcmp(strTokenInsult,strTokenWords)==0){
                    printf("correspondance !\n");
                    return 1;
                }
                strTokenInsult = strtok_r(NULL,insultSeparators,&saveptr2);
            }   
        }
        fclose(fp);
        strTokenWords = strtok_r(NULL,wordsSeparators,&saveptr1);
    }
    free(msg);
    return 0;
}


void hasInsultTreatement(int index){

    char* msgToSend = malloc(SIZEOFMESSAGE);
    int dSC = tabClient[index].dS;

    /*Check how many warnings have been sent*/
    switch (tabClient[index].nbOfWarnings)
    {
    case 0 :
        strcpy(msgToSend,"Ceci est votre premier avertissement, faites attention à votre langage !\n");
        break;
    case 1 :
        strcpy(msgToSend,"Ceci est votre second et dernier avertissement, vous serez banni au prochain écart de langage \n");
        break;
    case 2 :
        strcpy(msgToSend,"Vous avez pour la troisième fois utilisé des mots bannis votre compte est suspendu\n");
        sendMessageTo(dSC,msgToSend);
        /*Kick and ban the client*/
        blackListClient(tabClient[index].pseudo);
        reinitCellTab(dSC);
        pthread_cancel(tabThread[index]);
        
        break;
    
    default:
        break;
    }

    /*Increment the number of warnings sent*/
    tabClient[index].nbOfWarnings = tabClient[index].nbOfWarnings + 1;

    /*Send the appropriate message to the client */
    sendMessageTo(dSC,msgToSend);
    free(msgToSend);
}


void reinitCellTab(int dS){

    int c = 0;
    int trouve = 0;
    pthread_mutex_lock(&mutex_tab);

    while(c<nbClientMax && !trouve){

        if(tabClient[c].dS==dS && !trouve){

            tabClient[c].dS=-1;
            free(tabClient[c].pseudo);
            tabClient[c].nbOfWarnings = 0;
            pthread_mutex_unlock(&mutex_tab);
            printf("Tableau des clients reinitialisé\n");
            sem_post(&semaphoreNbClient);
            trouve = 1;
        }
        c = c + 1;
    }
}


int isClientRegistered(char* pseudo,char* password){

    FILE* fp = fopen("Server/Data/clientInfo.txt","r");

    if(fp == NULL){
        printf("probleme de fichiers lors de l'initialisation des rooms\n");
        exit(0);
    }

    char* strToken;
    char* separators = ",";
    char* currentLine = (char *) malloc(SIZEOFMESSAGE);
    int isClientInDB = 0;

    /*Until it reach EOF*/
    while((fgets(currentLine, sizeof(char)*100, fp) != NULL)&& !isClientInDB)
        {
            /*Compares the current pseudo to the one in parameter*/
            strToken = strToken = strtok(currentLine,separators);
            if(strcmp(strToken,pseudo)==0){
                isClientInDB = 1;
                /*strToken the password field*/
                strToken = strtok(NULL,separators);
                strcpy(password,strToken);
            }
        }
    fclose(fp);
    return isClientInDB; 
}


int isClientBlackListed(char* pseudo){

    FILE* fp = fopen("Server/Data/blackList.txt","r");

    if(fp == NULL){
        printf("probleme de fichiers lors de l'initialisation des rooms\n");
        exit(0);
    }

    char* strToken;
    char* separators = "\n";
    char* currentLine = (char *) malloc(SIZEOFMESSAGE);
    int isClientInBanned = 0;

    /*Until it reach EOF*/
    while((fgets(currentLine, sizeof(char)*100, fp) != NULL)&& !isClientInBanned)
        {
            /*Compares the current pseudo to the one in parameter*/
            strToken = strtok(currentLine,separators);
            if(strcmp(strToken,pseudo)==0){
                isClientInBanned = 1;
            }
        }

    printf("is Client banned : %d\n",isClientInBanned);
    fclose(fp);
    return isClientInBanned; 
}


void initClientFromDB(int index, char* pseudo ){

    FILE* fp = fopen("Server/Data/clientInfo.txt","r");

    if(fp == NULL){
        printf("probleme de fichiers lors de l'initialisation des rooms\n");
        exit(0);
    }

    char* strToken;
    char* separators = ",";
    char* currentLine = (char *) malloc(SIZEOFMESSAGE);
    int isClientInDB = 0;

    /*Until it reach EOF*/
    while((fgets(currentLine, sizeof(char)*100, fp) != NULL)&& !isClientInDB)
        {
            /*Compares the current pseudo to the one in parameter*/
            strToken = strtok(currentLine,separators);
            if(strcmp(pseudo,strToken)==0){

                strToken = strtok(NULL,separators);

                for(int i = 0 ;i<NBOFROOMS;i++){

                    strToken = strtok(NULL,separators);
                    tabClient[index].roleTab[i] = atoi(strToken);
                    printf("%d\n",index);
                }

                strToken = strtok(NULL,separators);
                tabClient[index].isAdmin = atoi(strToken);

            }    
            
        }
    fclose(fp);

}


void removeClientFromDB(char* pseudo){

    FILE* fp = fopen("Server/Data/clientInfo.txt","r");
    FILE * fp2 = fopen("Server/Data/clientInfo2.txt","w");

    if(fp == NULL){
        printf("probleme de fichiers lors de l'initialisation des rooms\n");
        exit(0);
    }
    int isClientFound = 0;

    char* separators = ",";
    char* currentLine = (char *) malloc(sizeof(char)*100);
    char* copyLine = (char *) malloc(sizeof(char)*100);
    char* strToken = malloc(sizeof(char)*50);

    /*Until it reach EOF*/
    while((fgets(currentLine, sizeof(char)*100, fp) != NULL))
        {
            
            if(!isClientFound){
                strcpy(copyLine,currentLine);
                strToken = strtok(copyLine,separators);
                /*Compares the current pseudo to the one in parameter*/
                /*Write the line only if it's not the client given in parameter*/
                if(strcmp(strToken,pseudo)!=0){
                    fprintf(fp2,"%s",currentLine);
                }
            }
            /*if the client has been found write the lines*/
            else{
                fprintf(fp2,"%s",currentLine);
            }
                        
        }

    fclose(fp);

    /*Delete the original file and makes the copy the true one*/
    int removeResult = remove("Server/Data/clientInfo.txt");
    int renameResult = rename("Server/Data/clientInfo2.txt","Server/Data/clientInfo.txt");

    if(removeResult !=0){
        perror("Le fichier n'a pu être supprimé ");
    }
    if(renameResult != 0){
        perror("Le fichier n'a pu être renommée ");
    }
    fclose(fp2);

}

void updateClientFromDB(char* pseudo){

    int dSC = findDSClientByPseudo(pseudo);
    int indexC = findIndexByDS(dSC);
    char* password = findPasswordByPseudo(pseudo);

    printf("\n le mot de passe extrait est %s\n",password);

    removeClientFromDB(pseudo);
    registerClientToDB(indexC,password); 
    printf("\n\nfin updateClient From FB \n\n");

}


void initRooms(){

    printf("Opening roomNames.txt\n");
    FILE* fp = fopen("Server/Data/roomNames.txt","r");

    if(fp == NULL){
        printf("probleme de fichiers lors de l'initialisation des rooms\n");
        exit(0);
    }
    
    char* currentName = (char *) malloc(SIZEOFROOMNAME);
    char* currentLine = (char *) malloc(sizeof(char)*100);
    char* currentIsDisplayed = (char *) malloc(sizeof(char));
    char* strToken;
    char* separators = ",";

    for(int i=0;i<NBOFROOMS;i++){
            
        roomsList[i].id = i;
        roomsList[i].name = malloc(SIZEOFROOMNAME);
        roomsList[i].description = malloc(SIZEOFDESCR);
        roomsList[i].maxMembers = nbClientMax;
        bzero(currentName,sizeof(char)*30);

        /*Extract a lign from the file*/
        if(fgets(currentLine, sizeof(char)*100, fp) == NULL)
        {
            printf("erreur lecture de ligne\n");
            exit(0);
        }

        /*We are cutting the line to extract the name and boolean of the room*/
        char* strToken = strtok(currentLine,separators);

        /*Load the name*/
        strcpy(roomsList[i].name,strToken);
        strToken = strtok(NULL,separators);

        /*Load isDisplayed parameter*/
        roomsList[i].isDisplayed = atoi(strToken);
        if(roomsList[i].isDisplayed != -1){
            nbOfRoomDisplayed = nbOfRoomDisplayed + 1;
        }

        /*Load the description*/
        strToken = strtok(NULL,separators);
        strcpy(roomsList[i].description,strToken);

        /*Sets all the potentiel member to their default value*/
        for(int j=0;j<nbClientMax;j++){
            roomsList[i].members[j] = -1;
        }

    }

    fclose(fp);

    printf("Salles de discussions initialisées avec succès !\n\n");

}


/*TODO*/
void moveClient(int clientIndex){

    
    /*Ask a pseudo from the client*/
    char* roomName = malloc(SIZEOFROOMNAME);
    char* pseudo = malloc(SIZEOFPSEUDO);
    char* msg = "\nEntrez le pseudo de la personne que vous souhaitez déplacer :\n\n";
    getFeedBackFrom(clientIndex,msg,pseudo);

    /*Ask a roomName from the client*/
    msg = "\nEntrez le nom de la room ou vous souhaitez le déplacer :\n\n";
    getFeedBackFrom(clientIndex,msg,roomName);
    

    if(isAdminServer(clientIndex)){
        
        int idR = findIdRoomByName(roomName);
        int dSCToMove = findDSClientByPseudo(pseudo);
        int indexToMove = findIndexByDS(dSCToMove);

        /*Verify if valid argument*/
        if(idR != -1 && dSCToMove != -1){
            joinRoom(indexToMove,roomName);
            sendMessageTo(tabClient[clientIndex].dS,"\nLe client a bien été déplacé ! \n\n");
        }
        else{
            sendMessageTo(tabClient[clientIndex].dS,"\nLa personne ou la room n'existe pas\n");
        }   
    }
    else{
        sendMessageTo(tabClient[clientIndex].dS,"\nVous ne possedez pas les droits pour effectuer cette commande\n\n");
    }
    
}


int kickFromRoom(char* pseudo,char* roomName){

    int idRoomParam = findIdRoomByName(roomName);
    int dSC = findDSClientByPseudo(pseudo);
    int index = findIndexByDS(dSC);
    int idR = tabClient[index].roomId;
    int isMoved = 0;

    if(idR == idRoomParam && idR !=-1){
        tabClient[index].roomId = -1;
        roomsList[idR].members[index] = -1;
        isMoved = 1;
    }

    return isMoved;
    
}


void kickFromRoomTreatment(int clientIndex){
    
    char* roomName = (char*) malloc(SIZEOFROOMNAME);
    char* pseudo = (char *) malloc(SIZEOFPSEUDO);
    char* msg1 = "\nEntrez le nom du client que vous souhaitez kick :\n\n";
    char* msg2 = "\nEntrez le nom de la room où vous souhaitez l'exclure :\n\n";
    char* status = (char *) malloc(SIZEOFMESSAGE);

    getFeedBackFrom(clientIndex,msg1,pseudo);
    getFeedBackFrom(clientIndex,msg2,roomName);

    int dSToKick = findDSClientByPseudo(pseudo);

    if(isAdminServer(clientIndex)||isAdminOf(clientIndex,roomName)||isModeratorOf(clientIndex,roomName)){
        
        if(dSToKick != -1){

            if(kickFromRoom(pseudo,roomName)){

                strcpy(status,"\nLe client à été exclu avec succès !\n\n"); 
            }
            else{
                strcpy(status,"\nLe client n'est pas dans la room !\n\n"); 
            }
        }
        else{
            strcpy(status,"\nCette personne n'existe pas\n\n");
        }
        
    }
    else{
        strcpy(status,"\nVous ne possedez pas les droits nécessaires pour exclure quelqu'un de cette room\n\n");
    }
    sendMessageTo(tabClient[clientIndex].dS,status);
    
}


void kickTreatement(int clientIndex){

    /*Check if the client executing the command has the right to do so*/
    if(isAdminServer(clientIndex)){

        /*Ask the client for a pseudo*/
        char* pseudo = malloc(SIZEOFPSEUDO);
        char* msg = "\nEntrez le pseudo de la personne que vous souhaitez exclure :\n\n";
        getFeedBackFrom(clientIndex,msg,pseudo);
        int dSToKick = findDSClientByPseudo(pseudo);
        int indexToKick = findIndexByDS(dSToKick);


        /*if the client couldn't be kicked from the server*/
        if(indexToKick == -1){

            char* errorMsg = "\nCe client n'existe pas ou n'est pas connecté :(\n\n";
            sendMessageTo(tabClient[clientIndex].dS,errorMsg);

        }
        else{
            disconnectClient(clientIndex);
        }
    }
    /*Send a message if he doesn't have the rights*/
    else{

        char* msg = "\nVous ne possedez pas les droits pour effectuer cette commande\n\n";
        sendMessageTo(tabClient[clientIndex].dS,msg);
    }
}


void blackListClient(char* pseudoToBan){

    int dSC = findDSClientByPseudo(pseudoToBan);
    int index = findIndexByDS(dSC);

    /*THE FILE IS OPENED*/
    FILE* fp = fopen("Server/Data/blackList.txt","a");

    /*CHECK IF THE FILED IS SUCCESFULLY OPENED */
    if(fp == NULL){
        printf("probleme de fichiers lors de l'initialisation des rooms\n");
        exit(0);
    }
        char* line = (char *) malloc(sizeof(char)*100);
        bzero(line, sizeof(char)*100);

        /*Load the line to write*/
        strcpy(line,pseudoToBan);
        strcat(line,"\n");

        /*WE WRITE STARTING AT THE END OF THE FILE*/
        fprintf(fp,"%s",line);
        fclose(fp);
        
        free(line);

        /*Kick the client if he's connected*/
        disconnectClient(index);
  
        return;

}


void unBlackListClient(char* pseudoToUnban){

    /*THE FILE IS OPENED*/
    FILE* fp = fopen("Server/Data/blackList.txt","r+");

    int isDone = 0;
    char* strToken = malloc(sizeof(char)*30);
    int size = 0;
    int sizeToErase= 0;
    char* separators = "\n";

    /*CHECK IF THE FILED IS SUCCESFULLY OPENED */
    if(fp == NULL){
        printf("probleme de fichiers lors de l'initialisation des rooms\n");
        exit(0);
    }
    char* currentLine = (char *) malloc(sizeof(char)*100);
    bzero(currentLine, sizeof(char)*100);
    
    while((fgets(currentLine, sizeof(char)*100, fp) != NULL)&& !isDone)
    {

        strToken = strtok(currentLine,separators);
        printf("Taille : %d\n",size);

        printf("Token : %s\n",strToken);
        /*Compares the current pseudo to the one in parameter*/
        if(strcmp(strToken,pseudoToUnban)==0){
            printf("ça passe ici\n\n");
            sizeToErase = strlen(strToken);

            /*/!\ DIRTY Write above the pseudo to get rid of the ban*/
            for(int i = 0;i<sizeToErase;i++){
                fseek( fp,i+size, SEEK_SET );
                fputs(" ", fp);
                fputs("\n", fp);
            }
            isDone = 1;
        }
        /*Count the nb of char to get to the matching pseudo*/
        else{
            size = size + strlen(strToken)+1;
        }
    }
    fclose(fp);
}


void banTreatment(int clientIndex){

    /*Check if the admin has the right to do so*/
    if(isAdminServer(clientIndex)){

        /*Ask the client executing the command for a pseudo*/
        char* pseudo = malloc(SIZEOFPSEUDO);
        char* msg = "\nEntrez le pseudo de la personne que vous souhaitez bannir :\n";
        getFeedBackFrom(clientIndex,msg,pseudo);
        int dSToBan = findDSClientByPseudo(pseudo);
        int indexToBan = findIndexByDS(dSToBan);

        /*Check if the client is connected*/
        if(dSToBan !=-1){
            blackListClient(pseudo);
        }
        /*If not*/
        else{
            char* errorMsg = "\nCe client n'existe pas ou n'est pas connecté :(\n\n";
            sendMessageTo(tabClient[clientIndex].dS,errorMsg);
        }
    }
    /*If he doesn't have the rights*/
    else{
        char* msg = "\nVous n'êtes pas admin du server cette commande est impossible\n\n";
        sendMessageTo(tabClient[clientIndex].dS,msg);
    }    
}


void unBanTreatment(int clientIndex){

    char* statusMsg = malloc(SIZEOFMESSAGE);

    /*Check if the admin has the right to do so*/
    if(isAdminServer(clientIndex)){

        /*Ask the client executing the command for a pseudo*/
        char* pseudo = malloc(SIZEOFPSEUDO);
        char* msg = "Entrez le pseudo de la personne que vous souhaitez deban :\n";
        getFeedBackFrom(clientIndex,msg,pseudo);
        int dSToUnban = findDSClientByPseudo(pseudo);
        int indexToUnban = findIndexByDS(dSToUnban);
        unBlackListClient(pseudo);
        strcpy(statusMsg,"\nLe client à été deban\n\n");
    }
    /*If he doesn't have the rights*/
    else{
        strcpy(statusMsg,"\nVous n'êtes pas admin du server cette commande est impossible\n\n");
    }
    sendMessageTo(tabClient[clientIndex].dS,statusMsg);    
}


void displayCommandToClient(int indexC){

    /*Open the file*/
    FILE* fp = fopen("./Server/Data/man.txt","r");

    /*Check if the file is successfully opened*/
    if(fp == NULL){
        printf("probleme de fichiers lors de l'initialisation des rooms\n");
        exit(0);
    }

    char* currentLine = (char *) malloc(sizeof(char)*500);
    char* buffer = (char *) malloc(sizeof(char *)*500);
    strcpy(buffer,"");

    /*Until it reach EOF*/
    while((fgets(currentLine, sizeof(char)*100, fp) != NULL))
        {
            printf("envoi en cours\n");
            strcat(buffer,currentLine);
        }
    sendMessageTo(tabClient[indexC].dS,buffer);
    free(currentLine);
    free(buffer);
}


void displayRoomsToClient(int clientIndex){

    int dSClient;
    int clientIndexs;
    char* msg = (char*) malloc(sizeof(char)*500);
    char name[100];
    strcpy(msg,"\n\n____________________________________________________________\n\n");
    strcat(msg,"                      Liste des Rooms                       \n");
    strcat(msg,"____________________________________________________________\n");

    /*Cross every rooms to display them*/    
    for(int i=0;i<NBOFROOMS;i++){

        /*Check if the room is Displayed*/
        if(roomsList[i].isDisplayed != -1){

            strcat(msg,"\n");
            strcat(msg,"** ");
            strcat(msg,roomsList[i].name);
            strcat(msg," **\n\n");
            strcat(msg,"\tDescription : ");
            strcat(msg,roomsList[i].description);
            strcat(msg,"\n\n");
            strcat(msg,"\tMembres : ");

            /*Display every clients in the room*/
            for(long j=0;j<nbClientMax;j++){

                clientIndexs = roomsList[i].members[j];

                if(clientIndexs != -1){

                    strcpy(name,tabClient[clientIndexs].pseudo);
                    strcat(msg," ");
                    strcat(msg,name);

                }
                
            }
            
            strcat(msg,"\n\n\0");
        }
        
    }
    strcat(msg,"____________________________________________________________\n\n\n");
    sendMessageTo(tabClient[clientIndex].dS,msg);
    /*sendMessageTo(tabClient[clientIndex].dS,"\n\n");*/
    free(msg);
    
}


void saveRoomInfo(){

    char * line;
    char * create;

    /*Suppression fichier*/
    int ret = remove("Server/Data/roomNames.txt");
    if (ret == -1){
        perror("erreur suppression updateRoom: \n");
        exit(1);
    }

    /*Création du fichier*/
    int fd = open("Server/Data/roomNames.txt", O_CREAT | O_WRONLY, 0666);
    if (fd == -1){
        perror("erreur création updateRoom: \n");
        exit(1);
    }

    /*Cross all the data needed and pack it in line*/
    for (int i = 0; i < NBOFROOMS; i++){

        line = (char *) malloc(sizeof(char)*100);
        create = (char *)malloc(sizeof(char)*100);
        bzero(line, sizeof(char)*100);
        bzero(create, sizeof(char)*100);
        
        /*NOM*/
        strcat(line,roomsList[i].name);
        strcat(line,",");

        /*CREATED*/
        sprintf(create,"%d",roomsList[i].isDisplayed);
        strcat(line, create);
        strcat(line,",");
        /*DESCR*/
        strcat(line, roomsList[i].description);
        strcat(line,",");
        strcat(line, "\n");
        

        int w = write(fd,line,strlen(line));
        if(w == -1){
            perror("erreur au write\n");
            exit(1);
        }

        free(create);
    }
    free(line);
    return;
}


void closeRoom(int clientIndex,char* roomName){

    char* defaultDescription = "Description par défaut !";
    int idR = findIdRoomByName(roomName);

    /*Check if the client executing the command has the right to do so*/
    if(isAdminOf(clientIndex,roomName)||isAdminServer(clientIndex)){

        pthread_mutex_lock(&mutex_rooms);

        /*Set the room to it's default value*/
        roomsList[idR].nbOfMembers = 0;
        roomsList[idR].isDisplayed = -1;
        nbOfRoomDisplayed = nbOfRoomDisplayed -1;
        strcpy(roomsList[idR].description,defaultDescription);

        for (int i=0;i<NBOFROOMS;i++){
            if(roomsList[idR].members[i]!=-1){
                /*makes clients leave the room*/
                tabClient[roomsList[idR].members[i]].roomId = -1;
            }
            roomsList[idR].members[i]=-1;
        }
        
        /*Release the mutex*/
        pthread_mutex_unlock(&mutex_rooms);
        printf("Le salon %s a été fermé\n",roomName);
        
        /*Saves the rooms modification in the file*/
        saveRoomInfo();
    }
    else{
        char* errorMsg = "\nVous ne possedez pas les droits pour fermer cette room\n\n";
        sendMessageTo(tabClient[clientIndex].dS,errorMsg);
    }


}


void shutdownRooms(){

    pthread_mutex_lock(&mutex_rooms);

    /*Save the rooms info in the txt file*/
    saveRoomInfo();

    /*Free the space allocated*/
    for (int i=0;i<NBOFROOMS;i++){

        if(roomsList[i].isDisplayed){
            free(roomsList[i].name);
            free(roomsList[i].description);
        }
        
    }
    pthread_mutex_unlock(&mutex_rooms);
    printf("Les salons ont été fermés ! \n\n");
}


void CTRL_C_Treatment(int sign){

    printf("\nCTRL C détecté !\n\n");
    shutdownRooms();

    /*Clients info / space management is managed when they are disconnected*/

    shutdown(dS, 2);
    shutdown(dSFile, 2);
    exit(EXIT_SUCCESS);
}

int findIdRoomByName(char* roomName){
    
    for(int i=0;i<NBOFROOMS;i++){
        if(strcmp(roomsList[i].name,roomName)==0){
            return roomsList[i].id;
        }
    }
    return -1;
}


void joinRoomTreatment(int clientIndex){

    /*Ask the client for a room name*/
    displayRoomsToClient(clientIndex);
    
    char* m = malloc(SIZEOFMESSAGE);
    char* notif = malloc(SIZEOFMESSAGE);
    char* roomName = malloc(SIZEOFROOMNAME);
    strcpy(m,"Entrez le nom du salon que vous souhaitez rejoindre :\n\n");
    getFeedBackFrom(clientIndex,m,roomName);
    int idR = findIdRoomByName(roomName);

    /*Call the join room function*/
    int joinRoomResult = joinRoom(clientIndex,roomName);
    char* resultMsg = (char*) malloc(sizeof(char)*500);
    printf("%d\n",joinRoomResult);
    /*Send an appropriate message with the result of the function*/
    switch(joinRoomResult){
        case 0:
            strcpy(resultMsg,"\nLa room est pleine\n\n");
            break;
        case -1:
            strcpy(resultMsg,"\nLa room n'existe pas\n\n");
            break;
        case 1: 
            strcpy(resultMsg,"\n\n____________________________________________________________\n");
            strcat(resultMsg,"                          ");
            strcat(resultMsg,roomName);
            strcat(resultMsg,"           \n");
            strcat(resultMsg,"____________________________________________________________\n");
            strcat(resultMsg,"\n\nBienvenue dans la salle de discussion ");
            strcat(resultMsg,roomName);
            strcat(resultMsg," !\n\n");
            
            break;
        default:
            break;
    }
    

    sendMessageTo(tabClient[clientIndex].dS,resultMsg);

    strcpy(notif,"Le client ");
    strcat(notif,tabClient[clientIndex].pseudo);
    strcat(notif," a rejoint la salle de discussion !\n\n");
    sendMessageToRoom(clientIndex,notif);

    free(notif);
    free(resultMsg);
    free(roomName);
}



int joinRoom(int clientIndex,char* roomName){

    int idRoom = findIdRoomByName(roomName);
    int result = 0;

    /* send a message to the client if the room doesn't existe*/
    if(idRoom ==-1 || roomsList[idRoom].isDisplayed == -1){ 
        result = -1;
    }
    else{

        pthread_mutex_lock(&mutex_tab);

        /*If the client is already in a room*/
        if(tabClient[clientIndex].roomId != -1){
            printf("\n\non passe ici \n\n");
            exitRoom(clientIndex,roomsList[tabClient[clientIndex].roomId].name);
        }

        /*Lock the mutex so two people cannot join at the exact same time*/
        
        int nbOfClientInRoom = roomsList[idRoom].nbOfMembers;

        /*Verify if the room is full*/
        printf("%d\n",roomsList[idRoom].maxMembers);
        printf("nombre de client dans la salle %d\n",nbOfClientInRoom);
        if(roomsList[idRoom].maxMembers > nbOfClientInRoom){

            /*if not, the client join the room*/
            roomsList[idRoom].members[clientIndex] = clientIndex;
            tabClient[clientIndex].roomId = idRoom;

            if(tabClient[clientIndex].roleTab[idRoom] == -1){
                tabClient[clientIndex].roleTab[idRoom] = 0;
            }

            printf("le client à rejoins la salle avec succès !\n");

            /*increments the nb of Client in the room*/
            roomsList[idRoom].nbOfMembers = roomsList[idRoom].nbOfMembers + 1;
            printf("le client à rejoin salle num %d\n",tabClient[clientIndex].roomId);
            result = 1;
        }
        else{
            result = 0;
        }

        pthread_mutex_unlock(&mutex_tab);
    }
    printf("idr du client : %d\n",tabClient[clientIndex].roomId);
    printf("nombre de client dans la salle %d\n",roomsList[idRoom].nbOfMembers);
    return result;
}

void changeRoomDescrTreatment(int clientIndex){

    char* roomName = (char*) malloc(SIZEOFROOMNAME);
    getFeedBackFrom(clientIndex,"\nEntrez le nom de la room de discussion :\n\n",roomName);
    int idR = findIdRoomByName(roomName);

    if(idR!=-1){
        changeRoomDescription(clientIndex,roomName);
    }
    else{
        sendMessageTo(tabClient[clientIndex].dS,"\nCette room n'existe pas ! \n\n");
    }
}

void changeRoomDescription(int clientIndex,char* roomName){

    if(isAdminOf(clientIndex,roomName)||isAdminServer(clientIndex)||isModeratorOf(clientIndex,roomName)){

        /*Ask the client executing the command for a description*/
        char* newDescription = malloc(SIZEOFDESCR);
        char* msg = "\nEntrez la description de votre salle de discussion :\n\n";
        int idR = findIdRoomByName(roomName);
        getFeedBackFrom(clientIndex,msg,newDescription);

        /*Reinit the room description and gives it it new values*/
        free(roomsList[idR].description);
        roomsList[idR].description = (char *) malloc(SIZEOFDESCR);
        strcpy(roomsList[idR].description,newDescription);

        free(newDescription);
    }
    else{
        sendMessageTo(tabClient[clientIndex].dS,"\nVous ne possedez pas les droits pour changer la description de cette room\n\n");
    }
    
}


int firstIdRoomNotDisplayed(){

    int roomNotDisplayed = -1;
    int c = 0;
    int index = -1;
    while(roomNotDisplayed==-1 && c<NBOFROOMS){
        if(roomsList[c].isDisplayed == -1){
            index = c;
            roomNotDisplayed = 1;
        }
        c = c + 1;
    }
    return index;
}

/*
createRoom : Int -> Void
*/
void createRoom(int clientIndex){

    /*Verify if the capacity is at it's max*/
    if(nbOfRoomDisplayed == NBOFROOMS){
        char* errorMessage = "\nCréation d'une salle impossible : le nombre maximum de salle a été atteint\n\n"; 
        sendMessageTo(tabClient[clientIndex].dS,errorMessage);
    }
    else{

        printf("Le client %s souhaite créer une salle de discussion\n",tabClient[clientIndex].pseudo);
        
        int idR = firstIdRoomNotDisplayed();

        /*The client become the admin of the Room he create*/
        tabClient[clientIndex].roleTab[idR] = 2;

        /*Ask for a valid room name and a description*/
        changeRoomName(clientIndex,roomsList[idR].name);
        changeRoomDescription(clientIndex,roomsList[idR].name);

        /*modify rooms attributes*/
        pthread_mutex_lock(&mutex_rooms);
        printf("id de la room créee : %d\n",idR);
        roomsList[idR].isDisplayed = 1;
        nbOfRoomDisplayed = nbOfRoomDisplayed + 1;

        pthread_mutex_unlock(&mutex_rooms);

        char* successMsg = "\nVous avez créee la salle de discussion avec succès !\n\n";
        sendMessageTo(tabClient[clientIndex].dS,successMsg);
        printf("Salle de discussion %s créée avec succès ! \n\n",roomsList[idR].name);

    }
    /*Saves the rooms modification in the file*/
    
    saveRoomInfo();

}


void setModeratorOf(int indexAdmin,char* pseudo,char* roomName){

    int idR = findIdRoomByName(roomName);
    int indexFuturModerator = findIndexByDS(findDSClientByPseudo(pseudo));
    char* statusMsg; 

    if(idR!=-1){

        /*Check if the client calling is an admin*/
        if(isAdminOf(indexAdmin,roomName)||isAdminServer(indexAdmin)){
            if(indexFuturModerator != -1){
                
                /*Check if the client isn't already Admin or mod*/
                if(isModeratorOf(indexFuturModerator,roomName)){
                    statusMsg = "\nLa personne possède déjà les droits que vous souhaitez lui attribuer\n\n";
                }
                else{
                    tabClient[indexFuturModerator].roleTab[idR] = 1;
                    updateClientFromDB(pseudo);
                    statusMsg = "\nLa personne à bien été nommée modérateur\n\n";
                }
            }
            else{
                statusMsg = "\nLa personne n'est pas connectée ou n'existe pas !\n\n";
            }
        }
        else{
            statusMsg = "\nVous ne possedez pas les droits nécessaires !\n\n";
        }
    }
    else{
        statusMsg = "\nLa room n'existe pas !\n\n";
    }
    sendMessageTo(tabClient[indexAdmin].dS,statusMsg);

}


void setAdminServer(int indexAdmin,char* pseudo){

    int dSFuturAdmin = findDSClientByPseudo(pseudo);
    int indexFuturAdmin = findIndexByDS(dSFuturAdmin);
    char* statusMsg = malloc(SIZEOFMESSAGE); 

    /*Check if the client calling is an admin*/
    if(isAdminServer(indexAdmin)){

        /*If the client is connected*/
        if(indexFuturAdmin != -1){

            /*Check if the client is already Admin*/
            if(isAdminServer(indexFuturAdmin)){
                statusMsg = "\nLa personne possède déjà les droits que vous souhaitez lui attribuer\n\n";
            }
            else{
                tabClient[indexFuturAdmin].isAdmin = 1;
                updateClientFromDB(pseudo);
                statusMsg = "\nLa personne à bien été nommée Admin\n\n";
            }
                
        }
        else{
            statusMsg = "\nCette personne n'est pas connectée ou n'existe pas !\n\n";
        }
    }
    else{
        statusMsg = "\nVous ne possedez pas les droits nécessaires pour nommer Admin du server\n\n";
    }

    sendMessageTo(tabClient[indexAdmin].dS,statusMsg);
    printf("\n\nfin fonction setAdminServ\n\n");

}


void setAdminOf(int indexAdmin,char* pseudo,char* roomName){

    /* /!\rajouter ça */
    /*tabClient[indexAdmin].roomId;*/
    int idR = findIdRoomByName(roomName);
    int dSFuturAdmin = findDSClientByPseudo(pseudo);
    int indexFuturAdmin = findIndexByDS(dSFuturAdmin);
    char* statusMsg = malloc(SIZEOFMESSAGE); 

    /*Check if the client calling is an admin*/
    printf("idR dans setAdmin : %d\n",indexFuturAdmin);
    if(isAdminOf(indexAdmin,roomName)||isAdminServer(indexAdmin)){

        /*Check the client is in the room*/
        if(indexFuturAdmin != -1){
            
            if(idR != -1){

            /*Check if the client is already Admin*/
            if(isAdminOf(indexFuturAdmin,roomName)){
                statusMsg = "\nLa personne possède déjà les droits que vous souhaitez lui attribuer\n\n";
            }
            else{
                tabClient[indexFuturAdmin].roleTab[idR] = 2;
                updateClientFromDB(pseudo);
                statusMsg = "\nLa personne à bien été nommée Admin\n\n";
                }    
            }
            else{
                statusMsg = "\nLa room n'existe pas ! \n\n";
            }            
        }
        else{
            statusMsg = "\nCette personne n'est pas connectée ou n'existe pas !\n\n";
        }
    }
    else{
        statusMsg = "\nVous ne possedez pas les droits nécessaires !\n\n";
    }
    sendMessageTo(tabClient[indexAdmin].dS,statusMsg);
}

/*hasMoreRights*/
int hasMoreRights(int index1, int index2, int idR){

    if(tabClient[index1].roleTab[idR]>tabClient[index2].roleTab[idR]){
        return 1;
    }
    else{
        if(tabClient[index1].roleTab[idR]==2){
            return 1;
        }
    }
    return 0;
}


void setRandomInRoom(int indexAdmin,char* pseudo,char* roomName){

    int idR = findIdRoomByName(roomName);
    int indexFuturRandom = findIndexByDS(findDSClientByPseudo(pseudo));
    char* statusMsg; 

    /*Check if the client calling is an admin*/
    if(isAdminOf(indexAdmin,roomName)||isModeratorOf(indexAdmin,roomName)||isAdminServer(indexAdmin)){
        /*If the client is connected*/
        if(indexFuturRandom != -1){
            
            if(idR!=-1){

                /*Check if the client has more rights than the person he tries to demote*/
                if(isAdminServer(indexAdmin) || hasMoreRights(indexAdmin,indexFuturRandom,idR)){
                    tabClient[indexFuturRandom].roleTab[idR] = 0;

                    printf("droit dans setRandomInRoom %d\n",tabClient[indexFuturRandom].roleTab[idR]);
                    printf("idR dans setRandomInRoom %d\n",idR);

                 updateClientFromDB(pseudo);
                 statusMsg = "\nLa personne à bien été demote\n\n";
                }
                else{
                    statusMsg = "\nLa personne que vous souhaitez demote est votre superieur hierarchique\n\n";
                }
            }
            else{
                statusMsg = "\nLa room n'existe pas ! \n\n";
            }
    
        }
        else{
            statusMsg = "\nCette personne n'est pas connectée ou n'existe pas !\n\n";
        }
    }
    else{
        statusMsg = "\nVous ne possedez pas les droits nécessaires !\n\n";
    }
    sendMessageTo(tabClient[indexAdmin].dS,statusMsg);

}


/*isAdminServer*/
int isAdminServer(int indexClient){
    return tabClient[indexClient].isAdmin;
}

/*isAdminOF*/
int isAdminOf(int indexClient,char *roomName){

    int idR = findIdRoomByName(roomName);

    if( idR != -1 && tabClient[indexClient].roleTab[idR]==2){
        return 1;
    }

    return 0;
}
/*isModeratorOf*/
int isModeratorOf(int indexClient,char* roomName){

    int idCR = findIdRoomByName(roomName);

    if( idCR != -1 && tabClient[indexClient].roleTab[idCR]==1){
            return 1;
    }
    return 0;
}


int isRoomNameAvailable(char* name){

    /*Cross every room to check if the name given in parameter is used*/
    for(int i = 0; i<NBOFROOMS ; i++){
    
        if(strcmp(roomsList[i].name,name)==0){

            if(roomsList[i].isDisplayed != -1){
                printf("isDisplayed : %d",roomsList[i].isDisplayed);
                printf("room name found%s\n",roomsList[i].name);
                return 0;
            }
        }
    }
    
    return 1;
}


void whoIsHere(int clientIndex){

    char* clientConnected = malloc(sizeof(char)*400);
    char* message = "\nLes clients connectés sont : \n\n";
    char* form = "\t- ";
    char* backToLine = "\n";

    strcat(clientConnected,message);

    /*Cross all the client connected and write it in a string*/
    for(int i = 0;i<nbClientMax;i++){
        if(tabClient[i].dS != -1){
            strcat(clientConnected,form);
            strcat(clientConnected,tabClient[i].pseudo);
            strcat(clientConnected,backToLine);
            strcat(clientConnected,backToLine);
        }
    }
    sendMessageTo(tabClient[clientIndex].dS,clientConnected);
    free(clientConnected);
}

/*
changeRoomName
*/
void changeRoomName(int clientIndex,char* Oldname){

    /*Check if the caller has the rights to do so */
    if(isModeratorOf(clientIndex,Oldname)||isAdminOf(clientIndex,Oldname)||isAdminServer(clientIndex)){

        int isRoomNameValid = 0;
        int dSC = tabClient[clientIndex].dS;
        char* msgToC = "\nEntrez un nouveau nom pour la salle de discussion :\n\n";
        
        
        sendMessageTo(dSC,msgToC);
        int idR = findIdRoomByName(Oldname);

        while(!isRoomNameValid){

            char* newName = malloc(SIZEOFPSEUDO);
            char* statusMsg;
            receiveMessageFrom(dSC,newName);

            /*If the room name is available*/
            if(isRoomNameAvailable(newName)){
                printf("le nom  \"%s\" n'est pas utilisé\n ",newName);
                pthread_mutex_lock(&mutex_rooms);

                /*Mise à jour du nom*/
                strcpy(roomsList[idR].name,newName);
                printf("Modification du nom de la salle effectuée !\n");
                pthread_mutex_unlock(&mutex_rooms);
                isRoomNameValid = 1;
                statusMsg = "\nVous avez renommé la salle de discussion avec succès !\n\n";
            }
            else{
                statusMsg = "\nCe nom est deja utilisee choisissez en un autre\n\n";
            }
            sendMessageTo(dSC,statusMsg);
            free(newName);
        }
        /*Saves the rooms modification in the file*/
        saveRoomInfo();
    }
    else{
        char* errorMessage = "\nVous ne disposez pas des droits pour effectuer cette commande\n\n";
        sendMessageTo(tabClient[clientIndex].dS,errorMessage);
    }
    
}

/*
exitRoom : Int -> Void
*/
void exitRoom(int clientIndex,char* roomName){

    int roomId = findIdRoomByName(roomName);

    printf("roomId %d\n\n",roomId);
    printf("Room Id du client%d\n\n",tabClient[clientIndex].roomId);

    /*Verify that the client is in a room*/
    if(roomId !=-1 && tabClient[clientIndex].roomId == roomId){

        printf("\n\nca passe ici chacal\n\n");
        
        char* notif = malloc(SIZEOFMESSAGE);
        pthread_mutex_lock(&mutex_rooms);

        roomsList[roomId].members[clientIndex] = -1;
        roomsList[roomId].nbOfMembers = roomsList[roomId].nbOfMembers - 1;
        tabClient[clientIndex].roomId = -1;
        /*unleash the mutex*/
        pthread_mutex_unlock(&mutex_rooms);
        /*sends the info to the client*/
        char* successMsg = "\nVous avez quitté le salon avec succès !\n\n";
        sendMessageTo(tabClient[clientIndex].dS,successMsg);

        strcpy(notif,"Le client ");
        strcat(notif,tabClient[clientIndex].pseudo);
        strcat(notif," a quitté la salle de discussion !\n\n");
        sendMessageToRoom(clientIndex,notif);
    }

}


void exitRoomTreatement(int clientIndex){

    /*Ask the client from a rooom name*/
    char* message = "\nEntrez le nom salon que vous souhaitez quitter :\n\n";
    char* roomToLeave = malloc(SIZEOFROOMNAME);
    getFeedBackFrom(clientIndex,message,roomToLeave);
    int idR = findIdRoomByName(roomToLeave);

    if(idR != -1){
        exitRoom(clientIndex,roomToLeave);
    }
    else{
        char* errorMessage = "\nCe nom ne correspond à aucun salon de discussion\n\n";
            sendMessageTo(tabClient[clientIndex].dS,errorMessage);
        }
}


void deleteRoomTreatement(int clientIndex){

    char* message = "\nEntrez le nom du salon que vous souhaitez supprimer :\n\n";
    char* roomName = malloc(SIZEOFROOMNAME);

    displayRoomsToClient(clientIndex);
    getFeedBackFrom(clientIndex,message,roomName);

    /*Verifiy if it exists*/
    int idR7 = findIdRoomByName(roomName);
    if(idR7 != -1){
        closeRoom(clientIndex,roomName);
    }
    else{
        char* errorMessage = "\nCe nom ne correspond à aucun salon de discussion\n\n";
        sendMessageTo(tabClient[clientIndex].dS,errorMessage);
    }
}

void promoteToAdminOfServerTreatement(int clientIndex){

    char* message = "\nEntrez le pseudo de la personne que vous souhaitez rendre Admin du server :\n\n";
    char* pseudo = malloc(SIZEOFPSEUDO);

    getFeedBackFrom(clientIndex,message,pseudo);
    setAdminServer(clientIndex,pseudo);
    free(pseudo);
}

// void demoteServerTreatement(int clientIndex){

//     char* message = "Entrez le pseudo de la personne que vous souhaitez rendre Admin du server\n";
//     char* pseudo = malloc(SIZEOF);
//     int dS = findDSClientByPseudo(pseudo);
//     int index = findIndexByDS(dS);

//     getFeedBackFrom(clientIndex,message,pseudo);

    
//     free(pseudo);
// }

void promoteToAdminOfRoomTreatement(int clientIndex){
    char* message = "\nEntrez le pseudo de la personne que vous souhaitez rendre Admin d'une room :\n\n";
    char* pseudo = malloc(SIZEOFPSEUDO);
    char* roomName = malloc(SIZEOFROOMNAME);
    getFeedBackFrom(clientIndex,message,pseudo);
    message = "\nEntrez le nom de la salle :\n\n";
    getFeedBackFrom(clientIndex,message,roomName);
    setAdminOf(clientIndex,pseudo,roomName);
    free(roomName);
    free(pseudo);
}

void promoteToModOfRoomTreatement(int clientIndex){
    char* message = "\nEntrez le pseudo de la personne que vous souhaitez rendre modérateur d'une room :\n\n";
    char* pseudo = malloc(SIZEOFPSEUDO);
    char* roomName = malloc(SIZEOFROOMNAME);
                                                                    
    getFeedBackFrom(clientIndex,message,pseudo);
    message = "\nEntrez le nom de la salle :\n\n";
    getFeedBackFrom(clientIndex,message,roomName);
    setModeratorOf(clientIndex,pseudo,roomName);
    free(roomName);
    free(pseudo);

    printf("\n\nFin promoteToModTreatment\n\n");
}

void demoteRoomTreatement(int clientIndex){
    char* message = "\nEntrez le pseudo de la personne à demote : ";
    char* pseudo = malloc(SIZEOFPSEUDO);
    char* roomName = malloc(SIZEOFROOMNAME);
                                                                        
    getFeedBackFrom(clientIndex,message,pseudo);
    message = "\n\nEntrez le nom de la salle :";
    getFeedBackFrom(clientIndex,message,roomName);
    setRandomInRoom(clientIndex,pseudo,roomName);
    free(pseudo);
    free(roomName);
}


void renameRoomTreatement(int clientIndex){
    char* m = "\nEntrez le nom salon que vous souhaitez modifier :\n\n";
    char* currentRoomName = malloc(SIZEOFROOMNAME);

    getFeedBackFrom(clientIndex,m,currentRoomName);
    int idR = findIdRoomByName(currentRoomName);
    if(idR != -1){
        changeRoomName(clientIndex,currentRoomName);
    }
    else{
        char* errorMessage = "\nCe nom ne correspond à aucun salon de discussion\n\n";
        sendMessageTo(tabClient[clientIndex].dS,errorMessage);
    }
}



/* Sending functions ======================================================== */


void*  getFileSendable(void* dSC){

    long dSClient = (long) dSC;
    /*on récupère*/
    int cr = system("ls Server/Files/ > Server/Data/fileList.txt");
    /*Vérification*/
    if(cr == -1){
        printf("Commande echouée\n");
    }
    FILE *fp;
    fp = fopen("Server/Data/fileList.txt", "r");
    if(fp==NULL){
        perror("Erreur de lecture du fichier");
        exit(0);
    }
    char data[50];
    char* bufferFileList = (char*) malloc(sizeof(char)*500);

    /* Until the whole file is sent, sending file data */
    while(fgets(data, sizeof(char)*50, fp) != NULL) {

        strcat(bufferFileList,data);
        bzero(data, sizeof(char)*50);
    }
    strcat(bufferFileList,"\n");
    sendMessageTo(dSClient,bufferFileList);

    // if (s == -1) {
            
    //         perror("Erreur d'envoi du fichier.\n");
    //         printf("envoi des fichiers en cours");
    //         exit(1);
    // }

    /* Waiting for the file name sent by the client */
    char* fileName = malloc(SIZEOFMESSAGE);
    receiveMessageFrom(dSClient,fileName);
    printf("nom du fichier reçu : %s\n",fileName);

    /* Determining the file path */
    char* filePath = malloc(SIZEOFMESSAGE);
    strcpy(filePath,"Server/Files/");
    strcat(filePath,fileName);
    printf("chemin fichier : %s\n",filePath);

    free(bufferFileList);

    /* Sending file */
    send_file(filePath);
    /* Releasing the memory */
    free(filePath);
    free(fileName);
    printf("on termine le thread\n");
    fclose(fp);
    pthread_exit(0);

}


void send_file(char* filePath){

    /* Creating and connecting socket */
    struct sockaddr_in aC1;
    socklen_t lg1 = sizeof(struct sockaddr_in);
    char data[SIZE];
    int dSCFile = 0;
    int size =-1;
    dSCFile = accept(dSFile, (struct sockaddr*) &aC1,&lg1);

    /* Opening the file to access */
    int fd;
    fd = open(filePath, O_RDONLY);
    printf("\n%s\n",filePath);
    /* Checking errors */
    if(fd==-1){
      perror("Erreur de lecture du fichier");
      exit(0);
    }

    /* While there is something to read */
    while(size != 0) {

        size = read(fd,data,SIZE-1);
        
        /* Read the file and send its size to the server */
        send(dSCFile,&size,sizeof(int),0);

        if(size!=0){

            if (send(dSCFile, data, SIZE, 0) == -1) {

                perror("Erreur d'envoi du fichier.\n");
                exit(1);
            }
            printf("le fichier s'envoie\n");
        }
        /* Resetting the buffer */
        bzero(data, SIZE);
    }

    close(fd);
    shutdown(dSCFile,2);

}


void sendMessageToAll(int dSC, char * msg){
    /* Crossing all the connected clients */
    for (int i=0;i<nbClient;i++){
        /* if it is not the message author */
        if(dSC != tabClient[i].dS && tabClient[i].dS!=-1){
            
            printf("envoie au dS qui est : %d\n",tabClient[i].dS);
            int sendR = send(tabClient[i].dS, msg, strlen(msg)+1, 0);
            /* Checking errors */
            if (sendR == -1){
                perror("erreur\n");
                exit(-1);
            }
            else{
                printf("ok\n");
            }
            
        }
    }
}

void sendMessageTo(int dSC,char*msg){

    int sendR = send(dSC, msg, strlen(msg)+1, 0);

    /* Checking errors */
    if (sendR == -1){
        perror("erreur\n");
        exit(-1);
    }
}


void sendMessageToRoom(int clientIndex,char*msg){

    int idR = findIDRByIndex(clientIndex);
    int dSSendingClient = tabClient[clientIndex].dS;
    int clientReceivingIndex;
    int clientReceivingdS;
    printf("idR %d\n",idR);

    if(idR!=-1){
        for(int i = 0;i<nbClientMax;i++){

            printf("Les membres : %d\n",roomsList[idR].members[i]);
            /*Verification in order to send only to everyone else*/
            clientReceivingIndex = roomsList[idR].members[i];
            if(clientReceivingIndex != -1 && clientReceivingIndex != clientIndex){
                    sendMessageTo(tabClient[clientReceivingIndex].dS,msg);
            }   
        }
    }
    else{
        printf("idr est -1 ! \n");
        for(int i = 0;i<nbClientMax;i++){
            if(tabClient[i].roomId == -1 && i!= clientIndex && tabClient[i].dS!=-1){
                clientReceivingIndex = i;
                sendMessageTo(tabClient[clientReceivingIndex].dS,msg);
            }
        }
    }
    printf(" fin\n");
}


/* Receiving functions ======================================================== */


int receiveMessageFrom(int dSC, char* msg){

    /* Receiving the message of the first client*/
    int recvR1 = recv(dSC, msg, sizeof(char)*100, 0);

    /* Checking errors */
    if (recvR1 == -1){
        perror("Une erreur est survenue lors de la réception du message.\n");
        exit(-1);
    }
    else{
        printf("\nLe message reçu est : %s \n", msg); 
        
    }
    /*We delete \n*/
    msg[strcspn(msg, "\n")] = 0;
    return recvR1;
}


void getFeedBackFrom(int clientIndex,char* msgToSend, char* msgBuffer){

    int dSC = tabClient[clientIndex].dS;
    sendMessageTo(dSC,msgToSend);
    receiveMessageFrom(dSC,msgBuffer);
}


void registerClientToDB(int indexClient,char* password){

    char * line;
    char * create;

    /*THE FILE IS OPENED*/
    FILE* fp = fopen("Server/Data/clientInfo.txt","a");
    if(fp == NULL){
        printf("probleme de fichiers lors de l'initialisation des rooms\n");
        exit(0);
    }
        line = (char *) malloc(sizeof(char)*100);
        create = (char *)malloc(sizeof(char)*100);
        bzero(line, sizeof(char)*100);
        bzero(create, sizeof(char)*100);
        
        /*NAME*/
        strcat(line,tabClient[indexClient].pseudo);
        strcat(line,",");
    
        /*PASSWORD*/
        strcat(line,password);
        strcat(line,",");

        /*ROOMSTATUS*/
        for(int j=0;j<NBOFROOMS;j++){
            
            sprintf(create,"%d",tabClient[indexClient].roleTab[j]);
            strcat(line, create);
            strcat(line,",");
        }
        /*BOOLEAN ISADMINOFSERVER*/
        sprintf(create,"%d",tabClient[indexClient].isAdmin);
        strcat(line, create);
        strcat(line,",");
        strcat(line, "\n");

        /*WE WRITE STARTING AT THE END OF THE FILE*/
        fprintf(fp,"%s",line);
        fclose(fp);
        free(create);
    
        free(line);

        return;
}


void* receiveFile(void* nomFichier){

    /* Accepting the connection for the file transfer */
    char* nomDuFichier = (char*) nomFichier;

    /* Recovering the file name */
    char* cheminFichier = malloc(SIZEOFMESSAGE);
    strcpy(cheminFichier,"Server/Files/");
    strcat(cheminFichier,nomFichier);
    printf("chemin fichier : %s\n",cheminFichier);
    struct sockaddr_in aC1;
    socklen_t lg1 = sizeof(struct sockaddr_in);
    int dSCFile = accept(dSFile, (struct sockaddr*) &aC1,&lg1);

    /* Writing the file */
    int fd;
    int recvF;
    char buffer[SIZE];
    int sizeToWrite =-1;
    recv(dSCFile, &sizeToWrite, sizeof(int), 0); 
    fd = open(cheminFichier, O_WRONLY | O_CREAT,0666);

    /* While there is something to receive */
    while(sizeToWrite != 0){
        
        /* Recovering the file data and writing */
        recvF = recv(dSCFile, buffer, SIZE, 0);
        write(fd,buffer,sizeToWrite);
        /* Resetting the buffer*/
        bzero(buffer,SIZE);
        /* Recovering from the server how many bytes to receive and writing */
        recv(dSCFile, &sizeToWrite, sizeof(int), 0);
    }

    close(fd);
    free(cheminFichier);
    printf("\nVous avez recu le fichier !\n");
    return NULL;
}


int encodeCommand(char *msg){

    /* extract the command */
    char* cmd = extractCommand(msg);
    printf("extractCommand renvoie %s\n",cmd);
    int idCmd = -1;
    printf("La commande est : %s\n", cmd);

    /* End the conversation */
    if(strcmp(cmd,"/end")==0){
        idCmd = 0;
    }

    /* Send private message */
    if(strcmp(cmd,"/mp")==0){
        idCmd = 1;
    }

    /* Upload file */
    if(strcmp(cmd,"/file")==0){
        idCmd = 2;
    }

    /* Download file */
    if(strcmp(cmd,"/filerequest")==0){
        idCmd = 3;
    }

    /* Join a room */
    if(strcmp(cmd,"/jroom")==0){
        idCmd = 4;
    }

    /* Create a room */
    if(strcmp(cmd,"/croom")==0){
        idCmd = 5;
    }

    /* Rename a room */
    if(strcmp(cmd,"/rroom")==0){
        idCmd = 6;
    }

    /* Delete a room */
    if(strcmp(cmd,"/droom")==0){
        idCmd = 7;
    }

    /* Display the rooms */
    if(strcmp(cmd,"/rooms")==0){
        idCmd = 8;
    }

    /* Exit a room */
    if(strcmp(cmd,"/eroom")==0){
        idCmd = 9;
    }
    /*Promote someone to admin*/
    if(strcmp(cmd,"/promoteToAdmin")==0){
        idCmd = 10;
    }
    /*Promote someone to moderator*/
    if(strcmp(cmd,"/promoteToMod")==0){
        idCmd = 11;
    }
    /*demote someone*/
    if(strcmp(cmd,"/demote")==0){
        idCmd = 12;
    }
    /*kick someone from the server*/
    if(strcmp(cmd,"/kick")==0){
        idCmd = 13;
    }
    /*display the clients connected*/
    if(strcmp(cmd,"/whoishere")==0){
        idCmd = 14;
    }
    /*sends a message to everyclient*/
    if(strcmp(cmd,"/all")==0){
        idCmd = 15;
    }
    /*display all the commands available*/
    if(strcmp(cmd,"/man")==0){
        idCmd = 16;
    }
    /*move a client from a room to another*/
    if(strcmp(cmd,"/move")==0){
        idCmd = 17;
    }
    /*ban a client from the server*/
    if(strcmp(cmd,"/ban")==0){
        idCmd = 18;
    }
    /*promote a client to Admin of the server*/
    if(strcmp(cmd,"/setAdminServ")==0){
        idCmd = 19;
    }
    /*unban a client from the server*/
    /*To finish*/
    if(strcmp(cmd,"/unban")==0){
        idCmd = 20;
    }
    /*to change pseudo*/
    if(strcmp(cmd,"/changePseudo")==0){
        idCmd = 21;
    }
    /*kick from a room*/
    if(strcmp(cmd,"/kickFromRoom")==0){
        idCmd = 22;
    }
    /*kick from a room*/
    if(strcmp(cmd,"/deleteAccount")==0){
        idCmd = 23;
    }
    /*to change password*/
    if(strcmp(cmd,"/changePassword")==0){
        idCmd = 24;
    }
    /*to change the description of a room*/
    if(strcmp(cmd,"/changeRoomDesc")==0){
        idCmd = 25;
    }
    /* Wrong entry */
    if(idCmd == -1) {
        printf("idCmd = %d Cette commande n'est pas reconnue, veuillez réessayer\n",idCmd);
    }
    return idCmd;
}

void disconnectClient(int clientIndex){

    /* Closing the connections and resetting the variables */
    nbClient = nbClient - 1;
    close(tabClient[clientIndex].dS);
    reinitCellTab(tabClient[clientIndex].dS);
    sem_post(&semaphoreNbClient);
    printf("Il y'a desormais %d clients.\n\n",nbClient);

}

int executeCommand(int clientIndex,long dSC,int rcv,pthread_t threadReceiveFile,pthread_t threadSendFile,char* msg){

    if(isCommand(msg)){
        int command = encodeCommand(msg);
        printf("Command code is : %d\n",command);
        printf("Searching attribute..\n");
        char* attribute1 = malloc(sizeof(char)*15);
        char* separator = " ";
        attribute1 = extractAttributeN(1,msg," ");
        printf("attribute = %s\n",attribute1);
        printf("Command execution...\n");

        switch(command){
            case -1:;
                sendMessageTo(tabClient[clientIndex].dS,"\nCette commande n'existe pas\n\n");
                break;
            /* END THE CONVERSATION */
            case 0:;
                /* Closing the connections and resetting the variables */
                disconnectClient(clientIndex);
                return 1;

            /* SEND PRIVATE MESSAGE */
            case 1:;
                printf("y'a t'il quelqu'un avec ce pseudo\n");
                char* attribute2 = extractPrivateMessage(msg);
                printf("private message is = %s\n",attribute2);
                /* Searching the socket descripter of the recipient client */
                int dSmp = findDSClientByPseudo(attribute1);
                if(dSmp != -1){
                    printf("envoie du msg privé à :%s\n",attribute1);
                    char* m1 = malloc(SIZEOFMESSAGE);
                    strcpy(m1,tabClient[clientIndex].pseudo);
                    strcat(m1," vous envoi : ");
                    strcat(m1,attribute2);
                    strcat(m1,"\n");
                    sendMessageTo(dSmp,m1);
                    free(m1);
                }
                break;          

            /* UPLOAD FILE */
            case 2:;
                printf("C'est un envoi de fichier\n");
                /* Storing the file name */
                char* m2 = "\nSaisissez le nom du fichier à envoyer parmi :\n\n";
                sendMessageTo(dSC,m2);
                char* nomFichier = malloc(SIZEOFMESSAGE);
                receiveMessageFrom(dSC,nomFichier);
                printf("nomFichier: %s\n",nomFichier);
                /* Creating a thread to receive the file */
                pthread_create(&threadReceiveFile,NULL,(void*)receiveFile,(void*)nomFichier);
                break;

            /* DOWNLOAD FILE */
            case 3:;
                pthread_create(&threadSendFile,NULL,(void*)getFileSendable,(void*)dSC);
                break;

            /* JOIN A ROOM */
            case 4:;
                printf(" b switch case\n");
                joinRoomTreatment(clientIndex);
                printf(" a switch case\n");
                break;

            /* CREATE A ROOM */
            case 5:;
                createRoom(clientIndex);
                break;

            /* RENAME A ROOM */
            case 6:;
                renameRoomTreatement(clientIndex);
                break;

            /* DELETE A ROOM */
            case 7:;
                deleteRoomTreatement(clientIndex);
                break;

            /* DISPLAY ROOMS */
            case 8:;
                displayRoomsToClient(clientIndex);
                break;

            /* EXIT A ROOM */
            case 9:;
                exitRoomTreatement(clientIndex);
                break;

            /* PROMOTE SOMEONE TO ADMIN OF A ROOM*/
            case 10:;
                promoteToAdminOfRoomTreatement(clientIndex);
                break;
            /* PROMOTE SOMEONE TO MODERATOR OF A ROOM*/
            case 11:;
                promoteToModOfRoomTreatement(clientIndex);
                break;

            /* DEMOTE SOMEONE*/
            case 12:;
                 demoteRoomTreatement(clientIndex);
               break;

            /* KICK SOMEONE FROM A ROOM*/   
            case 13:;
                kickTreatement(clientIndex);
                break;

            /*DISPLAY WHO IS CONNECTED TO THE SERVER*/
            case 14:;
                whoIsHere(clientIndex);
                break;

            /*SEND A MESSAGE TO EVERY CLIENT IN EVERY ROOM*/
            case 15:;
                messageTreatement(msg,clientIndex);
                sendMessageToAll(tabClient[clientIndex].dS,msg);
                break;
            
                /*DISPLAY EVERY COMMAND TO THE CLIENT*/
            case 16:;
                displayCommandToClient(clientIndex);
                break;

                /*MOVE A CLIENT FROM A ROOM TO ANOTHER*/
            case 17:;
                moveClient(clientIndex);
                break;

                /*BAN A CLIENT FROM THE SERVER*/
            case 18:;
                banTreatment(clientIndex);
                break;

                /*PROMOTE A CLIENT TO ADMIN OF THE SERVER*/
            case 19:;
                promoteToAdminOfServerTreatement(clientIndex);
                break;
            
            /*UNBAN A CLIENT FROM THE SERVER*/
            case 20:;
                unBanTreatment(clientIndex);
                break;

            /*TO CHANGE PSEUDO*/    
            case 21:;
                changePseudo(clientIndex);
                break;

            /*KICK SOMEONE FROM A ROOM*/
            case 22:;
                kickFromRoomTreatment(clientIndex);
                break;

            /*DELETE A USER FROM THE DB AND DISCONNECT HIM*/
            case 23:;
                removeClientFromDB(tabClient[clientIndex].pseudo);
                disconnectClient(clientIndex);
                return 1;
            
            /*CHANGE PASSWORD FOR A NEW ONE*/
            case 24:;
                changePassword(clientIndex);
                break;
            /*CHANGE ROOM DESCR*/
            case 25:;
                changeRoomDescrTreatment(clientIndex);
                break;

        free(attribute1);
        return 0;
        }
    }
    else{
        /*VERIFY IF BANWORDS ARE USED*/
        if(hasInsult(msg)){
            printf("insulte détectée\n");
            hasInsultTreatement(clientIndex);
        }
        /* SEND MESSAGE TO ALL */
        else{
            printf("Retransmission du message aux autres clients\n");
            messageTreatement(msg,clientIndex);
            /*Send the message to the default room*/
            printf("idR Dans transfert : %d\n",findIDRByIndex(clientIndex));
            printf("id Dans transfert : %d\n",clientIndex);
            sendMessageToRoom(clientIndex,msg);
        } 
    }
    return 0;
}


void *transfert(void* indice){

    int isEnd = 0;
    int clientIndex = (long) indice;
    long dSCurrentClient = tabClient[clientIndex].dS;
    char* password = malloc(SIZEOFPSWD);
    pthread_t threadReceiveFile;
    pthread_t threadSendFile;
    int rcv;
    /* Entering the pseudo */
    selectPseudo(dSCurrentClient);
    /*Check if Client in DB*/
    int isCRegistered = isClientRegistered(tabClient[clientIndex].pseudo,password);
    if(isCRegistered){
        /*Check if the password matches with the pseudo if in DB*/
        if(loginUser(clientIndex,password) != 1){

            /*Disconnect the client if wrong password*/
            close(dSCurrentClient);
            reinitCellTab(dSCurrentClient);
            sem_post(&semaphoreNbClient);
            nbClient = nbClient - 1;
            isEnd = 1;

        }
        /*The client is connected get the info from the DB*/
        else{
            initClientFromDB(clientIndex,tabClient[clientIndex].pseudo);            
        }
    }
    /*register the client if not in DB*/
    else{
        registerUser(clientIndex);
    }

    /* While the conversation is not stopped by the client */
    while(!isEnd){

        char * msg = (char *) malloc(SIZEOFMESSAGE);
        /* Listening to the client */
        printf("\n\non écoute le client là \n\n");
        rcv = receiveMessageFrom(dSCurrentClient,msg);
        /*If the client is disconnected end the thread*/
        if(rcv ==0 ){
            isEnd = 1;
            disconnectClient(clientIndex);
        }
        else{
            /* if the /end command is received */
            isEnd = executeCommand(clientIndex,dSCurrentClient,rcv,threadReceiveFile,threadSendFile,msg);
        }
        
        printf("isEnd vaut : %d",isEnd);
        /* Releasing allocated memory */
        free(msg);   
    }
    pthread_exit(0);
}



/* Main server program */

int main(int argc, char *argv[]) {

    /* Setting variables*/
    char* endWord = "fin\n";
    nbClient = 0;
    nbClientMax = atoi(argv[2]);
    nbOfRoomDisplayed = 0;
    sem_init(&semaphoreNbClient,0,nbClientMax);
    int PORT = atoi(argv[1]);
    int PORTFILE = PORT + 1;
    
    /*Structures initializations*/

    for(int i = 0;i<nbClientMax;i++){
        tabClient[i].dS=-1;
        tabClient[i].pseudo ="NULL";
        tabClient[i].roomId = -1;
        tabClient[i].nbOfWarnings = 0;
        tabClient[i].isAdmin = 0;
    }
    printf("Structures initialized\n");

    /*----------------------------------- Server setting -----------------------------------*/

    printf("\n---- Phase de préparation du serveur ----\n\n");
  
    if(argc != 3){
        printf("lancez le programme avec ./server adresseIP nbClientMax");
        exit(-1);
    }
    printf("Message socket initialization port : %d\n",PORT);
    dS = createServerSocket(PORT);
    printf("File socket initialization port : %d\n",PORTFILE);
    dSFile = createServerSocket(PORTFILE);

    printf("\nInitiating rooms\n");

    initRooms();

    /*The program is looking for this signal*/
    signal(SIGINT, CTRL_C_Treatment);


    /*----------------------------------- Communication state -----------------------------------*/

    while(1){
        
        /* While the table tabClient is not full */
        sem_wait(&semaphoreNbClient);
        struct sockaddr_in aC1;
        socklen_t lg1 = sizeof(struct sockaddr_in);
        int dSC = accept(dS, (struct sockaddr*) &aC1,&lg1);

        /* Checking errors */
        if (dSC == -1){
            perror("erreur\n");
            exit(-1);
        }
        else{
            /* Client has been well connected */
            printf("Connexion ok !\n\n");
            /* Searching an empty case in tabClient, then affect the client to it */
            long i = firstIndexEmpty(tabClient,5);
            /* Creating data sent to the thread */
            pthread_mutex_lock(&mutex_tab);
            tabClient[i].dS = dSC;
            tabClient[i].pseudo = "NULL";

            pthread_mutex_unlock(&mutex_tab);
            /* Creating client thread */
            pthread_create(&tabThread[i],NULL,transfert,(void*)i);
            /* Incrementing the number of client(s) */
            nbClient = nbClient + 1;
            printf("Le nombre de client est :%d\n",nbClient);
                
        }
    }
    shutdown(dS,2);
    return 0;
    
}

