#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define OPENMODE O_WRONLY
#define TRUE 1

int main (void) {
	int fifo;
	char c[100];
	char bla[] = "r./prog1";
    char bla2[] = "p./prog23";
    char bla3[] = "t./prog32030";
    char bla4[] = "r./prog4";
    char bla5[] = "p./prog52";
    char bla6[] = "t./prog60203";
    char bla7[] = "r./prog7";
    char bla8[] = "p./prog85";
    char bla9[] = "r./prog9";
    char bla10[] = "p./stols1";
	if ((fifo = open("minhaFifo2", OPENMODE)) < 0) {
		puts ("Erro ao abrir a FIFO para escrita"); 
		return -1; 
	}
    write(fifo,bla,strlen(bla) + 1);
    sleep(1);
    write(fifo,bla2,strlen(bla2) + 1);
    sleep(1);
    write(fifo,bla3,strlen(bla3) + 1);
    sleep(1);
    write(fifo,bla4,strlen(bla4) + 1);
    sleep(1);
    write(fifo,bla5,strlen(bla5) + 1);
    sleep(1);
    write(fifo,bla6,strlen(bla6) + 1);
    sleep(1);
    write(fifo,bla7,strlen(bla7) + 1);
    sleep(1);
    write(fifo,bla8,strlen(bla8) + 1);
    sleep(1);
    write(fifo,bla9,strlen(bla9) + 1);
    sleep(1);
    write(fifo,bla10,strlen(bla10) + 1);
    close (fifo);
    return 0;
}
