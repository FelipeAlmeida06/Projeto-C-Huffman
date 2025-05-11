#ifndef HUFFMAN_H
#define HUFFMAN_H

#include "codigo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EXT_LEN 10
#define MAX_PATH_LEN 1024

typedef unsigned char U8;

typedef struct No {
    U8 data;
    int freq;
    struct No *esq, *dir;
} No;

typedef struct HuffmanCodes {
    Codigo* codigos[256]; // Para todos os valores de byte possíveis (0-255)
} HuffmanCodes;

typedef struct ArvoreHuffman {
    No* raiz;
    HuffmanCodes huffmanCodes;
} ArvoreHuffman;

No* criarNo(U8 data, int freq);
void liberarArvore(No* node);
No* construirArvore(U8* dados, int* frequencias, int tamanho);
void gerarCodigos(No* raiz, HuffmanCodes* huffmanCodes);
void escreverArvore(FILE* arquivo, No* raiz);
No* lerArvore(FILE* arquivo);
int comprimir(const char* inputFile, const char* outputFile);
int descomprimir(const char* inputFile, const char* outputFile);
void getFileExtension(const char* filename, char* extension, size_t extSize);

#endif