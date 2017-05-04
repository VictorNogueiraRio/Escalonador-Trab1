#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {
        while(1) {
                printf("prio1\n");
		usleep(500000);
        }
}

