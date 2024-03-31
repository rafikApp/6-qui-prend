#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#define PORT 8070
#define BUFFER_SIZE sizeof(Table)




typedef struct{
    int number;
    int tetesDeBoeuf;
    char name[100];    
}Carte;

typedef struct{
    Carte cartes[10];
    int points;
    int socket;
    char name[100];
    Carte carteJouet;
    int nbCartesRestantes;
    int indice;
    int gagné;
}Joueur;

typedef struct
{
    int NbCartes;
    Carte cartes[104];
}Paquet;

typedef struct {
    Carte **ranges;
    int *nombresCartesRange;
    int *tetesDeBoeuf;
    int manche;
    int tour;
}Table;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//Joueur* joueurs[MAX_PLAYERS];
// static int nbJoueurs;
Paquet *paquetJeu;
Table *tableJeu;
int MAX_PLAYERS;
int SCORE;
int max=0;
int min;





void afficheCarte(Carte carte)
{
      if(carte.tetesDeBoeuf==1)
            printf("\033[1;31mcarte n° %d : contient 1 tete de boeuf par : %s\033[0m\n ",carte.number,carte.name);
      else
            printf("\033[1;31mcarte n° %d : contient %d tetes de boeuf par : %s\033[0m\n ",carte.number,carte.tetesDeBoeuf,carte.name);
}
void afficherPaquet(Joueur *joueur)
{
    for(int i=0;i<joueur->nbCartesRestantes;i++)
        afficheCarte(joueur->cartes[i]);
} 

Carte retirCarte(Paquet *paquet)
{
  Carte carte;
  if(paquet->NbCartes>0)
  {
     
      carte = paquet->cartes[paquet->NbCartes-1];
      paquet->NbCartes = paquet->NbCartes - 1;
      return carte;
  } 
  else
  {
    carte.number = 0;
    carte.tetesDeBoeuf = 0;
    
  }
  return carte; 
}

void initialiserPaquetJoueur(Joueur joueur)
{
  for(int i=0;i<10;i++)
    {
      joueur.cartes[i] = retirCarte(paquetJeu); 
    }
} 

Joueur initialiserJoueur()
{
  Joueur joueur;
  joueur.nbCartesRestantes=10;
  initialiserPaquetJoueur(joueur);
  return joueur;
} 



void placerDansRangee(Table *tableau, Carte *carte, int *socketsJoueurs,Joueur *joueur) {
   
    int differenceMinimale = INT_MAX;
    int rangeeCible = -1;

   
    for (int i = 0; i < 4; ++i) {
        
        // Vérifier si la carte peut être placée dans la rangee
        if (carte->number >= tableau->ranges[i][0].number) {
            // Vérifier si la carte est inférieure à toutes les cartes de la rangee
            bool peutEtrePlacee = true;
            for (int j = 0; j < tableau->nombresCartesRange[i]; ++j) {
                if (carte->number <= tableau->ranges[i][j].number) {
                    peutEtrePlacee = false;
                    break;
                }
            }
            // Si la carte peut être placée, mettre à jour la rangee cible si la différence est plus petite
            if (peutEtrePlacee) {
                int difference = carte->number - tableau->ranges[i][tableau->nombresCartesRange[i]-1].number;

                // Mettre à jour la rangee cible si la différence est plus petite
                if (difference < differenceMinimale) {
                    differenceMinimale = difference;
                    rangeeCible = i;
                }
            }
        }
    }

    // Placer la carte dans la rangee cible
    if (rangeeCible != -1) {
        // Vérifier si la taille de la rangee est inférieure à 5 avant d'ajouter une nouvelle carte
        if (tableau->nombresCartesRange[rangeeCible] < 5) 
        {
            // Ajouter la nouvelle carte à la rangee
            tableau->ranges[rangeeCible][tableau->nombresCartesRange[rangeeCible]] = *carte;
            tableau->nombresCartesRange[rangeeCible]++;
            tableau->tetesDeBoeuf[rangeeCible] += carte->tetesDeBoeuf;
        } 
        else 
        {
            for (int i = 0; i < tableau->nombresCartesRange[rangeeCible]; ++i) 
            {
            joueur->points += tableau->ranges[rangeeCible][i].tetesDeBoeuf;
            }

        // Vider la rangée avec le moins de têtes de bœuf
        tableau->nombresCartesRange[rangeeCible] = 1;
        tableau->tetesDeBoeuf[rangeeCible] = carte->tetesDeBoeuf;
        tableau->ranges[rangeeCible][0] = *carte;

        }
    }
}


Table *initialiserTable( Paquet *paquet) {
    Table *table = malloc(sizeof(Table));
    table->ranges = malloc(4 * sizeof(Carte *));
    table->nombresCartesRange = malloc(4 * sizeof(int));
    table->tetesDeBoeuf = malloc(4 * sizeof(int));
    

    for (int i = 0; i < 4; i++) {
        table->ranges[i] = malloc(5 * sizeof(Carte));
        table->nombresCartesRange[i] = 0;
        table->ranges[i][0] = retirCarte(paquet);
        strcpy(table->ranges[i][0].name,"Gestionnaire");
        table->nombresCartesRange[i]++;
        table->tetesDeBoeuf[i] = table->ranges[i][0].tetesDeBoeuf;
    }
    table->manche = 1;
    table->tour = 1;
    return table;
}


void libererTable(Table *table) {
    for (int i = 0; i < 4; i++) {
        free(table->ranges[i]);
    }
    free(table->ranges);
    free(table->nombresCartesRange);
    free(table);
}


void serializeJoueur(const Joueur *joueur, char *buffer, size_t bufferSize) {
    if (joueur == NULL || buffer == NULL || bufferSize < sizeof(Joueur)) {
        return;
    }

    // Copie les membres de la structure sauf le tableau de cartes
    size_t offset = 0;
    memcpy(buffer + offset, &joueur->points, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &joueur->socket, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &joueur->indice, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &joueur->gagné, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &joueur->nbCartesRestantes, sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, joueur->name, sizeof(char) * 100);
    offset += sizeof(char) * 100;

    memcpy(buffer + offset, &joueur->carteJouet, sizeof(Carte));
    offset += sizeof(Carte);

    // Copie les cartes une par une
    for (int i = 0; i < 10; ++i) {
        memcpy(buffer + offset, &joueur->cartes[i], sizeof(Carte));
        offset += sizeof(Carte);
    }
}



 

Paquet* initialiserPaquet()
{
    Paquet *paquet = malloc(sizeof(Paquet));
    paquet->NbCartes=104;

     for (int i = 0; i < 104; i++)
  {
    paquet->cartes[i].number = i + 1;

    if ((i + 1) == 55) // Numéro 55 est le seul numéro qui est multiple de 11 et se termine par 5
    {
      paquet->cartes[i].tetesDeBoeuf = 7;
    }

    else if ((i + 1) % 10 == 0) // Numéros qui se termine par 0
    {
      paquet->cartes[i].tetesDeBoeuf = 3;
    }

    else if ((i + 1) % 5 == 0 && (i + 1) % 2 != 0) // Numéros qui se termine par 5
    {
      paquet->cartes[i].tetesDeBoeuf = 2;
    }

    else if ((i + 1) % 11 == 0) // Numéros multiple de 11
    {
      paquet->cartes[i].tetesDeBoeuf = 5;
    }
    else // Cartes normaux
    {
      paquet->cartes[i].tetesDeBoeuf = 1;
    }
  }
  
  for (int i = 0; i < 104; i++)
  {
    /* On choisit un entier au pif entre i et 104; */
    int randval = (rand() % (104 - i)) + i;
    /* On permute Carte[i] et Carte[randval] */
    Carte tmp = paquet->cartes[i];
    paquet->cartes[i] = paquet->cartes[randval];
    paquet->cartes[randval] = tmp;
  }
    return paquet;
}

void afficheTable(Table *table) {
    printf("Manche n°%d tour n°%d\n\n",table->manche,table->tour-1);
    for (int i = 0; i < 4; i++) {
        printf("\033[1;31mRangee %d : \033[0m", i + 1);
        printf("\033[1;31m( %d )\033[0m\n ",table->tetesDeBoeuf[i]);
        for (int j = 0; j < table->nombresCartesRange[i]; j++) {
            afficheCarte(table->ranges[i][j]);
        }
        printf("\n");
    }
}

void afficheTableFinale(Table *table) {
    for (int i = 0; i < 4; i++) {
        printf("\033[1;31mRangee %d : \033[0m", i + 1);
        printf("\033[1;31m( %d )\033[0m\n ",table->tetesDeBoeuf[i]);
        for (int j = 0; j < table->nombresCartesRange[i]; j++) {
            afficheCarte(table->ranges[i][j]);
        }
        printf("\n");
    }
}


void initialiserJeu(Paquet* paquet, Table* table, Joueur* joueurs, int nbJoueurs)
{
  paquet = initialiserPaquet();
  for(int i=0;i<nbJoueurs;i++)
   {
      for(int j=0;j<10;j++)
     {
        Carte carte = retirCarte(paquet);
        joueurs[i].cartes[j] = carte;
     }   
   } 
  table = initialiserTable(paquet);
}


void deserializeJoueur(const char *buffer, size_t bufferSize, Joueur *joueur) {
    if (buffer == NULL || joueur == NULL || bufferSize < sizeof(Joueur)) {
        return;
    }

    // Copie les membres de la structure sauf le tableau de cartes
    size_t offset = 0;
    memcpy(&joueur->points, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&joueur->socket, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&joueur->indice, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&joueur->gagné, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&joueur->nbCartesRestantes, buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(joueur->name, buffer + offset, sizeof(char) * 100);
    offset += sizeof(char) * 100;

    memcpy(&joueur->carteJouet, buffer + offset, sizeof(Carte));
    offset += sizeof(Carte);

    // Copie les cartes une par une
    for (int i = 0; i < 10; ++i) {
        memcpy(&joueur->cartes[i], buffer + offset, sizeof(Carte));
        offset += sizeof(Carte);
    }
}






int trouverRangeeAvecMoinsTetesDeBoeuf(Table *tableau) {
    int minTetesDeBoeuf = INT_MAX;
    int indiceMinTetes = -1;

    // Parcourir les rangées pour trouver celle avec le moins de têtes de bœuf
    for (int i = 0; i < 4; ++i) {
        if (tableau->tetesDeBoeuf[i] < minTetesDeBoeuf) {
            minTetesDeBoeuf = tableau->tetesDeBoeuf[i];
            indiceMinTetes = i;
        }
    }

    return indiceMinTetes;
}


void testerCarteEtViderRangee(Table *tableau, Carte *carte,int *socketsJoueurs, Joueur *joueur) {
    int rangeeAVider = 7;
    //int minTetesDeBoeuf = INT_MAX;
    for (int i = 0; i < 4; i++) {
        
        int derniereCarte = tableau->ranges[i][tableau->nombresCartesRange[i] - 1].number;
        if (carte->number < derniereCarte) {
            rangeeAVider = rangeeAVider -1;
        }
    }
    if (rangeeAVider == 3) {
        
        rangeeAVider = trouverRangeeAvecMoinsTetesDeBoeuf(tableau);
        for (int i = 0; i < tableau->nombresCartesRange[rangeeAVider]; ++i) {
            joueur->points += tableau->ranges[rangeeAVider][i].tetesDeBoeuf;
        }
        tableau->nombresCartesRange[rangeeAVider] = 1;
        tableau->tetesDeBoeuf[rangeeAVider] = carte->tetesDeBoeuf;
        tableau->ranges[rangeeAVider][0] = *carte;
    }
    else
    {
      placerDansRangee(tableau,carte,socketsJoueurs,joueur);
    } 
}




void serializeCard(Carte *carte, char *buffer, size_t *offset) {
    memcpy(buffer + *offset, carte, sizeof(Carte));
    *offset += sizeof(Carte);
}

// Fonction de désérialisation pour Card
void deserializeCard(char *buffer, size_t *offset, Carte *carte) {
    memcpy(carte, buffer + *offset, sizeof(Carte));
    *offset += sizeof(Carte);
}

// Fonction de sérialisation pour int *
void serializeIntPtr(int *data, int count, char *buffer, size_t *offset) {
    memcpy(buffer + *offset, data, sizeof(int) * count);
    *offset += sizeof(int) * count;
}

// Fonction de désérialisation pour int *
void deserializeIntPtr(char *buffer, size_t *offset, int *data, int count) {
    memcpy(data, buffer + *offset, sizeof(int) * count);
    *offset += sizeof(int) * count;
}

// Fonction de sérialisation pour Table
void serializeTable(Table *table, char *buffer, size_t bufferSize) {
    size_t offset = 0;

    // Débogage : Afficher le nombre de cartes dans chaque rangée côté serveur
    for (int i = 0; i < *table->nombresCartesRange; ++i) {
        printf("Nombre de cartes dans la rangée %d côté serveur : %d\n", i + 1, table->nombresCartesRange[i]);
    }

    // Sérialiser ranges
    for (int i = 0; i < *table->nombresCartesRange; ++i) {
        serializeCard(table->ranges[i], buffer, &offset);
    }

    // Sérialiser nombresCartesRange
    serializeIntPtr(table->nombresCartesRange, *table->nombresCartesRange, buffer, &offset);

    // Sérialiser tetesDeBoeuf
    serializeIntPtr(table->tetesDeBoeuf, *table->nombresCartesRange, buffer, &offset);

    memcpy(buffer + offset, &(table->manche), sizeof(int));
    offset += sizeof(int);

    memcpy(buffer + offset, &(table->tour), sizeof(int));
    offset += sizeof(int);
}

// Fonction de désérialisation pour Table
void deserializeTable(char *buffer, size_t bufferSize, Table *table) {
    size_t offset = 0;

    // Désérialiser ranges
    for (int i = 0; i < *table->nombresCartesRange; ++i) {
        deserializeCard(buffer, &offset, table->ranges[i]);
    }

    // Désérialiser nombresCartesRange
    deserializeIntPtr(buffer, &offset, table->nombresCartesRange, *table->nombresCartesRange);

    // Désérialiser tetesDeBoeuf
    deserializeIntPtr(buffer, &offset, table->tetesDeBoeuf, *table->nombresCartesRange);

    // Débogage : Afficher le nombre de cartes dans chaque rangée côté joueur
    for (int i = 0; i < *table->nombresCartesRange; ++i) {
        printf("Nombre de cartes dans la rangée %d côté joueur : %d\n", i + 1, table->nombresCartesRange[i]);
    }

    memcpy(&(table->manche), buffer + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&(table->tour), buffer + offset, sizeof(int));
    offset += sizeof(int);
}

// Fonction pour libérer la mémoire allouée pour Table
void freeTableMemory(Table *table) {
    // Libérer la mémoire pour ranges
    for (int i = 0; i < *table->nombresCartesRange; ++i) {
        free(table->ranges[i]);
    }
    free(table->ranges);

    // Libérer la mémoire pour nombresCartesRange et tetesDeBoeuf
    free(table->nombresCartesRange);
    free(table->tetesDeBoeuf);
}


size_t calculateBufferSize(Table *table) {
    size_t bufferSize = 0;

    // Taille pour ranges
    for (int i = 0; i < *table->nombresCartesRange; ++i) {
        bufferSize += sizeof(Carte);
    }

    // Taille pour nombresCartesRange et tetesDeBoeuf
    bufferSize += sizeof(int) * *table->nombresCartesRange * 2;

    return bufferSize;
}

void sendTable(int clientSocket, Table *table) {
    int size = 4 * 5 * sizeof(Carte) + 4 * sizeof(int) + 4 * sizeof(int) + 2 * sizeof(int); 
    char *data = (char *)malloc(size);
    int offset = 0;

    
    memcpy(data + offset, table->nombresCartesRange, 4 * sizeof(int));
    offset += 4 * sizeof(int);
    memcpy(data + offset, table->tetesDeBoeuf, 4 * sizeof(int));
    offset += 4 * sizeof(int);

    
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 5; ++j) {
            memcpy(data + offset, &table->ranges[i][j], sizeof(Carte));
            offset += sizeof(Carte);
        }
    }

   
    memcpy(data + offset, &table->manche, sizeof(int));
    offset += sizeof(int);
    memcpy(data + offset, &table->tour, sizeof(int));

    send(clientSocket, data, size, 0);
    free(data);
}

void initialiserJoueurs(Joueur* joueurs[])
{
    
    for(int i=0;i<MAX_PLAYERS;i++)
    {
        joueurs[i] = malloc(sizeof(Joueur));
        joueurs[i]->gagné=0; 
        joueurs[i]->indice = i;
        joueurs[i]->nbCartesRestantes = 10;
        for(int j=0;j<10;j++)
        {
            joueurs[i]->cartes[j] = retirCarte(paquetJeu);
        }
    }
}

       
      

int compareJoueurs(const void *a, const void *b) {
    const Joueur *joueurA = *(const Joueur **)a;
    const Joueur *joueurB = *(const Joueur **)b;

    // Comparaison basée sur le numéro de la carteJouet
    return joueurA->carteJouet.number - joueurB->carteJouet.number;
}


int trouverScoreMin(Joueur* joueurs[]) {
    int minScore = joueurs[0]->points;

    for (int i = 1; i < MAX_PLAYERS; i++) {
        if (joueurs[i]->points < minScore) {
            minScore = joueurs[i]->points;
        }
    }

    return minScore;
}

int trouverScoreMax(Joueur* joueurs[]) {
    int maxScore = joueurs[0]->points;

    for (int i = 1; i < MAX_PLAYERS; i++) {
        if (joueurs[i]->points > maxScore) {
            maxScore = joueurs[i]->points;
        }
    }

    return maxScore;
}

int scoresJoueurs(Joueur *joueurs[])
{   
    for(int i=0;i<MAX_PLAYERS;i++)
    {
        if(joueurs[i]->points>=SCORE)
        {

            max = trouverScoreMax(joueurs);
            min = trouverScoreMin(joueurs);
            return 1;
        } 
    }
    return 0;
} 

void ecrire(Joueur *joueurs[])
{
    char command[1000];
    snprintf(command, sizeof(command), "./ecrire.sh %d", tableJeu->manche);

    // Ajout des noms et scores à la commande
    for (int i = 0; i < MAX_PLAYERS; i++) {
        snprintf(command + strlen(command), sizeof(command) - strlen(command), " %s %d", joueurs[i]->name, joueurs[i]->points);
    }
    system(command);
} 



int main() {
    srand(time(NULL));
    int server_fd, new_socket , valread, nbJoueurs=0;
    struct sockaddr_in address;
    int opt = 1;
    int finPartie=0;
    int static manche=1;
    int addrlen = sizeof(address);
   
   
    printf("Entrer le nombre maximale des joueurs (entre 2 et 10) : ");
    scanf("%d",&MAX_PLAYERS);
    while(MAX_PLAYERS<2 || MAX_PLAYERS>10)
    {
        printf("Le nombre maximale des joueurs doît être compris entre 2 et 10 : ");
        scanf("%d",&MAX_PLAYERS);
    } 
    printf("Entrer le score : ");
    scanf("%d",&SCORE);

    Joueur* joueurs[MAX_PLAYERS];

    paquetJeu = initialiserPaquet();
    tableJeu = initialiserTable(paquetJeu);
    initialiserJoueurs(joueurs);


    
    // Création du socket serveur
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configuration des options du socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Attachement du socket à l'adresse et au port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Écoute de nouvelles connexions
    if (listen(server_fd, MAX_PLAYERS) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

       
    

    printf("En attente des joueurs...\n");
    printf("\n\n");

    
    
    for (int i = 0; i < MAX_PLAYERS; i++) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Erreur lors de l'acceptation de la connexion");
            exit(EXIT_FAILURE);
        }
        char nom[150];
        recv(new_socket, &nom, sizeof(nom), 0); 
        printf("Connexion de : %s\n",nom);
        if (joueurs[i]->socket == 0) 
        {
            //pthread_mutex_lock(&mutex);
            joueurs[i]->socket = new_socket;   
            nbJoueurs++;
            //pthread_mutex_unlock(&mutex);
        }
    }
    printf("\n\n");
    printf("le Jeu va commencer dans quelques secondes... \n");
    
    sleep(2);
    printf("\n\n\n");
    afficheTable(tableJeu);
    for(int i=0;i<MAX_PLAYERS;i++)
    {
         sendTable(joueurs[i]->socket,tableJeu);
    }
    
    int t=0;
    while(t==0){ 
        Joueur* jrs[MAX_PLAYERS];
        for(int i=0;i<MAX_PLAYERS;i++)
        {
            jrs[i]=(Joueur*)malloc(sizeof(Joueur)); 
        }  
        size_t nj = MAX_PLAYERS; 
        for(int i =0;i<MAX_PLAYERS;i++)
        {
                int socket = joueurs[i]->socket;
                char serializedBuffer[sizeof(Joueur)];
                serializeJoueur(joueurs[i], serializedBuffer, sizeof(Joueur));
                send(socket, serializedBuffer, sizeof(Joueur), 0);
                char receivedBuffer[sizeof(Joueur)];
                valread = recv(socket, receivedBuffer, sizeof(Joueur), 0);
                deserializeJoueur(receivedBuffer, sizeof(Joueur), joueurs[i]);
                if (valread <= 0) {
                    //printf("Joueur déconnecté.\n");
                    for(int j=0;j<MAX_PLAYERS;j++)
                    {
                        if(j!=i)
                        {
                            //printf("Joueur %s a gagné.\n", joueurs[j]->name);
                            joueurs[j]->gagné = 1; 
                            serializeJoueur(joueurs[j], serializedBuffer, sizeof(Joueur));
                            send(joueurs[j]->socket, serializedBuffer, sizeof(Joueur), 0);
                        } 
                    } 
                    exit(0);    
                }
                jrs[i] = joueurs[i];                
        }
            qsort(jrs, nj, sizeof(Joueur*), compareJoueurs);
            printf("\n\n");
            
            for(int i =0;i<MAX_PLAYERS;i++)
            {  
                testerCarteEtViderRangee(tableJeu,&jrs[i]->carteJouet,&jrs[i]->socket,jrs[i]);
                
            }
            for(int i =0;i<MAX_PLAYERS;i++)
            {  
                
                printf("\033[1;34mLe score de %s est : %d\033[0m\n",joueurs[i]->name,joueurs[i]->points);
            }
            printf("\n\n");
            tableJeu->tour= tableJeu->tour+1;
            afficheTable(tableJeu);            
            if(tableJeu->tour < 12)
            {
                for(int i=0;i<MAX_PLAYERS;i++)
                {
                    sendTable(joueurs[i]->socket,tableJeu);
                }
            } 
            
            if(tableJeu->tour == 11)
            {
                ecrire(joueurs);
                tableJeu->tour=1;
                manche = tableJeu->manche + 1;
                t = scoresJoueurs(joueurs);
                if(t==1)
                {
                    for(int j=0;j<MAX_PLAYERS;j++)
                    {
                            if(joueurs[j]->points>min)
                            {
                                sendTable(joueurs[j]->socket,tableJeu);
                                printf("%s a perdu avec un score de : %d\n",joueurs[j]->name,joueurs[j]->points);
                                joueurs[j]->gagné = -1;
                                char serializedBuffer[sizeof(Joueur)];
                                serializeJoueur(joueurs[j], serializedBuffer, sizeof(Joueur));
                                send(joueurs[j]->socket, serializedBuffer, sizeof(Joueur), 0);
                            }
                            else
                            {
                                sendTable(joueurs[j]->socket,tableJeu);
                                printf("%s a gagné avec un score de : %d\n",joueurs[j]->name,joueurs[j]->points);
                                joueurs[j]->gagné = 1;
                                char serializedBuffer[sizeof(Joueur)];
                                serializeJoueur(joueurs[j], serializedBuffer, sizeof(Joueur));
                                send(joueurs[j]->socket, serializedBuffer, sizeof(Joueur), 0);
                            }
                    }
                    printf("\n");
                    printf("La table finale :\n\n");
                    afficheTableFinale(tableJeu);
                    finPartie = 1;
                    break;
                }
                if(finPartie==0)
                {
                    paquetJeu = initialiserPaquet();
                    tableJeu = initialiserTable(paquetJeu);
                    tableJeu->manche = manche;
                    afficheTable(tableJeu);
                    for(int i=MAX_PLAYERS-1;i>=0;i--)
                    {
                        sendTable(joueurs[i]->socket,tableJeu);
                    }
                    for(int i=0;i<MAX_PLAYERS;i++)
                    {
                        memset(joueurs[i]->cartes, 0, sizeof(joueurs[i]->cartes));
                        joueurs[i]->nbCartesRestantes=10;
                        for(int j=0;j<10;j++)
                        {
                            joueurs[i]->cartes[j] = retirCarte(paquetJeu);
                            
                        } 
                    } 
                }
                
            }  
               
    }


    close(server_fd); // Fermeture du socket serveur après la partie

    return 0;
}
