#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {
	while(1) {
		char prog1[] = "prog1\n";
         	write(1,prog1,sizeof(prog1) + 1);
            	usleep(500000);
       	}
}
