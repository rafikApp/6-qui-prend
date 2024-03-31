#!/bin/bash

# Vérifier si le nombre d'arguments est correct
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <fichier_texte.txt>"
    exit 1
fi

# Nom du fichier texte en entrée
input_file="$1"

# Vérifier si le fichier existe
if [ ! -e "$input_file" ]; then
    echo "Le fichier $input_file n'existe pas."
    exit 1
fi

# Nom du fichier PDF de sortie
output_pdf="${input_file%.txt}"

# Convertir le fichier texte en fichier PDF avec pdflatex
pdflatex -jobname="$output_pdf" <<EOF > /dev/null 2>&1
\\documentclass{article}
\\usepackage{verbatim}
\\begin{document}
\\verbatiminput{$input_file}
\\end{document}
EOF



# Supprimer les fichiers temporaires générés par pdflatex
rm "${input_file%.txt}.aux" "${input_file%.txt}.log"
