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

	char nome[30], arg1[15], arg2[15], numero[4];
	int num1, num2, n,fifo,tamanhoInt;
	FILE * exec;
    
    if ((fifo = open("minhaFifo2", OPENMODE)) < 0) {
        puts ("Erro ao abrir a FIFO para escrita");
        return -1;
    }

	exec = fopen ("exec.txt", "r");

	fscanf(exec, "%s", nome);

	while (1) {
        printf("entrou no loop\n");
		n = fscanf(exec, "%s %s", &(nome[5]), arg1);
        char *tamanhoChar = (char *) malloc(sizeof(char) * 3);
        tamanhoInt = strlen(&nome[5]);
        intToChar(tamanhoInt,&tamanhoChar);
        nome[1] = tamanhoChar[0];
        nome[2] = tamanhoChar[1];
        nome[3] = '.';
        nome[4] = '/';
		if (n==1){
			printf("%s - ultimo roundrobin\n", nome);
            sleep(1);
			break;
		}
		else if (arg1[0]=='E'){
            nome[0] = 'r';
            nome[tamanhoInt + 5] = 0;
            
            write(fifo,nome,tamanhoInt + 6);
            printf("round robin\n");
            sleep(1);
		}

		else if (arg1[0]=='P'){
            nome[0] = 'p';
			//printf("%s - prioridade\n", nome);
            printf("tamanhoInt: %d\n",tamanhoInt);
            nome[tamanhoInt + 5] = arg1[3];
            nome[tamanhoInt + 6] = 0;
            printf("nome exec: %s\n",nome);
            write(fifo,nome,tamanhoInt + 7);
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
            printf("st1 no int: %c\nst2 no int: %c\n",arg1[2],arg1[3]);
            if(arg1[3] == 0) {
                
                nome[tamanhoInt + 5] = arg1[3] + 48;
                nome[tamanhoInt + 6] = arg1[2];
            } else {
                nome[tamanhoInt + 5] = arg1[2];
                nome[tamanhoInt + 6] = arg1[3];
            }
			printf("st1 dps no int: %c\nst2 no int: %c\n",nome[tamanhoInt + 5],nome[tamanhoInt + 6]);
            fscanf(exec, "%s", arg2);

			numero[0]=arg2[2];
			numero[1]=arg2[3];
            
			num2=atof(numero);
            
            char *fim = (char *) malloc(3);
            
            num2 += num1;
            intToChar(num2,&fim);
            nome[tamanhoInt + 7] = fim[0];
            nome[tamanhoInt + 8] = fim[1];
            nome[tamanhoInt + 9] = 0;
            write(fifo,nome,tamanhoInt + 9);
            printf("real time: %s\n",nome);

			sleep(1);

			if (fscanf(exec, "%s", nome)!=1){

				break;
			}
            free(fim);
		}
        free(tamanhoChar);
        
	}
    
    	return 0;
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
