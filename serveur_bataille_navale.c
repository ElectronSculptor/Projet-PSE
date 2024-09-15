// Auteurs : Gevorg ISHKHANYAN et Gaetan HOULLIER
// Date de derniere modification : 09/06/2024




// ----------INCLUDES---------- //

#include "pse.h"
#include "erreur.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>


// ----------DEFINES---------- //

#define CMD "serveur"
#define GRID_SIZE 10
#define LIGNE_MAX 160


typedef struct Player {
    int socket_fd;
    int other_fd;
    int player_id;
} Player;
int player_turn;
char grid1[GRID_SIZE][GRID_SIZE], grid2[GRID_SIZE][GRID_SIZE];


// ----------PROTOTYPES---------- //

void initGrid(char grid[][GRID_SIZE]);
void printGrid(char grid[][GRID_SIZE]);
int lireLigne(int fd, char *buffer);
int ecrireLigne(int fd,char *buffer);
void processAttack(int attacker, int defender, char grid[][GRID_SIZE]);
void *sessionJeu(void *args);
void *sessionRemplissageGrille(void *args);



// ----------MAIN---------- //


int main(int argc, char *argv[]) {
    short port;
    int ecoute, canal1, canal2, ret;
    struct sockaddr_in adrEcoute, adrClient;
    unsigned int lgAdrClient;

    if (argc != 2)
        erreur("usage: %s port\n", argv[0]);

    port = (short)atoi(argv[1]);

    printf("%s: creating a socket\n", CMD);
    ecoute = socket(AF_INET, SOCK_STREAM, 0);
    if (ecoute < 0)
        erreur_IO("socket");

    adrEcoute.sin_family = AF_INET;
    adrEcoute.sin_addr.s_addr = INADDR_ANY;
    adrEcoute.sin_port = htons(port);
    printf("%s: binding to INADDR_ANY address on port %d\n", CMD, port);
    ret = bind(ecoute, (struct sockaddr *)&adrEcoute, sizeof(adrEcoute));
    if (ret < 0)
        erreur_IO("bind");

    printf("%s: listening to socket\n", CMD);
    ret = listen(ecoute, 5);
    if (ret < 0)
        erreur_IO("listen");

    printf("%s: waiting for player 1\n", CMD);
    canal1 = accept(ecoute, (struct sockaddr *)&adrClient, &lgAdrClient);
    if (canal1 < 0)
        erreur_IO("accept");
    printf("%s: player 1 connected\n", CMD);

    printf("%s: waiting for player 2\n", CMD);
    canal2 = accept(ecoute, (struct sockaddr *)&adrClient, &lgAdrClient);
    if (canal2 < 0)
        erreur_IO("accept");
    printf("%s: player 2 connected\n", CMD);

    Player p1;
    p1.socket_fd = canal1;
    p1.other_fd = canal2;
    p1.player_id = 1;
    Player p2;
    p2.socket_fd = canal2;
    p2.other_fd = canal1;
    p2.player_id = 2;
    
    player_turn = 1;
    
    
    initGrid(grid1);
    initGrid(grid2);
    
    pthread_t thread1, thread2;
    // Appeler les threads de remplissage
    pthread_create(&thread1, NULL, sessionRemplissageGrille, &p1);
    pthread_create(&thread2, NULL, sessionRemplissageGrille, &p2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    // Appeler les threads de jeu
    pthread_create(&thread1, NULL, sessionJeu, &p1);
    pthread_create(&thread2, NULL, sessionJeu, &p2);
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    
    
    close(canal1);
    close(canal2);

    if (close(ecoute) == -1)
        erreur_IO("fermeture ecoute");

    exit(EXIT_SUCCESS);
}




// ----------FONCTIONS---------- //


//Initialisation de grille
void initGrid(char grid[][GRID_SIZE]) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = '.';
        }
    }
}

//Affichage de la grille
void printGrid(char grid[][GRID_SIZE]) {
    printf("  ");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%d ", i);
    }
    printf("\n");
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%d ", i);
        for (int j = 0; j < GRID_SIZE; j++) {
            printf("%c ", grid[i][j]);
        }
        printf("\n");
    }
}

//Lecture sur un buffer
int lireLigne(int fd, char *buffer) {
    int n = read(fd, buffer, LIGNE_MAX);
    if (n < 0) erreur_IO("lireLigne");
    buffer[n] = '\0';
    return n;
}

//Ecriture sur un buffer
int ecrireLigne(int fd, char *buffer) {
    int n = write(fd, buffer, strlen(buffer));
    if (n < 0) erreur_IO("ecrireLigne");
    return n;
}

//Session de remplissage des Grilles au debut de jeu
void *sessionRemplissageGrille(void *args) {
    Player *p = (Player *)args;
    int row, col;
    char *buffer = malloc(sizeof(char) * LIGNE_MAX);
    
        if (p->player_id == 1) {
            for (int i = 0 ; i< 3; i++){
            ecrireLigne(p->socket_fd, "Client 1 place your ship (format: row col): ");
            lireLigne(p->socket_fd, buffer);
            sscanf(buffer, "%d %d", &row, &col);
            grid1[row][col] = 'S';
            ecrireLigne(p->socket_fd, "Ship correctly placed !\n");
            }
            printf("GRILLE 1 :\n");
            printGrid(grid1);
        } else {
            for (int i = 0 ; i< 3; i++) {
            ecrireLigne(p->socket_fd, "Client 2 place your ship (format: row col): ");
            lireLigne(p->socket_fd, buffer);
            sscanf(buffer, "%d %d", &row, &col);
            grid2[row][col] = 'S';
            ecrireLigne(p->socket_fd, "Ship correctly placed !\n");
            }
            printf("GRILLE 2 :\n");
            printGrid(grid2);
        }
    free(buffer);
}


//Session de jeu qui simule les attaques et met a jour la partie
void *sessionJeu(void *args) {
    Player *p = (Player *)args;
    char buffer[LIGNE_MAX];
    
    while (1) {
        if (player_turn == p->player_id) {
            char (*defenderGrid)[GRID_SIZE] = (player_turn == 1) ? grid2 : grid1;

            snprintf(buffer, LIGNE_MAX, "Player %d, enter coordinates to attack (row col): ", player_turn);
            ecrireLigne(p->socket_fd, buffer);
            lireLigne(p->socket_fd, buffer);
            int row, col;
            sscanf(buffer, "%d %d", &row, &col);

            if (defenderGrid[row][col] == 'S') {
                defenderGrid[row][col] = 'X';
                ecrireLigne(p->socket_fd, "Hit!\n");
                ecrireLigne(p->other_fd, "Your ship has been hit!\n");
            } else if (defenderGrid[row][col] == '.') {
                defenderGrid[row][col] = 'O';
                ecrireLigne(p->socket_fd, "Miss!\n");
                ecrireLigne(p->other_fd, "Opponent missed!\n");
            } else {
                ecrireLigne(p->socket_fd, "Already attacked !\n");
                ecrireLigne(p->other_fd, "Your opponent is dumb !\n");
            }
            if (player_turn == 1) {
                printGrid(grid2);
            } else {
                printGrid(grid1);
            }

            // Check if all ships are sunk
            int allSunk = 1;
            for (int i = 0; i < GRID_SIZE; i++) {
                for (int j = 0; j < GRID_SIZE; j++) {
                    if (defenderGrid[i][j] == 'S') {
                        allSunk = 0;
                        break;
                    }
                }
                if (!allSunk) break;
            }
            if (allSunk) {
                snprintf(buffer, LIGNE_MAX, "Player %d wins!\n", player_turn);
                ecrireLigne(p->socket_fd, buffer);
                ecrireLigne(p->other_fd, buffer);
                break;
            }
        
            player_turn = (player_turn == 1) ? 2 : 1;
        } else {
            ecrireLigne(p->socket_fd, "Defender");
            while (player_turn != p->player_id);
        }
    }
}
