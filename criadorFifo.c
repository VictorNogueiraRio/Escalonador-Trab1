#include <stdio.h>
#include <sys/stat.h>

int main (void) { 
	if (mkfifo("minhaFifo", S_IRUSR | S_IWUSR) == 0) {
		puts ("FIFO criada com sucesso");
	} 
    if (mkfifo("minhaFifo2", S_IRUSR | S_IWUSR) == 0) {
        puts ("FIFO criada com sucesso");
    }
    if (mkfifo("minhaFifo3", S_IRUSR | S_IWUSR) == 0) {
        puts ("FIFO criada com sucesso");
    }
} 
