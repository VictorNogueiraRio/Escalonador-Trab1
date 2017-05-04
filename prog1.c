#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {

	FILE *entrada = fopen("entrada.txt", "r");
	FILE *saida = fopen("saida.txt", "a");

	while(1) {
		char prog1[] = "prog1\n";
		fprintf(saida, prog1);
    	usleep(500000);

    }
}
