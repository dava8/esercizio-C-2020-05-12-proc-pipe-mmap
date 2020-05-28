#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <sys/mman.h>

#include <errno.h>



#define BUF_SIZE 1024

int main(int argc, char * argv[]) {


	char * addr;
	int pfd[2];
	char buf[BUF_SIZE];
	int numRead;

	if (pipe(pfd) == -1) {
		perror("problema con pipe");

		exit(EXIT_FAILURE);
	}
//
//	printf("file handler pipe, estremità di lettura: %d\n", pfd[0]); // dovrebbe valere 3 (se i descrittori 0,1,2 sono utilizzati)
//	printf("file handler pipe, estremità di scrittura: %d\n", pfd[1]); // dovrebbe valere 4


	addr = mmap(NULL,
				3000,
				PROT_READ | PROT_WRITE,
				MAP_SHARED | MAP_ANONYMOUS,
				-1,
				0); // offset nel file

	if (addr == MAP_FAILED) {
		perror("mmap()");
		exit(EXIT_FAILURE);
	}


	switch (fork()) {
		case -1:
			perror("problema con fork");

			exit(EXIT_FAILURE);

		case 0: // processo FIGLIO: leggerà dalla PIPE

		close(pfd[1]);

		char * str=malloc(1);

		printf("CHILD : ciao, inizio a copiare nella mmap\n\n");

			while(1){

				numRead = read(pfd[0], buf, BUF_SIZE);

				if (numRead == -1) {
					perror("errore in read");
					exit(EXIT_FAILURE);
				}



				if (numRead == 0)
					break;

				str=realloc(str,strlen(str)+numRead);
				str=strcat(str,buf);

			}


			memcpy(addr,str,strlen(str));



			printf("CHILD : finito,esco\n\n");

			close(pfd[0]); // chiudiamo l'estremità di lettura della pipe, ora il kernel libera tutta la struttura di pipe


			//printf("ciao\n");

			exit(EXIT_SUCCESS); // fine del processo figlio

		default:

			close(pfd[0]); // chiudiamo l'estremità di lettura della pipe, non ci serve

			char * file_name="/home/utente/git/esercizio-C-2020-04-22-fibo-file/src/fibonacci.txt";

			int fd = open(file_name, O_RDONLY);

			if (fd == -1) {
				perror("open()");
				exit(EXIT_FAILURE);
			}
			while (1){

				numRead = read(fd, buf, BUF_SIZE);

				if(numRead==0){
					printf("\nPARENT: ho finito di leggere e inviare il file!\n\n");
					break;
				}
				write(pfd[1], buf, numRead); // scriviamo sull'estremità di scrittura della pipe
			}
			close(pfd[1]);

			wait(NULL);



			printf("PARENT: stampo ciò che ho ricevuto\n\n");

			for (int i = 0; i < 3000; i++) {
				printf("%c", addr[i]);
			}


			printf("\nPARENT: finito,esco");

			exit(EXIT_SUCCESS);
			}



}
