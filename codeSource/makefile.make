CC = gcc
CFLAGS = -Wall -g

all:  Gestionnaire Joueur joueurRobot menu

programme1: Gestionnaire.c
	$(CC) $(CFLAGS) -o Gestionnaire Gestionnaire.c

programme2: Joueur.c
	$(CC) $(CFLAGS) -o Joueur Joueur.c

programme3: joueurRobot.c
	$(CC) $(CFLAGS) -o joueurRobot joueurRobot.c

programme: menu.c
	$(CC) $(CFLAGS) -o menu menu.c

clean:
	rm -f Gestionnaire Joueur joueurRobot menu
