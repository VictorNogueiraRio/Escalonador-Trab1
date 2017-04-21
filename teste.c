#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define OPENMODE O_WRONLY
#define TRUE 1

int main (void) {
	int fifo;
	char c[100];
	char bla[] = "./prog84";

	if ((fifo = open("minhaFifo2", OPENMODE)) < 0) {
		puts ("Erro ao abrir a FIFO para escrita"); 
		return -1; 
	}
    write(fifo,bla,strlen(bla) + 1);
	close (fifo);
	return 0;
}
