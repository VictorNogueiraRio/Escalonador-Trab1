#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define OPENMODE O_WRONLY
#define TRUE 1

void executarRoundRobin();
void executarPrioridade(int pri);
void executarRealTime(int seg, int dur);
void intToChar(int target,char **converted);

int main(){

	char nome[15], arg1[15], arg2[15], numero[4];
	int num1, num2, n,fifo;
	FILE * exec;
    
    if ((fifo = open("minhaFifo2", OPENMODE)) < 0) {
        puts ("Erro ao abrir a FIFO para escrita");
        return -1;
    }

	exec = fopen ("exec.txt", "r");

	fscanf(exec, "%s", nome);

	while (1){

		n = fscanf(exec, "%s %s", &(nome[3]), arg1);
        nome[1] = '.';
        nome[2] = '/';
		if (n==1){
			printf("%s - ultimo roundrobin\n", nome);
            sleep(1);
			break;
		}
            
		else if (arg1[0]=='E'){
            nome[0] = 'r';
            nome[8] = 0;
            write(fifo,nome,9);
			executarRoundRobin();
			sleep(1);
		}

		else if (arg1[0]=='P'){
            nome[0] = 'p';
			//printf("%s - prioridade\n", nome);

			numero[0]=arg1[3];
			numero[1]='\0';
            nome[8] = arg1[3];
            nome[9] = 0;
            write(fifo,nome,10);
			num1=atof(numero);
			sleep(1);

			if (fscanf(exec, "%s", nome)!=1){

				break;
			}
		}

		else if (arg1[0]=='I'){
            nome[0] = 't';
			//printf("%s - Real-time\n", nome);

			numero[0]=arg1[2];
			numero[1]=arg1[3];
			num1=atof(numero);
            char *ini = (char *) malloc(3);
            intToChar(num1,&ini);
            nome[8] = ini[0];
            nome[9] = ini[1];
			
            fscanf(exec, "%s", arg2);

			numero[0]=arg2[2];
			numero[1]=arg2[3];
            
			num2=atof(numero);
            
            char *fim = (char *) malloc(3);
            
            num2 += num1;
            intToChar(num2,&fim);
            nome[10] = fim[0];
            nome[11] = fim[1];
            nome[12] = 0;
            write(fifo,nome,13);
            //printf("nome: %s\n",nome);

			sleep(1);

			if (fscanf(exec, "%s", nome)!=1){

				break;
			}
		}
	}

	return 0;
}

void executarRoundRobin(){

}

void intToChar(int target,char **converted) {
    int i = 0;
    while(target/10 > 0) {
        (*converted)[i] = target/10 + 48;
        target %= 10;
        i++;
    }
    if(i == 0) {
        (*converted)[0] = 48;
        i++;
    }
    (*converted)[i] = target + 48;
    (*converted)[i + 1] = 0;
}

void executarPrioridade(int pri){

	printf("pri: %d\n", pri);
}

void executarRealTime(int seg, int dur){

	printf("com: %d, fim: %d\n", seg, dur);
}
