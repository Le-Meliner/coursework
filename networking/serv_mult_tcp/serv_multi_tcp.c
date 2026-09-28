// @TITLE : Serveur d'echo TCP multithreadé
// @BRIEF : Répète en majuscules ce qu'il reçoit.
//          La connexion s'arrête après réception de "bye".
// @AUTHOR : Ph Lefebvre - ENSICAEN et modifié par Axel Le Meliner


#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#define PAQUET_LEN 2000

void *main_of_thread(void *p)
{
    unsigned char paquet[PAQUET_LEN];
    int lgr;

    int sdLocal = *((int *)p);
    free(p);

    do {
        lgr = recv(sdLocal, paquet, PAQUET_LEN - 1, 0);

        if (lgr == 0) {
            printf("Déconnexion par le client !\n");
            break;
        }

        if (lgr < 0) {
            perror("pb recv");
            break;
        }

        paquet[lgr] = '\0';

        printf("Paquet reçu (%d octets) : %s\n", lgr, paquet);

        for (int i = 0; i < lgr; i++) {
            paquet[i] = toupper(paquet[i]);
        }

        if (send(sdLocal, paquet, lgr, 0) < 0) {
            perror("pb send");
            break;
        }

    } while (strncmp((char *)paquet, "BYE", 3) != 0);

    printf("Mon client s'est déconnecté.\n");

    close(sdLocal);

    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    int er;
    int s;
    int sd;
    int ret;
    pthread_t th1;

    struct sockaddr_in adBind, adFrom;

    unsigned int ladd = sizeof(struct sockaddr_in);
    int port;

    if (argc != 2) {
        printf("Erreur argument : ./serveur no_port\n");
        exit(EXIT_FAILURE);
    }

    sscanf(argv[1], "%d", &port);

    // Création de la socket TCP
    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("ERREUR socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    adBind.sin_family = AF_INET;
    adBind.sin_port = htons(port);
    adBind.sin_addr.s_addr = INADDR_ANY;

    // Association de la socket au port
    er = bind(s, (struct sockaddr *)&adBind, sizeof(struct sockaddr_in));

    if (er < 0) {
        perror("bind");
        close(s);
        exit(EXIT_FAILURE);
    }

    // Mise en écoute
    er = listen(s, 1);

    if (er < 0) {
        perror("listen");
        close(s);
        exit(EXIT_FAILURE);
    }

    printf("Serveur en écoute sur le port %d...\n", port);

    while (1) {
        sd = accept(s, (struct sockaddr *)&adFrom, &ladd);

        if (sd < 0) {
            perror("accept");
            continue;
        }

        printf("Client connecté depuis %s\n",
               inet_ntoa(adFrom.sin_addr));

        // Allocation mémoire pour transmettre la socket au thread
        int *p_sd = malloc(sizeof(int));

        if (p_sd == NULL) {
            perror("malloc");
            close(sd);
            continue;
        }

        *p_sd = sd;

        ret = pthread_create(&th1, NULL, main_of_thread, p_sd);

        if (ret != 0) {
            perror("Pb creation du thread");
            free(p_sd);
            close(sd);
            continue;
        }

        // Le thread sera automatiquement nettoyé à sa terminaison
        pthread_detach(th1);
    }

    close(s);

    return 0;
}
