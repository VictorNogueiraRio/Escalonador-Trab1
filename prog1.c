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
		fprintf(saida, "prog1\n");
    	usleep(500000);

    }
}
