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
void forkar(int *pid1, char *execName);
int filaEmExecucao = 0;
int currentPid = 0;
float count = 1.0;
Fila *filaPrioridade,*filaRoundRobin,*filaPrioridade2,*filaPrioridade3,*filaPrioridade4,*filaPrioridade5,*filaPrioridade6,*filaPrioridade7;
LIS_tppLista listaRT;
RealTime *r1,*r2;
Priority *pCorr = NULL,*pCorr2 = NULL,*pCorr3 = NULL,*pCorr4 = NULL,*pCorr5 = NULL,*pCorr6 = NULL,*pCorr7 = NULL;
RoundRobin *rrCorr = NULL;
int taExecRT = 0,foiMorto = 0;
int tamanhoFP,tamanhoFP2,tamanhoFP3,tamanhoFP4,tamanhoFP5,tamanhoFP6,tamanhoFP7,tamanhoFRR,taExecFP,taExecFP2,taExecFP3,taExecFP4,taExecFP5,taExecFP6,taExecFP7,taExecFRR;

int devolveNumFila(Fila *fila) {
    Fila *filas[7] = {filaPrioridade,filaPrioridade2,filaPrioridade3,filaPrioridade4,filaPrioridade5,filaPrioridade6,filaPrioridade7};
    int i;
    for(i=0;i<7;i++) {
        if(filas[i] == fila) {
            return i + 1;
        }
    }
    return 0;
}

void atualizaTamanhos() {
    obterTamanhoFila(filaPrioridade,&tamanhoFP);
    obterTamanhoFila(filaPrioridade2,&tamanhoFP2);
    obterTamanhoFila(filaPrioridade3,&tamanhoFP3);
    obterTamanhoFila(filaPrioridade4,&tamanhoFP4);
    obterTamanhoFila(filaPrioridade5,&tamanhoFP5);
    obterTamanhoFila(filaPrioridade6,&tamanhoFP6);
    obterTamanhoFila(filaPrioridade7,&tamanhoFP7);
    obterTamanhoFila(filaRoundRobin,&tamanhoFRR);

}

void queueHandler(int *currentPidPar, Fila **fila, int *foiMortoPar, Priority **corr, int tamanho,int outrosNaoEstaoExecutando, Fila *filaProx,Priority *pProx) {
    int numCiclos;
    Priority *corrTemp;
    obterNumCiclos(*fila, &numCiclos);
    if(numCiclos > 0) { /* se o numero de ciclos que a fila ainda vai realizar é maior que zero */
        numCiclos--; /* decresce em um numero de ciclos restantes */
        trocaNumCiclos(*fila,numCiclos); /* atualiza numero de ciclos */
        if((*corr) != NULL /*&& tamanho > 0 */) {
            filaEmExecucao = devolveNumFila(*fila);
            if((*corr)->numCiclos > 0) {
                /* se o numero de ciclos restantes do processo que está rodando for maior que um */
                if (*foiMortoPar == 1) {
                    kill(*currentPidPar, SIGCONT);
                    *foiMortoPar = 0;
                }
                (*corr)->numCiclos--; /* um ciclo foi consumido */
            } else { /* se o programa corrente percorreu todos os ciclos que podia */
                if(*currentPidPar == (*corr)->pid && *currentPidPar != 0) {
                    kill(*currentPidPar,SIGSTOP);
                }
                kill((*corr)->pid, SIGSTOP);
                (*corr)->numCiclos = (*corr)->priority; /* coloca o numero de ciclos que ele executa */
                FIL_InserirNaFila(*fila, (*corr)); /* o coloca no fim da fila */
                FIL_ObterValor(*fila,(void **)&corrTemp); /* pega novo processo da fila */
                (*corr) = corrTemp;
                *currentPidPar = (*corr)->pid; /* o transforma em processo corrente */
                kill((*corr)->pid, SIGCONT); /* o executa */
            }
        } else {
            if(*currentPidPar != 0) {
                kill(*currentPidPar, SIGSTOP);
            }
            FIL_ObterValor(*fila,(void **)&corrTemp); /* pega novo processo da fila */
            (*corr) = corrTemp;
            if((*corr) != NULL) {
                //printf("pelo menos aqui entrou %d\n",(*corr)->pid);
                filaEmExecucao = devolveNumFila(*fila);
                kill((*corr)->pid, SIGCONT);
                *currentPidPar = (*corr)->pid;
                (*corr)->numCiclos--;
            } else {
                //printf("entrou no tamanho igual zero\n");
                if(outrosNaoEstaoExecutando) {
                    //printf("deu certo essa parada\n");
                    trocaTaExec(filaProx,1); /* ativa a fila de processos round robin */
                    trocaTaExec(*fila, 0);
                    if(pProx && pProx->numCiclos > 0) { /* se algum processo round robin estava executando anteriormente */
                        *currentPidPar = pProx->pid;
                        kill(pProx->pid, SIGCONT); /* volte a executar ele */
                    }
                }
            }
        }
        
    } else { /* se o numero de ciclos tiver se esgotado */
        if((*corr) != NULL) {
            if(*currentPidPar != (*corr)->pid && *currentPidPar != 0) {
                kill(*currentPidPar, SIGSTOP);
            }
            obterNumCiclos(*fila, &numCiclos);
            kill((*corr)->pid, SIGSTOP); /* para o processo de prioridades rodando */
            trocaTaExec(*fila, 0); /* faz com que a fila de prioridades fique inativa */
            trocaNumCiclos(*fila,numCiclos); /* reinicializa o numero de ciclos */
            trocaTaExec(filaProx,1); /* ativa a fila de processos round robin */
            if(pProx && pProx->numCiclos > 0) { /* se algum processo round robin estava executando anteriormente */
                *currentPidPar = pProx->pid;
                obterNumCiclos(filaProx, &numCiclos);
                numCiclos--;
                trocaNumCiclos(filaProx, numCiclos);
                kill(pProx->pid, SIGCONT); /* volte a executar ele */
                pProx->numCiclos--;
            }
        }
    }
}



void scheduler() {
    RealTime *aux;
    int numCiclosFP,numCiclosFP2,numCiclosFP3,numCiclosFP4,numCiclosFP5,numCiclosFP6,numCiclosFP7,numCiclosFRR;
    IrInicioLista(listaRT);
    obterTaExec(filaPrioridade5,&taExecFP5);
    //printf("ta exec da 5: %d\n",taExecFP5);
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
            
            obterTaExec(filaPrioridade7,&taExecFP7);
            obterTaExec(filaRoundRobin,&taExecFRR);
            
            if(taExecFP7 == 1) { /* se um processo da fila de prioridades estiver em execução */
                

                //filaCorr = filaPrioridade3
                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP3 == 0 && taExecFP4 == 0 && taExecFP5 == 0 && taExecFP6 == 0 && taExecFRR == 0) {
                    //printf("entrei aqui\n");
                    queueHandler(&currentPid, &filaPrioridade7,&foiMorto, &pCorr7, tamanhoFP7,1, filaPrioridade6, pCorr6);
                } else {
                    queueHandler(&currentPid, &filaPrioridade7,&foiMorto, &pCorr7, tamanhoFP7,0, filaPrioridade6, pCorr6);
                }
                
            }
            
            obterTaExec(filaPrioridade7,&taExecFP7);
            obterTaExec(filaPrioridade6,&taExecFP6);
            
            if(taExecFP6 == 1) { /* se um processo da fila de prioridades estiver em execução */
                //filaCorr = filaPrioridade3
                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP3 == 0 && taExecFP4 == 0 && taExecFP5 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade6,&foiMorto, &pCorr6, tamanhoFP6,1, filaPrioridade5, pCorr5);
                } else {
                    queueHandler(&currentPid, &filaPrioridade6,&foiMorto, &pCorr6, tamanhoFP6,0, filaPrioridade5, pCorr5);
                }
                
            }
            //printf("taExecFP5: %d e taExecFP7: %d\n",taExecFP5,taExecFP7);
            obterTaExec(filaPrioridade6,&taExecFP6);
            obterTaExec(filaPrioridade5,&taExecFP5);
            obterTaExec(filaPrioridade2, &taExecFP2);
            if(taExecFP5 == 1) { /* se um processo da fila de prioridades estiver em execução */
                
                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP3 == 0 && taExecFP4 == 0 && taExecFP6 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade5,&foiMorto, &pCorr5, tamanhoFP5,1, filaPrioridade4, pCorr4);
                } else {
                    queueHandler(&currentPid, &filaPrioridade5,&foiMorto, &pCorr5, tamanhoFP5,0, filaPrioridade4, pCorr4);
                }
                
            }
            
            obterTaExec(filaPrioridade5,&taExecFP5);
            obterTaExec(filaPrioridade4,&taExecFP4);
            if(taExecFP4 == 1) { /* se um processo da fila de prioridades estiver em execução */
                //filaCorr = filaPrioridade3
                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP3 == 0 && taExecFP5 == 0 && taExecFP6 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade4,&foiMorto, &pCorr4, tamanhoFP4,1, filaPrioridade3, pCorr3);
                } else {
                    queueHandler(&currentPid, &filaPrioridade4,&foiMorto, &pCorr4, tamanhoFP4,0, filaPrioridade3, pCorr3);
                }
                
            }
            
            obterTaExec(filaPrioridade4,&taExecFP4);
            obterTaExec(filaPrioridade3,&taExecFP3);
            if(taExecFP3 == 1) { /* se um processo da fila de prioridades estiver em execução */
                //filaCorr = filaPrioridade3
                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP4 == 0 && taExecFP5 == 0 && taExecFP6 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade3,&foiMorto, &pCorr3, tamanhoFP3,1, filaPrioridade2, pCorr2);
                } else {
                    queueHandler(&currentPid, &filaPrioridade3,&foiMorto, &pCorr3, tamanhoFP3,0, filaPrioridade2, pCorr2);
                }
                
            }
            
            obterTaExec(filaPrioridade3,&taExecFP3);
            obterTaExec(filaPrioridade2,&taExecFP2);

            if(taExecFP2 == 1) { /* se um processo da fila de prioridades estiver em execução */
                //printf("to no 2\n");
                //filaCorr = filaPrioridade2;
                if(taExecFP == 0 && taExecFP3 == 0 && taExecFP4 == 0 && taExecFP5 == 0 && taExecFP6 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade2,&foiMorto, &pCorr2, tamanhoFP2,1, filaPrioridade, pCorr);
                } else {
                    queueHandler(&currentPid, &filaPrioridade2,&foiMorto, &pCorr2, tamanhoFP2,0, filaPrioridade, pCorr);
                }
            }
            
            obterTaExec(filaPrioridade2,&taExecFP2);
            obterTaExec(filaPrioridade,&taExecFP);
            //printf("taExecFp: %d\n",taExecFP);
            if(taExecFP == 1) { /* se um processo da fila de prioridades estiver em execução */
                if(taExecFP2 == 0 && taExecFP3 == 0 && taExecFP4 == 0 && taExecFP5 == 0 && taExecFP6 == 0 && taExecFP7 == 0 && taExecFRR == 0) {
                    queueHandler(&currentPid, &filaPrioridade,&foiMorto, &pCorr, tamanhoFP,1, filaRoundRobin, rrCorr);
                } else {
                    queueHandler(&currentPid, &filaPrioridade,&foiMorto, &pCorr, tamanhoFP,0, filaRoundRobin, rrCorr);
                }
            }
            atualizaTamanhos();
                obterTaExec(filaPrioridade,&taExecFP);
                obterTaExec(filaRoundRobin,&taExecFRR);
                if(taExecFRR == 1) { /* se um processo da fila de roundRobin estiver em execução */
                    //filaCorr = filaRoundRobin;
                    obterNumCiclos(filaRoundRobin, &numCiclosFRR);
                    if(numCiclosFRR > 0) { /* se o numero de ciclos que a fila ainda vai realizar é maior que zero */
                        if (foiMorto == 1) {
                            kill(currentPid, SIGCONT);
                            foiMorto = 0;
                        }
                        numCiclosFRR--;
                        trocaNumCiclos(filaRoundRobin,numCiclosFRR);
                        if(rrCorr != NULL /*&& tamanhoFRR > 0*/) { /* se há algum processo da fila de round robin executando */
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
                                rrCorr->numCiclos--; /* indica que um ciclo foi consumido
                            */
                            } else {
                                if(taExecFP == 0 && taExecFP2 == 0 && taExecFP3 == 0 && taExecFP5 == 0 && taExecFP6 == 0) {
                                    trocaTaExec(filaPrioridade7,1); /* ativa a fila de processos de proridade */
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
                            //printf("entrou nessa bagaça\n");
                            kill(rrCorr->pid, SIGSTOP); /* para o processo round robin rodando */
                            trocaTaExec(filaRoundRobin, 0); /* faz com que a fila de round robin fique inativa */
                            trocaNumCiclos(filaRoundRobin,6); /* reinicializa o numero de ciclos */
                            trocaTaExec(filaPrioridade7,1); /* ativa a fila de processos de proridade */
                            if(pCorr7 && pCorr7->numCiclos > 0) { /* se algum processo round robin estava executando anteriormente */
                                currentPid = pCorr7->pid;
                                obterNumCiclos(filaPrioridade7,&numCiclosFP7);
                                numCiclosFP7--;
                                trocaNumCiclos(filaPrioridade7, numCiclosFP7);
                                kill(pCorr7->pid, SIGCONT); /* volte a executar ele */
                                pCorr7->numCiclos--;
                            } else if(pCorr6 && pCorr6->numCiclos > 0) {
                                currentPid = pCorr6->pid;
                                obterNumCiclos(filaPrioridade6,&numCiclosFP6);
                                numCiclosFP6--;
                                trocaNumCiclos(filaPrioridade6, numCiclosFP6);
                                kill(pCorr6->pid, SIGCONT); /* volte a executar ele */
                                pCorr6->numCiclos--;
                            } else if(pCorr5 && pCorr5->numCiclos > 0) {
                                currentPid = pCorr5->pid;
                                obterNumCiclos(filaPrioridade5,&numCiclosFP5);
                                numCiclosFP5--;
                                trocaNumCiclos(filaPrioridade5, numCiclosFP5);
                                kill(pCorr5->pid, SIGCONT); /* volte a executar ele */
                                pCorr5->numCiclos--;
                            } else if(pCorr4 && pCorr4->numCiclos > 0) {
                                currentPid = pCorr4->pid;
                                obterNumCiclos(filaPrioridade4,&numCiclosFP4);
                                numCiclosFP4--;
                                trocaNumCiclos(filaPrioridade4, numCiclosFP4);
                                kill(pCorr4->pid, SIGCONT); /* volte a executar ele */
                                pCorr4->numCiclos--;
                            } else if(pCorr3 && pCorr3->numCiclos > 0) {
                                currentPid = pCorr3->pid;
                                obterNumCiclos(filaPrioridade3,&numCiclosFP3);
                                numCiclosFP3--;
                                trocaNumCiclos(filaPrioridade3, numCiclosFP3);
                                kill(pCorr3->pid, SIGCONT); /* volte a executar ele */
                                pCorr3->numCiclos--;
                            } else if(pCorr2 && pCorr2->numCiclos > 0) {
                                currentPid = pCorr2->pid;
                                obterNumCiclos(filaPrioridade2,&numCiclosFP2);
                                numCiclosFP2--;
                                trocaNumCiclos(filaPrioridade2, numCiclosFP2);
                                kill(pCorr2->pid, SIGCONT); /* volte a executar ele */
                                pCorr2->numCiclos--;
                            } else if(pCorr && pCorr->numCiclos > 0) {
                                currentPid = pCorr->pid;
                                obterNumCiclos(filaPrioridade,&numCiclosFP);
                                numCiclosFP--;
                                trocaNumCiclos(filaPrioridade, numCiclosFP);
                                kill(pCorr->pid, SIGCONT); /* volte a executar ele */
                                pCorr->numCiclos--;

                            }
                        }
                        
                    
                    }
                }

        }
    }

Priority *p23;

void catchAlarm(int signal) {
    printf("entrei aqui %f e %d\n",count,currentPid);
    Fila *filas[7] = {filaPrioridade,filaPrioridade2,filaPrioridade3,filaPrioridade4,filaPrioridade5,filaPrioridade6,filaPrioridade7};
    int numFila;
    Fila *ret;
    if(count == 15) {
        p23 = (Priority *) malloc(sizeof(Priority));
        p23->priority = 5;
        p23->execName = "./prog4";
        p23->numCiclos = 5;
        FIL_InserirNaFila(filaPrioridade5, (void *)p23);
        forkar(&(p23->pid),p23->execName);
        if(filaEmExecucao < 5) {
            printf("%d\n",p23->pid);
            trocaTaExec(filas[filaEmExecucao - 1], 0);
            trocaTaExec(filaPrioridade5, 1);
        }
    }
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
        //printf("ebah: %d\n",getpid());
                //raise(SIGSTOP);
        //printf("ebah: %d\n",getpid());
                execv(args[0],args);
    } else {
        kill(pid, SIGSTOP);
		*pid1 = pid;
    }

}



int main() {
	signal(SIGALRM, catchAlarm);
    char l[] = "l";

    
    
    int fifo;
    printf("entrei aqui\n");
    
	if((fifo = open("minhaFifo", O_RDONLY | O_NONBLOCK)) < 0) {
		puts("Erro ao abrir a FIFO escrita\n");
		return -1;
	}
    

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
    FIL_CriaFila(&filaPrioridade,0,10);
    FIL_CriaFila(&filaPrioridade2,0,12);
    FIL_CriaFila(&filaPrioridade3,0,14);
    FIL_CriaFila(&filaPrioridade4,0,16);
    FIL_CriaFila(&filaPrioridade5,0,18);
    FIL_CriaFila(&filaPrioridade6,0,20);
    FIL_CriaFila(&filaPrioridade7,1,22);
    FIL_CriaFila(&filaRoundRobin,0,6);
    
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
    p->priority = 2;
    p->execName = "./prog3";
    p->numCiclos = 2;
    FIL_InserirNaFila(filaPrioridade2, (void *)p);
    //p23 = (Priority *) malloc(sizeof(Priority));
    //p23->priority = 5;
    //p23->execName = "./prog4";
    //p23->numCiclos = 5;
    //FIL_InserirNaFila(filaPrioridade5, (void *)p23);
    ualarm(500000,500000);
	forkar(&(r1->pid),r1->execName);
	forkar(&(r2->pid),r2->execName);
	forkar(&(p->pid),p->execName);
    forkar(&(rr->pid), rr->execName);
    forkar(&(rr2->pid), rr2->execName);
    forkar(&(rr3->pid), rr3->execName);
    obterTaExec(filaPrioridade,&taExecFP);
    obterTaExec(filaPrioridade2,&taExecFP2);
    obterTaExec(filaPrioridade3,&taExecFP3);
    obterTaExec(filaPrioridade4,&taExecFP4);
    obterTaExec(filaPrioridade5,&taExecFP5);
    obterTaExec(filaPrioridade6,&taExecFP6);
    obterTaExec(filaPrioridade7,&taExecFP7);
    obterTamanhoFila(filaPrioridade, &tamanhoFP);
    obterTamanhoFila(filaPrioridade2, &tamanhoFP2);
    obterTamanhoFila(filaPrioridade3, &tamanhoFP3);
    obterTamanhoFila(filaPrioridade4, &tamanhoFP4);
    obterTamanhoFila(filaPrioridade5, &tamanhoFP5);
    obterTamanhoFila(filaPrioridade6, &tamanhoFP6);
    obterTamanhoFila(filaPrioridade7, &tamanhoFP7);
    obterTamanhoFila(filaRoundRobin, &tamanhoFRR);

	while(1);
	return 0;
}


void destruirValor(void *pValor) {
    free(pValor);
    
}
