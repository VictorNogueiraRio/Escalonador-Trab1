#include <stdio.h>
#include <unistd.h>

int main() {

	FILE *entrada = fopen("entrada.txt", "r");
	FILE *saida = fopen("saida.txt", "a");

	while(1) {

		fprintf(saida, "prog6\n");
		usleep(500000);

	}
	
	return 0;
}
