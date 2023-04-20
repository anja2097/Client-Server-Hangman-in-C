#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>

#define SIZEBUFFER 50

// Dejar de rechazar clientes una vez se termina de jugar
void handler(int signum){
	exit(EXIT_SUCCESS);
}

bool estaLetra(char letra, char palabra[SIZEBUFFER]){
	for (int i = 0; i < strlen(palabra); i++){
		if (letra == palabra[i]) return true;
	}
	return false;
}

// Si hay palabras entre las que elegir elige una y la descarta y sino pide otras 10 y elige de esas
void elegirPalabra (char palabras[10][SIZEBUFFER], int *numeroPalabras, char palabraElegida[SIZEBUFFER]){
	if (*numeroPalabras == 0) {
		char nuevasPalabras[1024];
		char separador;
		explicit_bzero(nuevasPalabras, 1024);
		printf("Vuelva a escribir otras 10 palabras separadas por espacios: \n");
		while (strlen(nuevasPalabras) < 20){		//En el caso de poner 10 palabras de una letra hay 20 letras contando con los espacios
			fgets(nuevasPalabras, 1024, stdin);
		}
		separador = ' ';
		explicit_bzero(palabras[0], SIZEBUFFER);
		strcpy(palabras[0], strtok(nuevasPalabras,&separador));
		for (int i = 1; i<9; i++){
			explicit_bzero(palabras[i], SIZEBUFFER);
			strcpy(palabras[i], strtok(NULL,&separador));
		}
		separador = '\n';
		explicit_bzero(palabras[9], SIZEBUFFER);
		strcpy(palabras[9], strtok(NULL,&separador));
		*numeroPalabras = 10;
		elegirPalabra(palabras, numeroPalabras, palabraElegida);	//Como ahora hay 10 palabras si volvemos a llamar a la funcion elige una palabra
	}
	else if(*numeroPalabras == 1){
		strcpy(palabraElegida,palabras[0]);
		printf("La palabra es %s \n",palabraElegida);
		*numeroPalabras = 0;
	}
	else{
		int indicePalabra = rand() % *numeroPalabras;
		strcpy(palabraElegida,palabras[indicePalabra]);
		strcpy(palabras[indicePalabra], palabras[*numeroPalabras-1]);
		strcpy(palabras[*numeroPalabras-1],palabraElegida);
		printf("La palabra es %s \n",palabraElegida);
		*numeroPalabras--;
    }
}

// Convertir la palabra a una palabra de la misma longitud pero con guiones
void guionizaPalabra(char palabra[SIZEBUFFER], char palabraGuionizada[SIZEBUFFER]){
	explicit_bzero(palabraGuionizada, SIZEBUFFER);
	for (int i = 0; i < strlen(palabra); i++){
		palabraGuionizada[i] = '_';
	}
}

// Comprobar si la letra introducida esta en la palabra y si lo esta ponerla en la palabra guionizada
void comprobarPalabra(char letra, char palabra[SIZEBUFFER], char palabraGuionizada[SIZEBUFFER]){
	for (int i = 0; i < strlen(palabra); i++){
		if (letra == palabra[i]) palabraGuionizada[i] = letra;
	}
}


int main(int argc, char *argv[]) {
	srand(time(NULL));
	system("clear");
	if (argc != 2){
		printf("ERROR! El numero de argumentos escritos en consola deben ser 2\n");
		exit(0);
	}
	//Guardar las palabras pasadas por consola
	char palabras[1000];
	char palabraElegida[50];
	int numeroPalabras = 10;
	printf("Escriba 10 palabras separadas por espacios: \n");
	fgets(palabras, 1000, stdin);
	for (int i = 0; i < strlen(palabras); i++){
		if (palabras[i] > 'A' && palabras[i] < 'Z'){
			palabras[i] = (char)tolower((int) palabras[i]);
		}
	}
	printf("\nLas palabras con las que se va a jugar son: %s \n",palabras);
	char guardarPalabras[10][50];
	strcpy(guardarPalabras[0], strtok(palabras, " "));
	for (int i = 1; i<9; i++){
		strcpy(guardarPalabras[i], strtok(NULL, " "));
	}
	strcpy(guardarPalabras[9], strtok(NULL, "\n"));


	// para manejar n jugadores
    int numeroJugadores = 2;
    printf("Escriba el numero de jugadores (entre 2 y 5): \n");
    scanf("%d",&numeroJugadores);
    while(numeroJugadores < 2 || numeroJugadores > 5){
        printf("El numero escrito es incorrecto. Vuelva a introducir el numero de jugadores: \n");
        scanf("%d",&numeroJugadores);
    }
	char mensajes[numeroJugadores][SIZEBUFFER], nombres[numeroJugadores][SIZEBUFFER];
	FILE * descriptores_sockets[numeroJugadores];

	// conexiones
	int sockfd, new_fd1; // Escuchar sobre sock_fd, nuevas conexiones sobre new_fd
	struct sockaddr_in my_addr; // información sobre mi dirección
	struct sockaddr_in their_addr; // información sobre la dirección del cliente
	int sin_size;
	int yes=1;
	int dir_puerto = atoi(argv[1]);
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	my_addr.sin_family = AF_INET; // Ordenación de bytes de la máquina
	my_addr.sin_port = htons(dir_puerto); // short, Ordenación de bytes de la red
	my_addr.sin_addr.s_addr = INADDR_ANY; // Rellenar con mi dirección IP
	memset(&(my_addr.sin_zero), '\0', 8); // Poner a cero el resto de la estructura
	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}
	if (listen(sockfd, 1) == -1) {
		perror("listen");
		exit(1);
	}
	sin_size = sizeof(struct sockaddr_in);
	printf("Esperando a NUEVOS CLIENTES\n");

	// Recepcion clientes
	for (int i = 0; i < numeroJugadores; i++){
		if ((new_fd1 = accept(sockfd, (struct sockaddr *)&their_addr,&sin_size)) == -1) {
			perror("Error recibiendo cliente");
			exit(EXIT_FAILURE);
		}
		descriptores_sockets[i] = fdopen(new_fd1, "a+");
		fprintf(descriptores_sockets[i] , "HOLA\n");

		// Eleccion del nombre
		fgets(mensajes[i], SIZEBUFFER, descriptores_sockets[i] );
		while (strlen(mensajes[i]) == 0){
			fprintf(descriptores_sockets[i], "NOK\n");
			fgets(mensajes[i], SIZEBUFFER, descriptores_sockets[i]);
		}

		strcpy(nombres[i], mensajes[i]);
		nombres[i][strlen(nombres[i]) - 1] = '\0';
		fprintf(descriptores_sockets[i], "OK\n");
		printf("Se ha conectado el jugador %d llamado %s\n", i+1 ,nombres[i]);
		// Si aun faltan jugadores
		if (i < numeroJugadores - 1) fprintf(descriptores_sockets[i], "ESPERA\n");
	}
	//Informar con quien juega cada uno
	for (int i = 0; i < numeroJugadores; i++){
		for (int j = 0; j < numeroJugadores; j++){
			if (i != j)fprintf(descriptores_sockets[i],"JUEGAS CON %s\n",nombres[j]);
		}
		fprintf(descriptores_sockets[i],"FIN PRESENTACIONES\n");
	}

	// Leer las respuestas de los jugadores
	for (int i = 0; i < numeroJugadores; i++){
		fgets(mensajes[i], SIZEBUFFER, descriptores_sockets[i]);
		printf("%s dice %s", nombres[i], mensajes[i]);
	}

	printf("\nComienza la partida\n");
	int pid = fork();
	if (pid == 0){ //El hijo se encarga de manejar la partida
		bool siguenJugando = true;
		do{
			char ganador[SIZEBUFFER], palabraElegida[SIZEBUFFER], palabraGuionizada[SIZEBUFFER], letra = '\0';
			int vidas = 10;
			bzero(palabraElegida, SIZEBUFFER);
			elegirPalabra(guardarPalabras, &numeroPalabras, palabraElegida);
			guionizaPalabra(palabraElegida, palabraGuionizada);

			while (vidas > 0 && strcmp(palabraElegida, palabraGuionizada) != 0){
				for (int i = 0; i < numeroJugadores; i++){
					fprintf(descriptores_sockets[i], "TURNO %s %d\n", palabraGuionizada, vidas);
					printf("TURNO %s %s %d\n", nombres[i],palabraGuionizada, vidas);
					fgets(mensajes[i], SIZEBUFFER, descriptores_sockets[i]);
					letra = mensajes[i][0];
					printf("%s dice la letra %c\n", nombres[i],letra);
					comprobarPalabra(letra, palabraElegida, palabraGuionizada);
					// Mientras acierte y la palabra no este completa
					while(estaLetra(letra, palabraElegida) && strcmp(palabraElegida, palabraGuionizada) != 0){
						// Informar el acierto
						printf("%s acierta\n", nombres[i]);
						fprintf(descriptores_sockets[i], "CORRECTO %s %d\n", palabraGuionizada, vidas);
						for (int j = 0; j < numeroJugadores; j++){
							if (i != j)fprintf(descriptores_sockets[j], "RIVAL ACIERTA %c\n", letra);
						}
						// Seguir adivinando
						fgets(mensajes[i], SIZEBUFFER, descriptores_sockets[i]);
						letra = mensajes[i][0];
						printf("%s dice la letra %c\n",nombres[i], letra);
						comprobarPalabra(letra, palabraElegida, palabraGuionizada);
					}

					if (strcmp(palabraElegida, palabraGuionizada) == 0){ // Si ha acertado la palabra
						strcpy(ganador, nombres[i]);
						break;
					}else{ // Si ha fallado
						vidas--;
						// Informar el fallo
						printf("%s falla\n", nombres[i]);
						fprintf(descriptores_sockets[i], "FALLO %c %d\n", letra, vidas);
						for (int j = 0; j < numeroJugadores; j++){
							if (i != j)fprintf(descriptores_sockets[j], "RIVAL FALLA %c\n", letra);
						}
						if (vidas == 0) break;
					}
				}
				if(vidas == 0){ // Si sale del for porque se han acabado las vidas
					for (int i = 0; i < numeroJugadores; i++){
						fprintf(descriptores_sockets[i], "DERROTA\n");
					}
				}else if (strcmp(palabraElegida, palabraGuionizada) == 0){// si hay un ganador
					for (int i = 0; i < numeroJugadores; i++){
						if (strcmp(ganador, nombres[i]) == 0){
							fprintf(descriptores_sockets[i], "ENHORABUENA %s %d\n", palabraGuionizada, vidas);
							for (int j = 0; j < numeroJugadores; j++){
								if (i != j)fprintf(descriptores_sockets[j], "GANA %s\n", nombres[i]);
							}
							break; // Una vez informado el ganador sale del for ya que no va a haber mas ganadores
						}
					}
				}
			}

			// ver si todos quieren seguir jugando
			bool quiereSeguir[numeroJugadores];
			for (int i = 0; i < numeroJugadores; i++){
				fgets(mensajes[i], SIZEBUFFER, descriptores_sockets[i]);
				if (strcmp(mensajes[i], "SEGUIR\n") != 0){
					siguenJugando = false;
					quiereSeguir[i] = false;
				}else quiereSeguir[i] = true;
			}
			if (siguenJugando){
				for (int i = 0; i < numeroJugadores; i++){
					fprintf(descriptores_sockets[i], "NUEVA PARTIDA\n");
				}
			}else{
				for (int i = 0; i < numeroJugadores; i++){
					if (quiereSeguir[i]) fprintf(descriptores_sockets[i], "NO HAY NUEVA PARTIDA\n");
				}
			}
		}while(siguenJugando);
		for (int i = 0; i < numeroJugadores; i++){
			fclose(descriptores_sockets[i]);
		}
		close(sockfd);
		exit(EXIT_SUCCESS);

	}else if(pid > 0){ //El padre se dedica a rechazar nuevas conexiones
		signal(SIGCHLD, handler);	//Espera que el hijo termine para terminar el programa
		while(true){
			int new_fd3;
			if ((new_fd3 = accept(sockfd, (struct sockaddr *)&their_addr,&sin_size)) == -1) {
				perror("Error recibiendo cliente 3");
				continue;
			}
			FILE * descriptor_socket3 = fdopen(new_fd3, "a+");
			fprintf(descriptor_socket3,"OCUPADO\n");
			close(new_fd3);
		}
	}

}
