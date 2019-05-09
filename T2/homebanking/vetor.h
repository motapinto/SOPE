#pragma once

#ifndef VETOR_H

#include "types.h"
//Baseado nas bibliotecas implementadas em PROG 2 do MIEEC 2017/2018, professor Luis Teixeira. Adaptado por up201704618

/**
* registo para armazenar cada elemento (apenas uma string neste caso)
*/
typedef struct
{
  bank_account_t* client;

} v_elemento;

/**
* este registo contem um vetor de elementos, um contador do tamanho e outro para a capacidade
*/
typedef struct
{
  /** numero de elementos do vetor */
  int tamanho;

  /** capacidade do vetor */
  int capacidade;

  /** array de elementos armazenados */
  v_elemento* elementos;

} vetor;

/**
*  cria um novo vetor com tamanho 0 e capacidade 0
*  retorno: apontador para o vetor criado
*  nota: se vetor nao foi criado retorna NULL
*/
vetor* vetor_novo();

/**
*  elimina um vetor, libertando toda a memoria ocupada
*  parametro: vec apontador para vetor
*  nota: se vec = NULL retorna sem fazer nada
*/
void vetor_apaga(vetor *vec);

/**
*  indica o o numero de elementos do vetor
*  parametro: vec apontador para vetor
*  retorno: -1 se ocorrer algum erro ou numero de elementos do vetor
*/
int vetor_tamanho(vetor *vec);

/**
*  retorna o elemento armazenado na posicao especificada
*  parametro: vec apontador para vetor
*  parametro: pos indice do elemento
*  retorno: apontador para a string na posicao correspondente
*  nota: se ocorrer algum erro retorna NULL (p.ex. se valor pos indicar uma posicao invalida)
*/

int vetor_insere(vetor *vec, const bank_account_t valor, int pos);

/**
*  remove o elemento da posicao especificada
*  parametro: vec apontador para vetor
*  parametro: pos posicao
*  retorno: -1 se ocorrer algum erro (p.ex. se valor pos indicar uma posicao invalida) ou 0 se bem sucedido
*/
int vetor_remove(vetor* vec, int pos);

/**
*  devolve a posicao do elemento especificado
*  parametro: vec apontador para vetor
*  parametro: str string pretendida
*  retorno: posicao do elemento ou -2 se ocorrer algum erro -1 se nao encontrar elemento
*/
int vetor_pesquisa(vetor *vec, uint32_t account_id);


#define VETOR_H
#endif