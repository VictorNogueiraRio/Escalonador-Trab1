//
//  Fila.h
//  
//
//  Created by Victor Nogueira on 4/15/17.
//
//


typedef struct fila Fila;

int FIL_CriaFila(Fila **fila,int taExec,int numCiclos);
void trocaNumCiclos(Fila *f,int numCiclos);
void trocaTaExec(Fila *f,int taExec);
void FIL_InserirNaFila(Fila *fila, void *elemento);
int FIL_ObterValor(Fila *fila,void **elemento);
void FIL_DeletaFila(Fila *fila);
void obterTaExec(Fila *fila, int *taExec);
void obterNumCiclos(Fila *fila, int *numCiclos);
void obterTamanhoFila(Fila *fila, int *tamanho);

