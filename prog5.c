#include <stdio.h>
#include <unistd.h>

int main() {

	FILE *entrada = fopen("entrada.txt", "r");
	FILE *saida = fopen("saida.txt", "a");
	char c[20];

	while(1) {

		fprintf(saida, "prog5\n");
		usleep(500000);

	}
	
	return 0;
}
