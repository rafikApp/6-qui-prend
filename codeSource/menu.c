#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

#define MAX_FILES 100

int main() {
    DIR *directory;
    struct dirent *entry;
    char files[MAX_FILES][256];
    int count = 0;

    // Ouvrir le répertoire courant
    directory = opendir(".");
    
    if (directory == NULL) {
        perror("Erreur lors de l'ouverture du répertoire");
        return 1;
    }

    // Lire tous les fichiers avec l'extension .c
    while ((entry = readdir(directory)) != NULL) {
        if (strstr(entry->d_name, ".c") != NULL) {
            strncpy(files[count], entry->d_name, sizeof(files[count]) - 1);
            files[count][sizeof(files[count]) - 1] = '\0';
            count++;
        }
    }

    closedir(directory);

    // Afficher le menu
    printf("Faites votre choix:\n");
    printf("Pour lancer le serveur tapez             1 \n");
    printf("Pour créer un joueur tapez               2 \n");
    printf("Pour créer un robot tapez                3 \n");
    printf("Pour afficher les statistiques tapez     4 \n");

    
    
    

    // Demander à l'utilisateur de choisir un fichier
    int choix;
    do {
        printf("Entrez un nombre entre 1 et 4 : ");
        scanf("%d", &choix);

        if (choix < 1 || choix > 4) {
            printf("Choix invalide. Veuillez choisir un nombre entre 1 et 4.\n");
        }

    } while (choix < 1 || choix > 4);

    // Construire la commande d'exécution
    char commande[256];
    if(choix==1)
    {
        system("clear");
        system("gcc Gestionnaire.c -o executable1 && ./executable1");
        system("rm executable1");
    }
    else if(choix==2)
    {   
        system("clear");
        system("gcc Joueur.c -o executable2 && ./executable2");
        system("rm executable2 2>/dev/null");
    }
    else if(choix==3)
    {   
        system("clear");
        system("gcc joueurRobot.c -o executable3 && ./executable3");
        system("rm executable3 2>/dev/null");
    }
    else
    {
        system("clear");
        system("less resultats.txt");
    }  
    

    // Exécuter la commande
    int resultat = system(commande);

    if (resultat == -1) {
        perror("Erreur lors de l'exécution de la commande");
        return 1;
    }

    return 0;
}
