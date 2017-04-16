#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include "Fila.h"
#include "Lista.h"


void destruirValor(void *pValor);

typedef struct realTime {
	int pid;
	float startTime;
	float EndTime;
    float duration;
	char *execName;
}RealTime;

typedef struct priority {
	int pid;
	int priority;
    int numCiclos;
	char *execName;
}Priority;

typedef struct roundroubin {
	int pid;
    int numCiclos;
	char *execName;
}RoundRobin;

void scheduler();

int currentPid = 0;
float count = 1.0;
Fila *filaPrioridade,*filaRoundRobin;
LIS_tppLista listaRT;
RealTime *r1,*r2;
Priority *pCorr = NULL;
RoundRobin *rrCorr = NULL;
int taExecRT = 0,foiMorto = 0;

void scheduler() {
    RealTime *aux;
    int taExecFP;
    int taExecFRR;
    int tamanhoFRR;
    int tamanhoFP;
    int numCiclosFP;
    int numCiclosFRR;
    obterTamanhoFila(filaPrioridade, &tamanhoFP);
    obterTamanhoFila(filaRoundRobin, &tamanhoFRR);
    IrInicioLista(listaRT);
        do {
            
            LIS_ObterNo(listaRT, (void **)&aux);
            if(aux == NULL) {
                break;
            }
            LIS_ObterNo(listaRT, (void **)&aux);
            if(count == aux->startTime) { /* começa o real time */
                //priorityHandler(aux->EndTime,0);
                taExecRT = 1;
                if(currentPid) { /* se houver algum processo não real time executando */
                    kill(currentPid,SIGSTOP); /* mata o processo */
                    //printf("quando matou %d\n",currentPid);
                }
                kill(aux->pid,SIGCONT);
            } else if(count == aux->EndTime) { /* termina o real time */
                //printf("pid do process:%d e %d\n",getpid(),aux->pid);
                if(currentPid) {
                    foiMorto = 1;
                }
                taExecRT = 0;
                kill(aux->pid,SIGSTOP);
            }

        } while (LIS_IrProx(listaRT) != LIS_CondRetFimLista);
        if(taExecRT == 0) { /*se nenhum processo real time estiver em execução */
            obterTaExec(filaPrioridade,&taExecFP);
            if(taExecFP == 1) { /* se um processo da fila de prioridades estiver em execução */
                obterNumCiclos(filaPrioridade, &numCiclosFP);
                if(numCiclosFP > 0) { /* se o numero de ciclos que a fila ainda vai realizar é maior que zero */
                    numCiclosFP--; /* decresce em um numero de ciclos restantes */
                    trocaNumCiclos(filaPrioridade,numCiclosFP); /* atualiza numero de ciclos */
                    if(pCorr != NULL) {
                        if(pCorr->numCiclos > 1) {
                            /* se o numero de ciclos restantes do processo que está rodando for maior que um */
                            if (foiMorto == 1) {
                                kill(currentPid, SIGCONT);
                                foiMorto = 0;
                            }
                            pCorr->numCiclos--; /* um ciclo foi consumido */
                        } else { /* se o programa corrente percorreu todos os ciclos que podia */
                            kill(pCorr->pid, SIGSTOP);
                            pCorr->numCiclos = pCorr->priority; /* coloca o numero de ciclos que ele executa */
                            FIL_InserirNaFila(filaPrioridade, pCorr); /* o coloca no fim da fila */
                            FIL_ObterValor(filaPrioridade,(void **)&pCorr); /* pega novo processo da fila */
                            currentPid = pCorr->pid; /* o transforma em processo corrente */
                            kill(pCorr->pid, SIGCONT); /* o executa */
                        }
                    } else {
                        FIL_ObterValor(filaPrioridade,(void **)&pCorr); /* pega novo processo da fila */
                        kill(pCorr->pid, SIGCONT);
                        currentPid = pCorr->pid;
                        pCorr->numCiclos--;
                    }
                    
                } else { /* se o numero de ciclos tiver se esgotado */
                    if(pCorr != NULL) {
                        kill(pCorr->pid, SIGSTOP); /* para o processo de prioridades rodando */
                        trocaTaExec(filaPrioridade, 0); /* faz com que a fila de prioridades fique inativa */
                        trocaNumCiclos(filaPrioridade,10); /* reinicializa o numero de ciclos */
                        trocaTaExec(filaRoundRobin,1); /* ativa a fila de processos round robin */
                        if(rrCorr && rrCorr->numCiclos > 0) { /* se algum processo round robin estava executando anteriormente */
                            kill(rrCorr->pid, SIGCONT); /* volte a executar ele */
                        }
                    }
                }
            }
                obterTaExec(filaRoundRobin,&taExecFRR);
                if(taExecFRR == 1) { /* se um processo da fila de roundRobin estiver em execução */
                    obterNumCiclos(filaRoundRobin, &numCiclosFRR);
                    if(numCiclosFRR > 0) { /* se o numero de ciclos que a fila ainda vai realizar é maior que zero */
                        if (foiMorto == 1) {
                            kill(currentPid, SIGCONT);
                            foiMorto = 0;
                        }
                        numCiclosFRR--;
                        trocaNumCiclos(filaRoundRobin,numCiclosFRR);
                        if(rrCorr != NULL) { /* se há algum processo da fila de round robin executando */
                            if(rrCorr->numCiclos > 1) { /* se o numero de ciclos restantes do processo que está rodando for maior que um */
                                rrCorr->numCiclos--; /* um ciclo foi consumido */
                            } else { /* se o programa corrente percorreu todos os ciclos que podia */
                                kill(rrCorr->pid, SIGSTOP); /* para o programa corrente */
                                rrCorr->numCiclos = 1; /* coloca o numero de ciclos total */
                                FIL_InserirNaFila(filaRoundRobin, rrCorr); /* o coloca no fim da fila */
                                FIL_ObterValor(filaRoundRobin,(void **)&rrCorr); /* pega novo processo da fila */
                                currentPid = rrCorr->pid; /* o transforma em processo corrente */
                                kill(rrCorr->pid, SIGCONT); /* o executa */
                            }
                        } else { /* se não há */
                            FIL_ObterValor(filaRoundRobin,(void **)&rrCorr); /* pega novo processo da fila */
                            kill(rrCorr->pid, SIGCONT); /* o executa */
                            currentPid = rrCorr->pid; /* o transforma em corrente */
                            rrCorr->numCiclos--; /* indica que um ciclo foi consumido */
                        }
                        
                    } else { /* se o numero de ciclos tiver se esgotado*/
                        if(rrCorr != NULL) {
                            kill(rrCorr->pid, SIGSTOP); /* para o processo round robin rodando */
                            trocaTaExec(filaRoundRobin, 0); /* faz com que a fila de round robin fique inativa */
                            trocaNumCiclos(filaRoundRobin,8); /* reinicializa o numero de ciclos */
                            trocaTaExec(filaPrioridade,1); /* ativa a fila de processos de proridade */
                            if(pCorr && pCorr->numCiclos > 0) { /* se algum processo round robin estava executando anteriormente */
                                kill(pCorr->pid, SIGCONT); /* volte a executar ele */
                            }
                        }
                    }
                }
        }
    }


void catchAlarm(int signal) {
        printf("entrei aqui %f e %d\n",count,currentPid);
	scheduler();
	if(count == 60) {
		count = 0.5;
	}
	count += 0.5;
}

void calcDuration(RealTime r) {
    r.duration = r.EndTime - r.startTime;
}

void forkar(int *pid1, char *execName) {
    int pid;
    char *const args[] = {execName,0};
	if ((pid = fork()) == 0) {
                raise(SIGSTOP);
                execv(args[0],args);
    } else {
		*pid1 = pid;
    }

}



int main() {
	signal(SIGALRM, catchAlarm);
    Priority *p23;
    char l[] = "l";

	/*
    int fifo;
	if((fifo = open("minhaFifo2", O_RDONLY)) < 0) {
		puts("foi porra\n");
		return -1;
	}
    */

    r1 = (RealTime *) malloc(sizeof(RealTime));
    r1->startTime = 1;
    r1->EndTime = 6;
    r1->execName = "./prog1";
    r2 = (RealTime *) malloc(sizeof(RealTime));
    r2->startTime = 10;
    r2->EndTime = 13;
    r2->execName = "./prog2";
    
    /* Cria Lista */
    LIS_CriarLista(&listaRT,l,destruirValor);
    //printf("end r1 antes: %d\n",r1);
    LIS_InserirNo(listaRT, (void *)r1);
    LIS_InserirNo(listaRT, (void *)r2);
    /* Cria filas */
    FIL_CriaFila(&filaPrioridade,1,10);
    FIL_CriaFila(&filaRoundRobin,0,8);
    
    RoundRobin *rr = (RoundRobin *) malloc(sizeof(RoundRobin));
    rr->numCiclos = 1;
    rr->execName = "./prog5";
    
    RoundRobin *rr2 = (RoundRobin *) malloc(sizeof(RoundRobin));
    rr2->numCiclos = 1;
    rr2->execName = "./prog6";
    
    RoundRobin *rr3 = (RoundRobin *) malloc(sizeof(RoundRobin));
    rr3->numCiclos = 1;
    rr3->execName = "./prog7";
    
    FIL_InserirNaFila(filaRoundRobin, (void *)rr);
    FIL_InserirNaFila(filaRoundRobin, (void *)rr2);
    FIL_InserirNaFila(filaRoundRobin, (void *)rr3);
    
    /* Cria prioridades */
    Priority *p = (Priority *) malloc(sizeof(Priority));
    if(p == NULL) {
        //printf("ruim brabo\n");
        exit(-1);
    }
    p->priority = 7;
    p->execName = "./prog3";
    p->numCiclos = 7;
    FIL_InserirNaFila(filaPrioridade, (void *)p);
    p23 = (Priority *) malloc(sizeof(Priority));
    p23->priority = 2;
    p23->execName = "./prog4";
    p23->numCiclos = 2;
    FIL_InserirNaFila(filaPrioridade, (void *)p23);
    ualarm(500000,500000);
	forkar(&(r1->pid),r1->execName);
	forkar(&(r2->pid),r2->execName);
	forkar(&(p->pid),p->execName);
    forkar(&(p23->pid),p23->execName);
    forkar(&(rr->pid), rr->execName);
    forkar(&(rr2->pid), rr2->execName);
    forkar(&(rr3->pid), rr3->execName);
    //printf("rr pid: %d\n",rr->pid);

	while(1);
	return 0;
}


void destruirValor(void *pValor) {
    free(pValor);
    
}
