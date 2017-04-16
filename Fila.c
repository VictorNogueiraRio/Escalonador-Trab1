//
//  Fila.c
//  
//
//  Created by Victor Nogueira on 4/15/17.
//
//
#include "Fila.h"
#include <stdio.h>
#include <stdlib.h>
#include "Lista.h"


char f[] = "f";

static void destruirValor(void *pValor);

typedef struct fila {
    LIS_tppLista lista;
    int taExec;
    int numCiclos;
    
} Fila;


void trocaTaExec(Fila *f,int taExec) {
    if(f != NULL) {
        f->taExec = taExec;
    }
}

void obterNumCiclos(Fila *fila, int *numCiclos) {
    if(fila != NULL) {
        *numCiclos = fila->numCiclos;
    }
}

void obterTaExec(Fila *fila, int *taExec) {
    if(fila != NULL) {
        *taExec = fila->taExec;
    }
}


void trocaNumCiclos(Fila *f,int numCiclos) {
    if(f != NULL) {
        f->numCiclos = numCiclos;
    }
}

int FIL_CriaFila(Fila **fila,int taExec,int numCiclos) {
    Fila *fi = (Fila *) malloc(sizeof(Fila));
    trocaTaExec(fi,taExec);
    trocaNumCiclos(fi, numCiclos);
    if(fi == NULL) {
        return -1;
    }
    LIS_CriarLista(&(fi->lista),f,destruirValor);
    *fila = fi;
    return 0;
}

void FIL_InserirNaFila(Fila *fila, void *elemento) {
     LIS_IrFimLista(fila->lista);
     LIS_InserirNo(fila->lista,elemento);
}

int FIL_ObterValor(Fila *fila,void **elemento) {
     void *aux = NULL;
     IrInicioLista(fila->lista);
     LIS_ObterNo(fila->lista,&aux);
     if(aux == NULL) {
         *elemento = NULL;
         return -1;
     }
     LIS_ExcluirNoCorrente(fila->lista);
     *elemento = aux;
     return 0;
}

void FIL_DeletaFila(Fila *fila) {
    LIS_DestroiLista(fila->lista);
    free(fila);
}

void obterTamanhoFila(Fila *fila, int *tamanho) {
    int tam;
    if(fila != NULL) {
        LIS_RetornaNumElementos(fila->lista, &tam);
        *tamanho = tam;
    } else {
        *tamanho = 0;
    }

}

void destruirValor(void *pValor) {
    free(pValor);

}
