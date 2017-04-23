#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void executarRoundRobin();
void executarPrioridade(int pri);
void executarRealTime(int seg, int dur);

int main(){

	char nome[15], arg1[15], arg2[15], numero[4];
	int num1, num2, n;
	FILE * exec;

	exec = fopen ("exec.txt", "r");

	fscanf(exec, "%s", nome);

	while (1){

		n = fscanf(exec, "%s %s", nome, arg1);

		if (n==1){
			printf("%s - ultimo roundrobin\n", nome);

			executarRoundRobin();
			sleep(1);

			break;
		}

		else if (arg1[0]=='E'){
			printf("%s - roundrobin\n", nome);

			executarRoundRobin();
			sleep(1);
		}

		else if (arg1[0]=='P'){
			printf("%s - prioridade\n", nome);

			numero[0]=arg1[3];
			numero[1]='\0';
			num1=atof(numero);
			
			executarPrioridade(num1);
			sleep(1);

			if (fscanf(exec, "%s", nome)!=1){

				break;
			}
		}

		else if (arg1[0]=='I'){
			printf("%s - Real-time\n", nome);

			numero[0]=arg1[2];
			numero[1]=arg1[3];
			num1=atof(numero);

			fscanf(exec, "%s", arg2);

			numero[0]=arg2[2];
			numero[1]=arg2[3];
			num2=atof(numero);

			executarRealTime(num1, num2);
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

void executarPrioridade(int pri){

	printf("%d\n", pri);
}

void executarRealTime(int seg, int dur){

	printf("%d, %d\n", seg, dur);
}
