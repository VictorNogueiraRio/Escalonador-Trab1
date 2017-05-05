#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <string.h>
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
    int numCiclos;
	int priority;
	char *execName;
}Priority;

typedef struct roundroubin {
	int pid;
    int numCiclos;
	char *execName;
}RoundRobin;


union semaun{

    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

// inicializa o valor do semáforo
int setSemValue(int semId);

// remove o semáforo
void delSemValue(int semId);

// operação P
int semaforoP(int semId);

//operação V
int semaforoV(int semId);

void scheduler();
void forkar(int *pid1, char *execName);
int filaEmExecucao = 8,currentPid = 0,segmento,*valorShm,semId;
float count = 1.0;
Fila *filaPrioridade,*filaRoundRobin,*filaPrioridade2,*filaPrioridade3,*filaPrioridade4,*filaPrioridade5,*filaPrioridade6,*filaPrioridade7;
LIS_tppLista listaRT;
Priority *pCorr = NULL,*pCorr2 = NULL,*pCorr3 = NULL,*pCorr4 = NULL,*pCorr5 = NULL,*pCorr6 = NULL,*pCorr7 = NULL;
RoundRobin *rrCorr = NULL;
int taExecRT = 0,foiMorto = 0;
int taExecFP,taExecFP2,taExecFP3,taExecFP4,taExecFP5,taExecFP6,taExecFP7,taExecFRR;

int devolveNumFila(Fila *fila) {
    Fila *filas[8] = {filaRoundRobin,filaPrioridade,filaPrioridade2,filaPrioridade3,filaPrioridade4,filaPrioridade5,filaPrioridade6,filaPrioridade7};
    int i;
    for(i=0;i<8;i++) {
        if(filas[i] == fila) {
            return i;
        }
    }
    return 0;
}


int verificaRTValido(int startTime, int EndTime) {
    RealTime *aux;
    IrInicioLista(listaRT);
    do {
        LIS_ObterNo(listaRT, (void **)&aux);
        if(aux == NULL) {
            break;
        }
        if(EndTime > 60) {
            return 0;
        }
        if(startTime >= aux->startTime && startTime < aux->EndTime) {
            return 0;
        }
        if(EndTime >= aux->startTime && EndTime <= aux->EndTime) {
            return 0;
        }
    } while (LIS_IrProx(listaRT) != LIS_CondRetFimLista);
    return 1;
}

void queueHandler(int *currentPidPar, Fila **fila, int *foiMortoPar, Priority **corr,int outrosNaoEstaoExecutando, Fila *filaProx,Priority *pProx) {
    int numCiclos;
    Priority *corrTemp;
    obterNumCiclos(*fila, &numCiclos);
    if(numCiclos > 0) { /* se o numero de ciclos que a fila ainda vai realizar é maior que zero */
        numCiclos--; /* decresce em um numero de ciclos restantes */
        trocaNumCiclos(*fila,numCiclos); /* atualiza numero de ciclos */
        if((*corr) != NULL) {
            filaEmExecucao = devolveNumFila(*fila);
            if((*corr)->numCiclos > 0) { /* se o numero de ciclos restantes do processo que está rodando for maior que um */
                if (*foiMortoPar == 1) {
                    kill(*currentPidPar, SIGCONT);
                    *foiMortoPar = 0;
                } else {
                    if(*currentPidPar != (*corr)->pid) {
                        *currentPidPar = (*corr)->pid;
                        kill(*currentPidPar,SIGCONT);
                    }
                }
                (*corr)->numCiclos--; /* um ciclo foi consumido */
            } else { /* se o programa corrente percorreu todos os ciclos que podia */
                if(*currentPidPar == (*corr)->pid && *currentPidPar != 0) {
                    kill(*currentPidPar,SIGSTOP);
                }
                kill((*corr)->pid, SIGSTOP);
                (*corr)->numCiclos = 8 - (*corr)->priority; /* coloca o numero de ciclos que ele executa */
                FIL_InserirNaFila(*fila, (*corr)); /* o coloca no fim da fila */
                FIL_ObterValor(*fila,(void **)&corrTemp); /* pega novo processo da fila */
                (*corr) = corrTemp;
                *currentPidPar = (*corr)->pid; /* o transforma em processo corrente */
                (*corr)->numCiclos--;
                kill((*corr)->pid, SIGCONT); /* o executa */
            }
        } else { /* se é a primeira vez que se executa processos dessa fila */
            if(*currentPidPar != 0) {
                kill(*currentPidPar, SIGSTOP);
            }
            FIL_ObterValor(*fila,(void **)&corrTemp); /* pega novo processo da fila */
            (*corr) = corrTemp;
            if((*corr) != NULL) { /* se há alguem na fila */
                filaEmExecucao = devolveNumFila(*fila);
                kill((*corr)->pid, SIGCONT);
                *currentPidPar = (*corr)->pid;
                (*corr)->numCiclos--;
            } else { /* se a fila está vazia */
                if(outrosNaoEstaoExecutando) {
                    trocaTaExec(filaProx,1); /* ativa a fila a próxima fila a ser executada */
                    trocaTaExec(*fila, 0); /* desativa a fila sendo executada */
                    if(pProx  != NULL/*&& pProx->numCiclos > 0 */) { /* se algum processo round robin estava executando anteriormente */
                        trocaNumCiclos(filaProx, devolveNumFila(filaProx) * 2 + 10);
                        //*currentPidPar = pProx->pid;
                        //kill(pProx->pid, SIGCONT);/ * volte a executar ele */
                    }
                }
            }
        }

    } else { /* se o numero de ciclos tiver se esgotado */
        if((*corr) != NULL) {
            if(*currentPidPar != (*corr)->pid && *currentPidPar != 0) {
                kill(*currentPidPar, SIGSTOP);
            }
            kill((*corr)->pid, SIGSTOP); /* para o processo de prioridades rodando */
            trocaTaExec(*fila, 0); /* faz com que a fila de prioridades que estava sendo executada fique inativa */
            trocaNumCiclos(*fila,devolveNumFila(*fila) * 2 + 10); /* reinicializa o numero de ciclos */
            trocaTaExec(filaProx,1); /* ativa a próxima fila de processos a ser executada */
            if(pProx != NULL) { /* se a próxima fila a ser executada tiver um processo que foi parado */
                ////printf("entrei no if que devia\n");
                //*currentPidPar = pProx->pid;
                trocaNumCiclos(filaProx,devolveNumFila(filaProx) * 2 + 10);
            }
        }
    }
}



void scheduler() {
    RealTime *aux;
    int numCiclosFRR;
    IrInicioLista(listaRT);
    obterTaExec(filaPrioridade5,&taExecFP5);
        do {
            LIS_ObterNo(listaRT, (void **)&aux);
            if(aux == NULL) {
                break;
            }
            if(count == aux->startTime) { /* começa o real time */
                //printf("vou executar o real time\n");
                taExecRT = 1;
                if(currentPid) { /* se houver algum processo não real time executando */
                    kill(currentPid,SIGSTOP); /* mata o processo */
                }
                kill(aux->pid,SIGCONT);
            } else if(count == aux->EndTime) { /* termina o real time */
                if(currentPid) {
                    foiMorto = 1;
                }
                taExecRT = 0;
                kill(aux->pid,SIGSTOP);
            }

        } while (LIS_IrProx(listaRT) != LIS_CondRetFimLista);
        if(taExecRT == 0) { /*se nenhum processo real time estiver em execução */

            obterTaExec(filaRoundRobin,&taExecFRR);
            obterTaExec(filaPrioridade,&taExecFP);
            if(taExecFP == 1) { /* se um processo da fila de prioridades 1 estiver em execução */
                if(taExecFP2 == 0 && taExecFP3 == 0 && taExecFP4 == 0 && taExecFP5 == 0 && taExecFP6 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade,&foiMorto, &pCorr,1,filaPrioridade2, pCorr2);
                } else {
                    queueHandler(&currentPid, &filaPrioridade,&foiMorto, &pCorr,0,filaPrioridade2, pCorr2);
                }
            }


            obterTaExec(filaPrioridade,&taExecFP);
            obterTaExec(filaPrioridade2,&taExecFP2);

            if(taExecFP2 == 1) { /* se um processo da fila de prioridades 2 estiver em execução */
                if(taExecFP == 0 && taExecFP3 == 0 && taExecFP4 == 0 && taExecFP5 == 0 && taExecFP6 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade2,&foiMorto, &pCorr2,1, filaPrioridade3, pCorr3);
                } else {
                    queueHandler(&currentPid, &filaPrioridade2,&foiMorto, &pCorr2,0, filaPrioridade3, pCorr3);
                }
            }

            obterTaExec(filaPrioridade2,&taExecFP2);
            obterTaExec(filaPrioridade3,&taExecFP3);
            if(taExecFP3 == 1) { /* se um processo da fila de prioridades 3 estiver em execução */
                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP4 == 0 && taExecFP5 == 0 && taExecFP6 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade3,&foiMorto, &pCorr3,1, filaPrioridade4, pCorr4);
                } else {
                    queueHandler(&currentPid, &filaPrioridade3,&foiMorto, &pCorr3,0, filaPrioridade4, pCorr4);
                }

            }

            obterTaExec(filaPrioridade3,&taExecFP3);
            obterTaExec(filaPrioridade4,&taExecFP4);
            if(taExecFP4 == 1) { /* se um processo da fila de prioridades 4 estiver em execução */
                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP3 == 0 && taExecFP5 == 0 && taExecFP6 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade4,&foiMorto, &pCorr4,1, filaPrioridade5, pCorr5);
                } else {
                    queueHandler(&currentPid, &filaPrioridade4,&foiMorto, &pCorr4,0, filaPrioridade5, pCorr5);
                }

            }


            obterTaExec(filaPrioridade5,&taExecFP5);
            obterTaExec(filaPrioridade4,&taExecFP4);
            if(taExecFP5 == 1) { /* se um processo da fila de prioridades 5 estiver em execução */
                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP3 == 0 && taExecFP4 == 0 && taExecFP6 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade5,&foiMorto, &pCorr5,1, filaPrioridade6, pCorr6);
                } else {
                    queueHandler(&currentPid, &filaPrioridade5,&foiMorto, &pCorr5,0, filaPrioridade6, pCorr6);
                }

            }

            obterTaExec(filaPrioridade5,&taExecFP5);
            obterTaExec(filaPrioridade6,&taExecFP6);
            if(taExecFP6 == 1) { /* se um processo da fila de prioridades 6 estiver em execução */
                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP3 == 0 && taExecFP4 == 0 && taExecFP5 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade6,&foiMorto, &pCorr6,1, filaPrioridade7, pCorr7);
                } else {
                    queueHandler(&currentPid, &filaPrioridade6,&foiMorto, &pCorr6,0, filaPrioridade7, pCorr7);
                }

            }

            obterTaExec(filaPrioridade6,&taExecFP6);
            obterTaExec(filaPrioridade7,&taExecFP7);
            if(taExecFP7 == 1) { /* se um processo da fila de prioridades 7 estiver em execução */
                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP3 == 0 && taExecFP4 == 0 && taExecFP5 == 0 && taExecFP6 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade7,&foiMorto, &pCorr7,1, filaRoundRobin, rrCorr);
                } else {
                    queueHandler(&currentPid, &filaPrioridade7,&foiMorto, &pCorr7,0, filaRoundRobin, rrCorr);
                }

            }

            ////printf("taExecFP5: %d e taExecFP7: %d\n",taExecFP5,taExecFP7);

                obterTaExec(filaPrioridade,&taExecFP);
                obterTaExec(filaRoundRobin,&taExecFRR);
                if(taExecFRR == 1) { /* se um processo da fila de roundRobin estiver em execução */
                    ////printf("taExecFP: %d\n",taExecFP);
                    obterNumCiclos(filaRoundRobin, &numCiclosFRR);
                    ////printf("entrou no rr: %d\n",numCiclosFRR);
                    if(numCiclosFRR > 0 || (pCorr == NULL && pCorr2 == NULL && pCorr3 == NULL && pCorr4 == NULL && pCorr5 == NULL && pCorr6 == NULL && pCorr7 == NULL)) { /* se o numero de ciclos que a fila ainda vai realizar é maior que zero */
                        if (foiMorto == 1) {
                            kill(currentPid, SIGCONT);
                            foiMorto = 0;
                        }
                        numCiclosFRR--;
                        if(numCiclosFRR == 0 && (pCorr == NULL && pCorr2 == NULL && pCorr3 == NULL && pCorr4 == NULL && pCorr5 == NULL && pCorr6 == NULL && pCorr7 == NULL)) {
                            trocaNumCiclos(filaRoundRobin,6);
                        } else {
                            trocaNumCiclos(filaRoundRobin,numCiclosFRR);
                        }
                        if(rrCorr != NULL) { /* se há algum processo da fila de round robin executando */
                            filaEmExecucao = devolveNumFila(filaRoundRobin);
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
                            if(rrCorr != NULL) {
                                filaEmExecucao = devolveNumFila(filaRoundRobin);
                                kill(rrCorr->pid, SIGCONT); /* o executa */
                                currentPid = rrCorr->pid; /* o transforma em corrente */
                                rrCorr->numCiclos--; /* indica que um ciclo foi consumido */
                            } else {
                                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP3 == 0 && taExecFP5 == 0 && taExecFP6 == 0) {
                                    trocaTaExec(filaPrioridade,1); /* ativa a fila de processos de proridade */
                                    trocaTaExec(filaRoundRobin, 0);
                                    if(pCorr7 && pCorr7->numCiclos > 0) { /* se algum processo round robin estava executando anteriormente */
                                        currentPid = pCorr7->pid;
                                        kill(pCorr7->pid, SIGCONT); /* volte a executar ele */
                                    }
                                }
                            }
                        }

                    } else { /* se o numero de ciclos tiver se esgotado*/
                        if(rrCorr != NULL) {
                            ////printf("rrcorr: %d e curr: %d\n",rrCorr->pid,currentPid);
                            if(rrCorr->pid == currentPid) {
                                ////printf("entrou no igual\n");
                                kill(rrCorr->pid, SIGSTOP); /* para o processo round robin rodando */
                                trocaTaExec(filaRoundRobin, 0); /* faz com que a fila de round robin fique inativa */
                                trocaNumCiclos(filaRoundRobin,10); /* reinicializa o numero de ciclos */
                            } else {
                                ////printf("entrou no diferente\n");
                                kill(currentPid,SIGSTOP);
                                kill(rrCorr->pid,SIGCONT);
                                trocaNumCiclos(filaRoundRobin,9);
                                rrCorr->numCiclos--;
                            }
                        }
                        if(rrCorr->pid == currentPid) {
                            trocaTaExec(filaPrioridade,1);
                            scheduler();
                        }
                    }
                }

        }
    }

Priority *p23;

void catchAlarm(int signal) {
    char oq[30];
    char echo[33] = {'e','c','h','o',32,'s','e','c',':',32,0,0,0,0,0,0,0,0,0,0,0,0,0};
    char novo [14]= {32,'>','>',32,'s','a','i','d','a','.','t','x','t',0};
    sprintf(oq,"%f",count);
    oq[4] = '\0';
        strcat(echo,oq);
    strcat(echo,novo);
    printf("%s\n",echo);
    system(echo);
    scheduler();
	if(count == 59.5) {
		count = 0.5;
	}
	count += 0.5;
}

void criaRealTime(char *execName,int startTime,int EndTime) {
    RealTime *r1;
    int validez;
    validez = verificaRTValido(startTime,EndTime);
    if(validez == 1) {
        r1 = (RealTime *) malloc(sizeof(RealTime));
        r1->startTime = startTime;
        r1->EndTime = EndTime;
        r1->execName = (char *) malloc(sizeof(char) * strlen(execName));
        strcpy(r1->execName, execName);
        LIS_InserirNo(listaRT, (void *)r1);
        forkar(&(r1->pid),r1->execName);
    }
}

void criaRoundRobin(char *execName) {
    RoundRobin *r1;
    r1 = (RoundRobin *) malloc(sizeof(RoundRobin));
    r1->execName = (char *) malloc(sizeof(char) * strlen(execName) + 1);
    r1->numCiclos = 1;
    strcpy(r1->execName, execName);
    FIL_InserirNaFila(filaRoundRobin, (void *)r1);
    forkar(&(r1->pid),r1->execName);
}

void criaPriority(char *execName,int priority) {
    Priority *p1 = (Priority *) malloc(sizeof(Priority));
    p1->execName = (char *) malloc(8);
    Fila *filas[8] = {filaRoundRobin,filaPrioridade,filaPrioridade2,filaPrioridade3,filaPrioridade4,filaPrioridade5,filaPrioridade6,filaPrioridade7};
    p1->numCiclos = priority;
    p1->priority = priority;
    strcpy(p1->execName,execName);
    switch (priority) {
        case 1:
            FIL_InserirNaFila(filaPrioridade, (void *)p1);
            if(filaEmExecucao > 1 || filaEmExecucao == 0) {
                trocaTaExec(filaPrioridade,1);
                trocaTaExec(filas[filaEmExecucao],0);
                filaEmExecucao = 1;
            }
            break;
        case 2:
            FIL_InserirNaFila(filaPrioridade2, (void *)p1);
            if(filaEmExecucao > 2 || filaEmExecucao == 0) {
                trocaTaExec(filas[filaEmExecucao],0);
                trocaTaExec(filaPrioridade2,1);
                filaEmExecucao = 2;
            }
            break;
        case 3:
            FIL_InserirNaFila(filaPrioridade3, (void *)p1);
            if(filaEmExecucao > 3 || filaEmExecucao == 0) {
                trocaTaExec(filas[filaEmExecucao],0);
                trocaTaExec(filaPrioridade3,1);
                filaEmExecucao = 3;

            }
            break;
        case 4:
            FIL_InserirNaFila(filaPrioridade4, (void *)p1);
            if(filaEmExecucao > 4 || filaEmExecucao == 0) {
                trocaTaExec(filas[filaEmExecucao],0);
                trocaTaExec(filaPrioridade4,1);
                filaEmExecucao = 4;
            }
            break;
        case 5:
            FIL_InserirNaFila(filaPrioridade5, (void *)p1);
            if(filaEmExecucao > 5 || filaEmExecucao == 0) {
                //printf("entrou certo 5\n");
                trocaTaExec(filas[filaEmExecucao],0);
                trocaTaExec(filaPrioridade5,1);
                filaEmExecucao = 5;
            }
            break;
        case 6:
            FIL_InserirNaFila(filaPrioridade6, (void *)p1);
            if(filaEmExecucao > 6 || filaEmExecucao == 0) {
                //printf("entrou certo 6\n");
                trocaTaExec(filas[filaEmExecucao],0);
                trocaTaExec(filaPrioridade6,1);
                filaEmExecucao = 6;
            }
            break;
        case 7:
            FIL_InserirNaFila(filaPrioridade7, (void *)p1);
            if(filaEmExecucao == 7 || filaEmExecucao == 0) {
                //printf("entrou certo 7\n");
                trocaTaExec(filas[filaEmExecucao],0);
                trocaTaExec(filaPrioridade7,1);
                filaEmExecucao = 7;
            }
            break;
        default:
            puts("erro numero prioridade\n");
            exit(-1);
    }
    forkar(&(p1->pid),p1->execName);
}


void forkar(int *pid1, char *execName) {
    int pid;
    char *const args[] = {execName,0};
    if ((pid = fork()) == 0) {
         semaforoP(semId);
         if(*valorShm ==  0) {
            *valorShm = 1;
            raise(SIGSTOP);
         }
        semaforoV(semId);
        execv(args[0],args);
    } else {
         semaforoP(semId);
         if(*valorShm ==  0) {
            *valorShm = 1;
            kill(pid, SIGSTOP);
         }
         semaforoV(semId);

         *pid1 = pid;
    }

}



int main() {
	signal(SIGALRM, catchAlarm);
    char l[] = "l";
    char priority,startTime,EndTime,tipo,tamanhoChar[3];
    int startTimeI,EndTimeI,priorityI,fifo,tamanhoInt;

    int fd1,fd2;
    segmento = shmget(1234, sizeof(int), IPC_CREAT | S_IRUSR | S_IWUSR);
    semId = semget(1234, 1, IPC_CREAT);
    valorShm = (int *) shmat(segmento, 0, 0);
    *valorShm = 0;
    if((fd1 = open("entrada.txt",O_RDONLY)) == 0) {
        puts("erro ao abrir entrada.txt");
        exit(-1);
    }
    if((fd2 = open("saida.txt",O_WRONLY)) == 0) {
        puts("erro ao abrir saida.txt");
        exit(-1);
    }

    if((fifo = open("minhaFifo2", O_RDONLY)) < 0) {
        puts("Erro ao abrir a FIFO2 escrita\n");
        return -1;
    }

    /*
    close(0);
    dup(fd1);
    close(1);
    dup(fd2);
     */

    /* Cria Lista */
    LIS_CriarLista(&listaRT,l,destruirValor);

    /* Cria filas */
    FIL_CriaFila(&filaPrioridade,0,24);
    FIL_CriaFila(&filaPrioridade2,0,22);
    FIL_CriaFila(&filaPrioridade3,0,20);
    FIL_CriaFila(&filaPrioridade4,0,18);
    FIL_CriaFila(&filaPrioridade5,0,16);
    FIL_CriaFila(&filaPrioridade6,0,14);
    FIL_CriaFila(&filaPrioridade7,0,12);
    FIL_CriaFila(&filaRoundRobin,1,10);
    filaEmExecucao = 0;

    ualarm(500000,500000);
    /* Inicializa booleano que diz se uma fila está sendo executada */
    obterTaExec(filaPrioridade,&taExecFP);
    obterTaExec(filaPrioridade2,&taExecFP2);
    obterTaExec(filaPrioridade3,&taExecFP3);
    obterTaExec(filaPrioridade4,&taExecFP4);
    obterTaExec(filaPrioridade5,&taExecFP5);
    obterTaExec(filaPrioridade6,&taExecFP6);
    obterTaExec(filaPrioridade7,&taExecFP7);

    while(1) {
        char execPriority[30];
        if(read(fifo,&tipo,1) > 0) {
            read(fifo,tamanhoChar,2);
            tamanhoChar[2] = 0;
            tamanhoInt = atoi(tamanhoChar);
            ////printf("tipo: %c\n",tipo);
            ////printf("tamanho: %d\n",tamanhoInt);
            switch (tipo) {
                case 'r': /* se for round robin */
                    read(fifo,execPriority,tamanhoInt + 3);
                    ////printf("exec: %s\n",execPriority);
                    *valorShm = 0;
                    criaRoundRobin(execPriority);
                                        break;
                case 't': /* se for real time */
                    read(fifo,execPriority,tamanhoInt + 7);
                    startTime = execPriority[tamanhoInt + 2];
                    startTimeI = startTime - 48;
                    startTimeI *= 10;
                    startTime = execPriority[tamanhoInt + 3];
                    startTimeI += (startTime - 48);
                    EndTime = execPriority[tamanhoInt + 4];
                    EndTimeI = execPriority[tamanhoInt + 4] - 48;
                    EndTimeI  *= 10;
                    EndTime = execPriority[tamanhoInt + 5];
                    EndTimeI  += (EndTime - 48);
                    execPriority[tamanhoInt + 2] = 0;
                    *valorShm = 0;
                    criaRealTime(execPriority,startTimeI,EndTimeI);
                    break;
                case 'p': /* se for prioridade */
                    read(fifo,execPriority,tamanhoInt + 2);
                    read(fifo,&priority,2);
                    priorityI = atoi(&priority);
                    ////printf("priority: %c\n",priority);
                    ////printf("exec pri: %s\n",execPriority);
                    *valorShm = 0;
                    criaPriority(execPriority,priorityI);
                    break;
                default:
                    puts("foi nenhum");
                    exit(-1);
                    break;
            }
        }

    }
	return 0;
}

int setSemValue(int semId){

    union semaun semUnion;
    semUnion.val = 1;
    return semctl(semId, 0, SETVAL, semUnion);
}

void delSemValue(int semId){

    union semaun semUnion;
    semctl(semId, 0, IPC_RMID, semUnion);
}

int semaforoP(int semId){

    struct sembuf semB;
    semB.sem_num = 0;
    semB.sem_op = -1;
    semB.sem_flg = SEM_UNDO;
    semop(semId, &semB, 1);
    return 0;
}

int semaforoV(int semId){

    struct sembuf semB;
    semB.sem_num = 0;
    semB.sem_op = 1;
    semB.sem_flg = SEM_UNDO;
    semop(semId, &semB, 1);
    return 0;
}


void destruirValor(void *pValor) {
    free(pValor);

}
