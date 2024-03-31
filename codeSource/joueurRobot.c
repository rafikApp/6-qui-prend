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



// Fonction de desérialisation
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


int compareCards(const void *a, const void *b) {
    return ((Carte *)a)->number - ((Carte *)b)->number;
}

// Fonction pour trier les cartes d'un joueur
void trierPaquet(Joueur *joueur) {
    qsort(joueur->cartes, joueur->nbCartesRestantes, sizeof(Carte), compareCards);
}

void supprimerCarte(int pos, Joueur *joueur)
{
    for(int i=pos;i<joueur->nbCartesRestantes;i++)
        joueur->cartes[i] = joueur->cartes[i+1];
    joueur->nbCartesRestantes = joueur->nbCartesRestantes-1; 
    
} 

Carte rechercherCarte(int nombre,Joueur *joueur)
{
    Carte carte;
    carte.number = 0;
    for(int i=0;i<joueur->nbCartesRestantes;i++)
    {
        if(joueur->cartes[i].number == nombre)
        {
            carte = joueur->cartes[i];
            supprimerCarte(i,joueur);
            //printf("%d",joueur->nbCartesRestantes);
            return carte; 
        } 
    } 
    return carte;
} 

void afficheCarte(Carte carte)
{
      if(carte.tetesDeBoeuf==1)
            printf("\033[1;31mcarte n° %d : contient 1 tete de boeuf par : %s\033[0m\n ",carte.number,carte.name);
      else
            printf("\033[1;31mcarte n° %d : contient %d tetes de boeuf par : %s\033[0m\n ",carte.number,carte.tetesDeBoeuf,carte.name);
}

void afficheCartePaquet(Carte carte)
{
      if(carte.tetesDeBoeuf==1)
            printf("\033[1;31mcarte n° %d : contient 1 tete de boeuf \033[0m\n ",carte.number);
      else
            printf("\033[1;31mcarte n° %d : contient %d tetes de boeuf \033[0m\n ",carte.number,carte.tetesDeBoeuf);
}

void afficherPaquet(Joueur *joueur)
{
    for(int i=0;i<joueur->nbCartesRestantes;i++)
        afficheCartePaquet(joueur->cartes[i]);
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

void receivePlayerInfo(int socket, Joueur *player) {
    char buffer[sizeof(Joueur)];
    size_t bytesRead = 0;

    while (bytesRead < sizeof(Joueur)) {
        ssize_t result = recv(socket, buffer + bytesRead, sizeof(Joueur) - bytesRead, 0);

        if (result == -1) {
            perror("Erreur lors de la réception des données du joueur");
            break;
        } else if (result == 0) {
            printf("Client déconnecté.\n");
            break;
        }

        bytesRead += result;
    }

    if (bytesRead == sizeof(Joueur)) {
        memcpy(player, buffer, sizeof(Joueur));
        printf("%s",player->name);
        for(int i=0;i<10;i++)
    {
        afficheCarte(player->cartes[i]);
    } 
        
    }
}

Table *initialiserTable() {
    Table *table = malloc(sizeof(Table));
    
    table->ranges = malloc(4 * sizeof(Carte *));
    table->nombresCartesRange = malloc(4 * sizeof(int));
    table->tetesDeBoeuf = malloc(4 * sizeof(int));
    
    Carte carte;
    carte.number = 0;
    carte.tetesDeBoeuf = 0;
    for (int i = 0; i < 4; i++) {
        table->ranges[i] = malloc(5 * sizeof(Carte));
        table->nombresCartesRange[i] = 0;
        table->ranges[i][0] = carte;
        strcpy(table->ranges[i][0].name,"Serveur");
        table->nombresCartesRange[i]++;
        table->tetesDeBoeuf[i] = table->ranges[i][0].tetesDeBoeuf;
    }
    table->manche = 1;
    table->tour = 1;
    return table;
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


void receiveTable(int serverSocket, Table *table) {
    int size = 4 * 5 * sizeof(Carte) + 4 * sizeof(int) + 4 * sizeof(int) + 2 * sizeof(int); // Updated size for new members
    char *data = (char *)malloc(size);
    recv(serverSocket, data, size, 0);

    int offset = 0;

    // Copy integer arrays
    memcpy(table->nombresCartesRange, data + offset, 4 * sizeof(int));
    offset += 4 * sizeof(int);
    memcpy(table->tetesDeBoeuf, data + offset, 4 * sizeof(int));
    offset += 4 * sizeof(int);

    // Copy Card structure data
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 5; ++j) {
            memcpy(&table->ranges[i][j], data + offset, sizeof(Carte));
            offset += sizeof(Carte);
        }
    }

    // Copy new members
    memcpy(&table->manche, data + offset, sizeof(int));
    offset += sizeof(int);
    memcpy(&table->tour, data + offset, sizeof(int));

    free(data);
}


int verifier(Table *tableau, Carte *carte) {
    // Initialiser la différence minimale à une valeur maximale
    int differenceMinimale = INT_MAX;
    int rangeeCible = -1;

    // Parcourir les ranges pour trouver la rangee cible
    for (int i = 0; i < 4; ++i) {
        if(tableau->nombresCartesRange[i]==0)
        {
          rangeeCible = i;
          break;
        } 
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
                int difference = carte->number - tableau->ranges[i][0].number;

                // Mettre à jour la rangee cible si la différence est plus petite
                if (difference < differenceMinimale) {
                    differenceMinimale = difference;
                    rangeeCible = i;
                }
            }
        }
    }
    return rangeeCible;
}


Carte placerCarteRobot(Table* tableau,Joueur* joueur)
{
    for(int i=0;i<joueur->nbCartesRestantes;i++)
    {
        int rangeeCible = verifier(tableau,&joueur->cartes[i]);
        if(rangeeCible !=-1)
        {
            if(tableau->nombresCartesRange[rangeeCible] < 5)
            {
                return rechercherCarte(joueur->cartes[i].number,joueur);
            }
        }
    }
    
        for(int i=0;i<joueur->nbCartesRestantes;i++)
        {
        int rangeeCible = verifier(tableau,&joueur->cartes[i]);
        if(rangeeCible !=-1)
        {
               return rechercherCarte(joueur->cartes[i].number,joueur);
        }
        }
    
        return rechercherCarte(joueur->cartes[0].number,joueur);
   
}

// void placerDansRangee(Table *tableau,Joueur *joueur)
// {
//     for(int i=0;i<joueur->nbCartesRestantes;i++)
//     {
//         int v = verifier(tableau,&joueur->cartes[i]);
//         if(v!=-1)
//         {
//             if (tableau->nombresCartesRange[v] < 5) {
//             tableau->ranges[v][tableau->nombresCartesRange[v]] = joueur->cartes[i];
//             tableau->nombresCartesRange[v]++;
//             tableau->tetesDeBoeuf[v] += joueur->cartes[i].tetesDeBoeuf;
//         } 
//         else {
            
//             for (int i = 0; i < tableau->nombresCartesRange[v]; ++i) {
//             joueur->points += tableau->ranges[v][i].tetesDeBoeuf;
//         }        
//         tableau->nombresCartesRange[v] = 1;
//         tableau->tetesDeBoeuf[v] = joueur->cartes[i].tetesDeBoeuf;
//         tableau->ranges[v][0] = joueur->cartes[i];
//         }
//         }
//     }
// }

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char nom[150]; 
    Table* tableJeu = initialiserTable();
    //char buffer[BUFFER_SIZE];
    char ip[15];

    printf("Bienvenue au jeu\n");


    printf("Entrez une adresse ip : ");
    scanf("%15s", ip);
    printf("Entrer votre nom : ");
    scanf("%s", nom);
    printf("\n\n");

    // Création du socket joueurRobot
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        //printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Conversion de l'adresse IP du serveur de présentation de chaîne en format binaire
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }
    //printf("%d\n",sock);
    send(sock, &nom, strlen(nom), 0);
    
    // receiveTable(sock,tableJeu);
    receiveTable(sock,tableJeu);
    afficheTable(tableJeu); 
    Joueur *joueurReçu = (Joueur*)malloc(sizeof(Joueur));  
     
    while (1)
    {          
                if(tableJeu->tour==11)
                {
                    receiveTable(sock,tableJeu);
                    afficheTable(tableJeu);    
                }
                char receivedBuffer[sizeof(Joueur)];
                recv(sock, receivedBuffer, sizeof(Joueur), 0);
                deserializeJoueur(receivedBuffer, sizeof(Joueur), joueurReçu);
                trierPaquet(joueurReçu);
                printf("\033[1;34mVotre score est : %d\033[0m\n\n",joueurReçu->points);
                if(joueurReçu->gagné!=0)
                {
                    break;
                }
                strcpy(joueurReçu->name,nom);
                printf("Votre paquet :\n\n");
                afficherPaquet(joueurReçu);
                printf("\n\n");               
                joueurReçu->carteJouet = placerCarteRobot(tableJeu,joueurReçu);
                strcpy(joueurReçu->carteJouet.name,joueurReçu->name);
                char serializedBuffer[sizeof(Joueur)];
                serializeJoueur(joueurReçu, serializedBuffer, sizeof(Joueur));
                send(sock, serializedBuffer, sizeof(Joueur), 0);
                receiveTable(sock,tableJeu);
                afficheTable(tableJeu);             
    } 

    if(joueurReçu->gagné==-1)
    {
        printf("\033[31;1mVous avez perdu\033[0m\n");
    }
    else
    {
        printf("\033[1;32mVous avez gagné, félicitations\033[0m\n");
    }
   
    
    close(sock); 

    return 0;
}
