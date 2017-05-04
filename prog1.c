#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {

	FILE *entrada;
	FILE *saida;

	while(1) {

		entrada = fopen("entrada.txt", "r");
		saida = fopen("saida.txt", "a");
		fprintf(saida, "prog1\n");
		fclose(entrada);
		fclose(saida);
    	usleep(500000);

    }
}
