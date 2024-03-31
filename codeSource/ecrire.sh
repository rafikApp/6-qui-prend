#!/bin/bash

# Validation des arguments
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 manche joueur1 points1 [joueur2 points2 ...]"
    exit 1
fi

# Variables
filename="resultats.txt"
tex_filename="resultats.tex"
pdf_filename="resultats.pdf"
#pdf_output="/home1/rr314702/Desktop/Mardi12/SR (1) (1) (2)/test.pdf"
score_file="/tmp/score.txt"

# Récupérer la manche
manche=$1
shift

# Créer un fichier texte ou ajouter au fichier existant
echo "Manche $manche" >> "$filename"

# Parcourir les joueurs et points et les écrire dans le fichier
while [ "$#" -gt 1 ]; do
    joueur="$1"
    points="$2"
    echo "$joueur: $points points" >> "$filename"
    shift 2
done
source toPdf.sh resultats.txt

