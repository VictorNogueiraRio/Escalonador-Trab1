#include <stdio.h>
#include <unistd.h>


int main() {

	char prog2[] = "prog2\n";

	FILE *entrada;
	FILE *saida;

	while(1) {

		entrada = fopen("entrada.txt", "r");
		saida = fopen("saida.txt", "a");
        fprintf(saida, "%s", prog2);
		fclose(entrada);
		fclose(saida);
        usleep(500000);
	}
}
