#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {
	char string[10];
	FILE *entrada = fopen("entrada.txt", "r");
	FILE *saida = fopen("saida.txt", "a");

    while(1) {
		fscanf(entrada, "%s", string);
        fprintf(saida, "p1 - %s", string);
		usleep(500000);
    }
}
