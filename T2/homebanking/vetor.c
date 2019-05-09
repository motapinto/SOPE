#include "vetor.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "types.h"

//Baseado nas bibliotecas implementadas em PROG 2 do MIEEC 2017/2018, professor Luis Teixeira. Adaptado por up201704618

vetor* vetor_novo()
{
	vetor* vec;

	/* aloca memoria para a estrutura vetor */
	vec = (vetor*)malloc(sizeof(vetor));
	if(vec == NULL)
	return NULL;

	vec->tamanho = 0;
	vec->capacidade = 0;
	vec->elementos = NULL;

	return vec;
}

void vetor_apaga(vetor* vec)
{
	int i;

	if(vec == NULL)
	return;

	/* liberta memoria de cada string */
	for (i = 0; i < vec->tamanho; i++)
	{
		free(vec->elementos[i].client);
	}

	/* liberta memoria dos apontares para as strings */
	free(vec->elementos);

	/* liberta memoria da estrutura vetor */
	free(vec);
}

int vetor_tamanho(vetor* vec)
{
	if(vec == NULL)
	return -1;

	return vec->tamanho;
}

int vetor_insere(vetor *vec, const bank_account_t valor, int pos)
{
	int i;

	if(vec == NULL || pos < -1 || pos > vec->tamanho)
		return -1;

    if(vec->tamanho==MAX_BANK_ACCOUNTS){
        printf("Atingimos o limite de clientes no vetor\n");
        return -1;
    }


	/* aumenta elementos do vetor se capacidade nao for suficiente */
	if(vec->tamanho == vec->capacidade)
	{
		if(vec->capacidade == 0)
		vec->capacidade = 1;
		else
		vec->capacidade *= 2;

		vec->elementos = (v_elemento*)realloc(vec->elementos, vec->capacidade * sizeof(v_elemento));
		if(vec->elementos == NULL)
		return -1;
	}

	/* se pos=-1 insere no fim do vetor */
	if(pos == -1)
		pos = vec->tamanho;

	/* copia todos os elementos a partir da posicao pos ate' ao fim do vetor para pos+1 */
	for(i=vec->tamanho-1; i>=pos; i--)
	{
		vec->elementos[i+1] = vec->elementos[i];
	}

	/* aloca espaco para a nova struct client na posicao pos */
	
    vec->elementos[pos].client = (bank_account_t*)calloc(1,sizeof(bank_account_t));
	
    if(vec->elementos[pos].client == NULL)
	    return -1;

	/* copia valor */
	vec->elementos[pos].client->account_id=valor.account_id;
    
    strcpy(vec->elementos[pos].client->hash,valor.hash);
    strcpy(vec->elementos[pos].client->salt,valor.salt);
    
    vec->elementos[pos].client->balance=valor.balance;

	vec->tamanho++;

	return pos;
}

int vetor_remove(vetor* vec, int pos)
{
	int i;

	if(vec == NULL || pos < 0 || pos >= vec->tamanho)
	return -1;

	/* liberta string na posicao a remover */
	free(vec->elementos[pos].client);

	/* copia todos os elementos a partir da posicao pos+1 ate' ao fim do vetor para pos */
	for(i=pos+1; i<vec->tamanho; i++)
	{
		vec->elementos[i-1] = vec->elementos[i];
	}

	vec->tamanho--;

	return 0;
}

int vetor_pesquisa(vetor *vec, uint32_t account_id)
{
	int i;

	if(vec == NULL)
	return -2;

	/* pesquisa sequencial */
	for (i = 0; i < vec->tamanho; i++)
	{
		if (vec->elementos[i].client->account_id==account_id)
		    return i;
	}

	return -1;
}
