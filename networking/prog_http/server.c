// @TITLE : Serveur d'echo sur le port pass� en argument 
// @BRIEF : R�p�te ce qu'il re�oit et l'affiche. La connexion s'arrete apres reception de bye.
// @AUTHOR : Ph Lefebvre - ENSI de Caen
// A compiler avec l'option -lpthread gcc serv_multi_tcp.c -o serveur -lpthread

#include <errno.h>                 // gestion des erreurs
#include <sys/socket.h>            // gestion pour les socket
#include <sys/types.h>
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <stdio.h>                 // gestion les E/S.
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <pthread.h> 

#define PAQUET_LEN 2000

void extraireNomFichier(char *nom_ressource, const char *req) {
    char tmp[PAQUET_LEN];
    strncpy(tmp, req, PAQUET_LEN-1);
    strtok(tmp, " ");                 
    char *token = strtok(NULL, " "); 
    if (token == NULL) { nom_ressource[0] = '\0'; return; }
    if (token[0] == '/') token++;    
    if (token[0] == '\0') strcpy(nom_ressource, "index.html");
    else strcpy(nom_ressource, token);
}

const char *getMimeType(const char *nom_fichier) {
    char *ext = strrchr(nom_fichier, '.');
    if (ext == NULL)               return "application/octet-stream";
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".htm")  == 0) return "text/html";
    if (strcmp(ext, ".jpg")  == 0) return "image/jpeg";
    if (strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".gif")  == 0) return "image/gif";
    if (strcmp(ext, ".png")  == 0) return "image/png";
    return "application/octet-stream";
}

void * main_of_thread (void *p) {
	unsigned char paquet[PAQUET_LEN];             // paquet recu / envoye

	int lgr ;
	int sdLocal = *( (int *)p ) ; // Attention not thread safe !
	 do {
		lgr = recv(sdLocal, paquet, PAQUET_LEN-1, 0);
        if (lgr == 0) { printf("Déconnexion par le client !\n"); break;}
        if (lgr < 0) { perror("pb recv"); break; }
		paquet[lgr] = '\0';
		printf("Paquet reçu (%d octets) : %s\n", lgr, paquet);
 
		char nom_fichier[PAQUET_LEN];
    	extraireNomFichier(nom_fichier, (char *)paquet);
    	printf("fichier demande : %s\n", nom_fichier);
		FILE *f = fopen(nom_fichier, "rb");
    	if (f == NULL) {
			char entete[] = "HTTP/1.1 404 ERROR\n\n";
			send(sdLocal, entete, strlen(entete), 0);
			printf("404 : fichier %s non trouve\n", nom_fichier);
		} else {
			
			const char *mime = getMimeType(nom_fichier);
			char entete[256];
			sprintf(entete, "HTTP/1.1 200 OK \ncontent-type: %s \n\n", mime);
			send(sdLocal, entete, strlen(entete), 0); // envoi de l'entete HTTP
			unsigned char buf[PAQUET_LEN];
			int n;
			while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
				send(sdLocal, buf, n, 0);
			}
			fclose(f);
        	printf("200 OK : %s [%s]\n", nom_fichier, mime);
    	}
    	break;
	} while ( (strncmp ((char *)paquet, "bye", 3)));      // on compare les 3 premiers octets recus a "bye"
    printf (" Mon client s'est deconnecte.\n"); 
	close (sdLocal);                         // fermeture de la socket de dialogue. Provoque la d�connexion si le buffer de r�ception est vid�
	pthread_exit(0);
}


int main( int argc, char **argv) {
	int er;                   // gestion des erreurs
	int s;                         // socket d'ecoute du port
	int sd;                        // socket de dialogue avec le client.
	pthread_t th1 ;

	struct sockaddr_in adBind, adFrom;    // adresses au format internet
  
	unsigned int ladd=sizeof (struct sockaddr_in);  // longueur de l'adresse de reception
	int port;						// port de connexion

	if (argc !=2 ) {
		printf ("Erreur argument 1 : serveur.exe no_port \n"); exit (-1);
	}
	else  sscanf (argv[1], "%d", &port); 
	
	/* On ouvre la socket  Internet en mode connecte. 6 est le num�ro de TCP , cf. le fichier /etc/protocols*/
	if ((s = socket(AF_INET, SOCK_STREAM, 6)) < 0) {
		perror("\nERREUR socket\n"); exit(-1);
	}
  
  /* On attache la socket au port d'ecoute */
	adBind.sin_family = AF_INET;           // adresses de type internet
	adBind.sin_port = htons (port);        // numero de port du serveur
	adBind.sin_addr.s_addr = INADDR_ANY;   // accepte les connexions de n'importe quelle interface ethernet, wifi...
	er = bind (s, (struct sockaddr *) &adBind, sizeof (struct sockaddr_in));
	if (er < 0) {
		perror ("bind : "); exit(-1);
	}
  
  /* On fixe le nombre maximum de clients simultanes en attente de connexion. Ici 1 seule*/
  /* autorise le syst�me d'exploitation � accepter des connexions */
	er = listen (s, 1);
	if (er < 0) {
		perror ("listen : "); exit(-1);
	}
	
/* D�pile une demande de connexion. Attend si aucune connexion encore n'est encore arriv�e.
 * Cr�e une socket de dialogue si un client se connecte.
 * */  
	while(1) {
		sd = accept( s, (struct sockaddr *) &adFrom, &ladd);             
		printf ("1 client dont l'adresse IP est %s !\n", inet_ntoa(adFrom.sin_addr));
		
		int ret = pthread_create(&th1, NULL, main_of_thread, (void *)&sd);	
		
		if (ret != 0) {
			perror ("Pb creation du thread\n");
			exit(0) ;
		}
	}
}

