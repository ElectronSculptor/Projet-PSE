// Auteurs : Gevorg ISHKHANYAN et Gaetan HOULLIER
// Date de derniere modification : 09/06/2024




// ----------INCLUDES---------- //

#include "pse.h"
#include "erreur.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// ----------DEFINES---------- //

#define CMD "client"
#define LIGNE_MAX 160



// ----------FONCTIONS---------- //


int lireLigne(int fd, char *buffer) {
    int n = read(fd, buffer, LIGNE_MAX);
    if (n < 0) erreur_IO("lireLigne");
    buffer[n] = '\0';
    return n;
}

int ecrireLigne(int fd, char *buffer) {
    int n = write(fd, buffer, strlen(buffer));
    if (n < 0) erreur_IO("ecrireLigne");
    return n;
}


// ----------MAIN---------- //


int main(int argc, char *argv[]) {
    int sock, ret;
    struct sockaddr_in *adrServ;
    char buffer[LIGNE_MAX];

    if (argc != 3)
        erreur("usage: %s machine port\n", argv[0]);

    printf("%s: creating a socket\n", CMD);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        erreur_IO("socket");

    printf("%s: DNS resolving for %s, port %s\n", CMD, argv[1], argv[2]);
    adrServ = resolv(argv[1], argv[2]);
    if (adrServ == NULL)
        erreur("adresse %s port %s inconnus\n", argv[1], argv[2]);

    printf("%s: adr %s, port %hu\n", CMD,
           stringIP(ntohl(adrServ->sin_addr.s_addr)),
           ntohs(adrServ->sin_port));

    printf("%s: connecting the socket\n", CMD);
    ret = connect(sock, (struct sockaddr *)adrServ, sizeof(struct sockaddr_in));
    if (ret < 0)
        erreur_IO("connect");

    // Remplissage des grilles
    for (int i = 0; i < 3; i++) {
        lireLigne(sock, buffer);
        printf("%s\n", buffer);
        if (fgets(buffer, LIGNE_MAX, stdin) == NULL) {
            erreur("saisie fin de fichier\n");
        }
        ecrireLigne(sock, buffer);

        if (strncmp(buffer, "Player", 6) == 0 && strstr(buffer, "wins") != NULL)
            break;
    }
    lireLigne(sock, buffer);
    printf("%s\n", buffer);
    printf("Debut du jeu\n");
    
    //Le jeu commence 
    while (1) {
        lireLigne(sock, buffer);
        if (!strcmp(buffer, "Defender")) {
            printf("You are defending\n");
              // Bloquer l'écriture
            lireLigne(sock, buffer);
            printf("%s\n", buffer);
        } else {
            printf("%s\n", buffer);
             if (fgets(buffer, LIGNE_MAX, stdin) == NULL) {
                erreur("saisie fin de fichier\n");
            }
            ecrireLigne(sock, buffer);
            lireLigne(sock, buffer);
            printf("%s\n", buffer);
        }
    }

    if (close(sock) == -1)
        erreur_IO("close socket");

    exit(EXIT_SUCCESS);
}


