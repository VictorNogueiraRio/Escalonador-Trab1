#include <stdio.h>
#include <unistd.h>

int main() {

	FILE *entrada;
	FILE *saida;

	while(1) {

		entrada = fopen("entrada.txt", "r");
		saida = fopen("saida.txt", "a");
		fprintf(saida, "prog4\n");
		fclose(entrada);
		fclose(saida);
		usleep(500000);

	}
	return 0;
}
