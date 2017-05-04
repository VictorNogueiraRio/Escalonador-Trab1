#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {
	char string[10];
	FILE *entrada;
	FILE *saida;

	while(1) {

		entrada = fopen("entrada.txt", "r");
		saida = fopen("saida.txt", "a");
		fscanf(entrada, "%s", string);
        fprintf(saida, "p1 - %s", string);
		fclose(entrada);
		fclose(saida);
		usleep(500000);
    }
}
