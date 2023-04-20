#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#define SIZE_BUF 50

bool letraYaUsada(char letra, char usadas[], char palabra[]){
	for (int i = 0; i < strlen(usadas); i++){
		if(letra == usadas[i]) return true;
	}
	for (int i = 0; i < strlen(palabra); i++){
		if(letra == palabra[i]) return true;
	}
	return false;
}

void dibujarAhorcado(char dibujos[10][70],int vidas){ //Imprimir los dibujos por consola
    printf("%s", dibujos[vidas]);
}

void rellenarDibujoAhorcado(char dibujos[10][70]){ //LLenar la tabla de dibujos
    for (int i = 0;i <=10;i++){
        bzero(dibujos[i], 70); //Cada dibujo ocupan cerca de 70 caracteres. Empleo esta funcion para que se printee el dibujo sin problemas
    }
    strcpy(dibujos[0],"  +---+\n  |   |\n  O   |\n /|\\  |\n / \\  |\n      |\n=========\n\n");
    strcpy(dibujos[1],"  +---+\n  |   |\n  O   |\n /|\\  |\n /    |\n      |\n=========\n\n");
    strcpy(dibujos[2],"  +---+\n  |   |\n  O   |\n /|\\  |\n      |\n      |\n=========\n\n");
    strcpy(dibujos[3],"  +---+\n  |   |\n  O   |\n /|   |\n      |\n      |\n=========\n\n");
    strcpy(dibujos[4],"  +---+\n  |   |\n  O   |\n /    |\n      |\n      |\n=========\n\n");
    strcpy(dibujos[5],"  +---+\n  |   |\n  O   |\n      |\n      |\n      |\n=========\n\n");
    strcpy(dibujos[6],"  +---+\n  |   |\n      |\n      |\n      |\n      |\n=========\n\n");
    strcpy(dibujos[7],"  +---+\n      |\n      |\n      |\n      |\n      |\n=========\n\n");
    strcpy(dibujos[8],"       \n      |\n      |\n      |\n      |\n      |\n=========\n\n");
    strcpy(dibujos[9],"       \n       \n       \n       \n       \n       \n=========\n\n");
    strcpy(dibujos[10],"       \n       \n       \n       \n       \n       \n         \n\n");
}
    
int main(int argc, char *argv[]) {
	int socket1, leidos;
	struct sockaddr_in servidor_dir;
	system("clear");

	if (argc != 3){
		printf("ERROR! El numero de argumentos escritos en consola deben ser 3\n");
		exit(0);
	}
	//Rellenamos la tabla de dibujos
	char dibujosAhorcado[10][70];
    rellenarDibujoAhorcado(dibujosAhorcado);

	char *dir_ip_server = argv[1];
	int dir_puerto = atoi(argv[2]);

	// Estableciendo Socket
	if ((socket1=socket(PF_INET,SOCK_STREAM,0)) == -1) {
		perror("Error de Socket\n");
		exit(EXIT_FAILURE);
	}

	// Definiendo Servidor -> Ip, Puerto
	servidor_dir.sin_family = AF_INET;
	servidor_dir.sin_port = htons(dir_puerto);
	servidor_dir.sin_addr.s_addr = inet_addr(dir_ip_server);
	memset(&(servidor_dir.sin_zero), '\0', 8);

	// Estableciendo Conexion
	if (connect(socket1, (struct sockaddr *) &servidor_dir,
	sizeof(struct sockaddr_in))  == -1) {
		perror("Error de Connect()\n");
		exit(EXIT_FAILURE);
	}


	// Abrir el fichero para lectura y escritura
	FILE *socket_fd = fdopen(socket1, "a+");
	char lector_socket[SIZE_BUF];
	char escritor_socket[SIZE_BUF];
	char seguirJugando;
	// Ver si el servidor acepta la conexion
	fgets(lector_socket, SIZE_BUF, socket_fd);
	if (strcmp(lector_socket, "HOLA\n") != 0){
		perror("El servidor no acepta nuevos clientes");
		fclose(socket_fd);
		exit(EXIT_SUCCESS);
	}

	// Enviar el nombre deseado al servidor
	printf("Hola, introduce tu nombre: ");
	explicit_bzero(escritor_socket, SIZE_BUF);
	scanf("%s", escritor_socket);
	if (fprintf(socket_fd, "%s\n", escritor_socket) < 0){
			perror("Error al escribir en el servidor\n");
			fclose(socket_fd);
			exit(EXIT_FAILURE);
	}

	// Respuesta del servidor al cambio de nombre
	fgets(lector_socket, SIZE_BUF, socket_fd);
	while (strcmp(lector_socket, "OK\n") != 0){
		perror("Introduce un nombre valido:\n");
		scanf("%s", escritor_socket);
		if (fprintf(socket_fd, "%s\n", escritor_socket) < 0){
				perror("Error al escribir en el servidor\n");
				fclose(socket_fd);
				exit(EXIT_FAILURE);
		}
	}
	printf("Nombre cambiado a %s satisfactoriamente\n", escritor_socket);
	fflush(stdout);

	// Si el servidor esta esperando a otro usuario se le informa y espera
	fgets(lector_socket, SIZE_BUF, socket_fd);
	if (strcmp(lector_socket, "ESPERA\n") == 0){
		printf("Esperando a otro jugador\n");
		fflush(stdout);
		fgets(lector_socket, SIZE_BUF, socket_fd);
	}

	// Recibir el nombre del rival del servidor
	if (strncmp(lector_socket, "JUEGAS CON ", strlen("JUEGAS CON ")) != 0){
		perror("El servidor no ha enviado lo esperado\n");
		fclose(socket_fd);
		exit(EXIT_FAILURE);
	}
	while(strncmp(lector_socket, "JUEGAS CON ", strlen("JUEGAS CON ")) == 0){
		printf("%s\n", lector_socket);
		fgets(lector_socket, SIZE_BUF, socket_fd);
	}
	if (fprintf(socket_fd, "OK\n") < 0){
			perror("Error al escribir en el servidor\n");
			fclose(socket_fd);
			exit(EXIT_FAILURE);
	}
	do{
		// Empieza la partida. Cuando el serivor indique es mi turno
		int vidas;
		char palabra[SIZE_BUF], usadas[64], letra;
		bzero(usadas, 64);
		usadas[0] = '\0';
		printf("Esperando turno...\n");
		fgets(lector_socket, SIZE_BUF, socket_fd);
		while (strncmp(lector_socket, "ENHORABUENA", strlen("ENHORABUENA")) != 0 && strcmp(lector_socket, "DERROTA\n") != 0
			&& strncmp(lector_socket, "GANA", strlen("GANA")) != 0){
			if (strncmp(lector_socket, "TURNO", strlen("TURNO")) == 0 || strncmp(lector_socket, "CORRECTO", strlen("CORRECTO")) == 0){
				strtok(lector_socket, " ");					// Saltar "TURNO" o "CORRECTO"
				strcpy(palabra, strtok(NULL, " "));			// Asignar palabra
				vidas = atoi(strtok(NULL, "\n"));			// Asignar vidas
				if (strlen(usadas) > 0){
					printf("Las letras usadas son: %s\n", usadas);
				}
				printf("La palabra es %s\nQuedan %d vidas\n", palabra, vidas);
				dibujarAhorcado(dibujosAhorcado,vidas);
				printf("\nIntroduce una letra: ");  
				scanf(" %c", &letra);
				// Si no es una letra valida o ya se ha usado
				while(letra < 'a' || letra > 'z' || letraYaUsada(letra, usadas, palabra)){
					printf("Introduce una letra valida y que no se haya usado ya: ");
					scanf(" %c", &letra);
				}
				if (fprintf(socket_fd, "%c\n", letra) < 0){
					perror("Error al escribir en el servidor\n");
					fclose(socket_fd);
					exit(EXIT_FAILURE);
				}
			}else if(strncmp(lector_socket, "RIVAL", strlen("RIVAL")) == 0){
				printf("%s\n", lector_socket);	// Indicar lo que ha hecho el rival
				if (strncmp(lector_socket, "RIVAL FALLA", strlen("RIVAL FALLA")) == 0){	// Si un rival falla se anade la letra usada a usadas
					char aux[3];
					sprintf(aux, "%c ", lector_socket[strlen("RIVAL FALLA ")]);
					strcat(usadas, aux);
				}
			}else if (strncmp(lector_socket, "FALLO", strlen("FALLO")) == 0){	// Si yo fallo se añade la letra fallada a usadas
				char aux[3];
				sprintf(aux, "%c ", lector_socket[strlen("FALLO ")]);
				strcat(usadas, aux);
			}
			printf("Esperando turno...\n");
			fgets(lector_socket, SIZE_BUF, socket_fd);
		}

		if(strncmp(lector_socket, "ENHORABUENA", strlen("ENHORABUENA")) == 0){
			lector_socket[strlen(lector_socket) - 1] = '\0';
			printf("%s. Has ganado\n", lector_socket);
		}else if(strcmp(lector_socket, "DERROTA\n") == 0){
			lector_socket[strlen(lector_socket) - 1] = '\0';
			dibujarAhorcado(dibujosAhorcado,0);
			printf("%s. Habeis perdido\n", lector_socket);
		}else{
			printf("%s. El rival gana\n", lector_socket);
		}
		printf("¿Deseas seguir Jugando? (S/N): ");
		scanf(" %c", &seguirJugando);
		if(seguirJugando == 's' || seguirJugando == 'S'){
			if (fprintf(socket_fd, "SEGUIR\n") < 0){
				perror("Error al escribir en el servidor\n");
				fclose(socket_fd);
				exit(EXIT_FAILURE);
			}
			printf("Esperando a los demas jugadores\n");
			fgets(lector_socket, SIZE_BUF, socket_fd);
			if(strcmp(lector_socket, "NUEVA PARTIDA\n") == 0){
				printf("Iniciando nueva partida\n");
			}else{
				printf("Los demas jugadores han rechazado. No se iniciara una nueva partida\n");
				seguirJugando = 'n';
			}
		}else{
			if (fprintf(socket_fd, "PARAR\n") < 0){
				perror("Error al escribir en el servidor\n");
				fclose(socket_fd);
				exit(EXIT_FAILURE);
			}
		}
	}while (seguirJugando == 's' || seguirJugando == 'S');
	fclose(socket_fd);
	close(socket1);
}
