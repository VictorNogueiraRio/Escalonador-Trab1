#include <stdio.h>
#include <unistd.h>


int main() {

	FILE *entrada = fopen("entrada.txt", "r");
	FILE *saida = fopen("/Users/mlfs95/Documents/PUC/SistemasDeComputação/Trab1/Escalonador-Trab1/saida.txt", "a");

    while(1) {

        fprintf(saida, "prog2\n");
        usleep(500000);
	}
}
