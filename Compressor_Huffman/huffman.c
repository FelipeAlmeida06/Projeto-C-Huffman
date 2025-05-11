#include "huffman.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define BUFFER_SIZE 8192

No* criarNo(unsigned char data, int freq) {
    No* novo = (No*)malloc(sizeof(No));
    if (!novo) return NULL;
    
    novo->data = data;
    novo->freq = freq;
    novo->esq = novo->dir = NULL;
    return novo;
}

void liberarArvore(No* node) {
    if (node == NULL) return;
    liberarArvore(node->esq);
    liberarArvore(node->dir);
    free(node);
}

No* construirArvore(unsigned char* dados, int* frequencias, int tamanho) {
    No *esq, *dir, *top;
    
    No** nos = (No**)malloc(tamanho * sizeof(No*));
    for (int i = 0; i < tamanho; i++) {
        nos[i] = criarNo(dados[i], frequencias[i]);
    }
    
    while (tamanho != 1) {
        int primeiro = 0, segundo = 1;
        if (nos[primeiro]->freq > nos[segundo]->freq) {
            primeiro = 1;
            segundo = 0;
        }
        
        for (int i = 2; i < tamanho; i++) {
            if (nos[i]->freq < nos[primeiro]->freq) {
                segundo = primeiro;
                primeiro = i;
            } else if (nos[i]->freq < nos[segundo]->freq) {
                segundo = i;
            }
        }
        
        esq = nos[primeiro];
        dir = nos[segundo];
        top = criarNo('$', esq->freq + dir->freq);
        top->esq = esq;
        top->dir = dir;
        
        nos[primeiro] = top;
        nos[segundo] = nos[tamanho - 1];
        tamanho--;
    }
    
    No* raiz = nos[0];
    free(nos);
    return raiz;
}

static void gerarCodigosAux(No* raiz, Codigo* codigoAtual, HuffmanCodes* huffmanCodes) {
    if (raiz->esq == NULL && raiz->dir == NULL) {
        huffmanCodes->codigos[raiz->data] = (Codigo*)malloc(sizeof(Codigo));
        novo_codigo(huffmanCodes->codigos[raiz->data]);
        clone(*codigoAtual, huffmanCodes->codigos[raiz->data]);
        return;
    }
    
    if (raiz->esq) {
        adiciona_bit(codigoAtual, 0);
        gerarCodigosAux(raiz->esq, codigoAtual, huffmanCodes);
        joga_fora_bit(codigoAtual);
    }
    
    if (raiz->dir) {
        adiciona_bit(codigoAtual, 1);
        gerarCodigosAux(raiz->dir, codigoAtual, huffmanCodes);
        joga_fora_bit(codigoAtual);
    }
}

void gerarCodigos(No* raiz, HuffmanCodes* huffmanCodes) {
    if (raiz == NULL) return;
    
    for (int i = 0; i < 256; i++) {
        huffmanCodes->codigos[i] = NULL;
    }
    
    Codigo codigoAtual;
    novo_codigo(&codigoAtual);
    gerarCodigosAux(raiz, &codigoAtual, huffmanCodes);
    free_codigo(&codigoAtual);
}

void escreverArvore(FILE* arquivo, No* raiz) {
    if (raiz == NULL) return;
    
    if (raiz->esq == NULL && raiz->dir == NULL) {
        fputc('1', arquivo);
        fputc(raiz->data, arquivo);
    } else {
        fputc('0', arquivo);
        escreverArvore(arquivo, raiz->esq);
        escreverArvore(arquivo, raiz->dir);
    }
}

No* lerArvore(FILE* arquivo) {
    int bit = fgetc(arquivo);
    if (bit == '1') {
        unsigned char data = fgetc(arquivo);
        return criarNo(data, 0);
    } else {
        No* node = criarNo('$', 0);
        node->esq = lerArvore(arquivo);
        node->dir = lerArvore(arquivo);
        return node;
    }
}

void getFileExtension(const char* filename, char* extension, size_t extSize);

// void comprimir
int comprimir(const char* inputFile, const char* outputFile) {
    HuffmanCodes* huffmanCodes = (HuffmanCodes*)malloc(sizeof(HuffmanCodes));
    if (!huffmanCodes) {
        perror("Erro ao alocar memória para HuffmanCodes");
        return 1;
    }
    for (int i = 0; i < 256; i++) {
        huffmanCodes->codigos[i] = NULL;
    }

    int frequencias[256] = {0};
    FILE* inFile = fopen(inputFile, "rb");
    if (!inFile) {
        perror("Erro ao abrir arquivo de entrada");
        free(huffmanCodes);
        return 1;
    }

    unsigned char byte;
    while (fread(&byte, sizeof(byte), 1, inFile) == 1) {
        frequencias[byte]++;
    }
    fclose(inFile);

    unsigned char dadosUnicos[256];
    int freqsUnicas[256];
    int numUnicos = 0;
    for (int i = 0; i < 256; i++) {
        if (frequencias[i] > 0) {
            dadosUnicos[numUnicos] = (unsigned char)i;
            freqsUnicas[numUnicos] = frequencias[i];
            numUnicos++;
        }
    }

    No* root = construirArvore(dadosUnicos, freqsUnicas, numUnicos);
    if (!root) {
        fprintf(stderr, "Erro ao construir a árvore de Huffman.\n");
        free(huffmanCodes);
        return 1;
    }

    gerarCodigos(root, huffmanCodes);

    char extOrig[256];
    getFileExtension(inputFile, extOrig, sizeof(extOrig));

    FILE* outFile = fopen(outputFile, "wb");
    if (!outFile) {
        perror("Erro ao abrir arquivo de saída para escrever a árvore");
        liberarArvore(root);
        free(huffmanCodes);
        return 1;
    }
    fprintf(outFile, "EXTENSION:%s\n", extOrig); // Escreve a extensão como string
    escreverArvore(outFile, root);
    fclose(outFile);

    inFile = fopen(inputFile, "rb");
    if (!inFile) {
        perror("Erro ao abrir arquivo de entrada para compressão");
        liberarArvore(root);
        free(huffmanCodes);
        return 1;
    }

    outFile = fopen(outputFile, "ab");
    if (!outFile) {
        perror("Erro ao abrir arquivo de saída para escrever dados comprimidos");
        fclose(inFile);
        liberarArvore(root);
        free(huffmanCodes);
        return 1;
    }

    unsigned char buffer[BUFFER_SIZE];
    size_t bytesLidos;
    unsigned char outputByte = 0;
    int bitCount = 0;

    while ((bytesLidos = fread(buffer, 1, BUFFER_SIZE, inFile)) > 0) {
        for (size_t i = 0; i < bytesLidos; i++) {
            unsigned char b = buffer[i];
            Codigo* codigo = huffmanCodes->codigos[b]; // Obtém o código para o byte atual
            if (codigo) {
                for (int j = 0; j < codigo->tamanho; j++) { // Itera sobre os bits do código
                    if (codigo->byte[j] == 1) {
                        outputByte |= (1 << (7 - bitCount));
                    }
                    bitCount++;
                    if (bitCount == 8) {
                        fwrite(&outputByte, 1, 1, outFile);
                        outputByte = 0;
                        bitCount = 0;
                    }
                }
            }
        }
    }

    // Escreve os bits restantes (padding com zeros)
    if (bitCount > 0) {
        fwrite(&outputByte, 1, 1, outFile);
    }

    fclose(inFile);
    fclose(outFile);
    liberarArvore(root);
    free(huffmanCodes);

    return 0;
}

// void descomprimir
int descomprimir(const char* inputFile, const char* outputFile) {
    FILE* inFile = fopen(inputFile, "rb");
    if (!inFile) {
        perror("Erro ao abrir arquivo compactado para leitura");
        return 1;
    }

    char extension[256];
    if (fscanf(inFile, "EXTENSION:%s\n", extension) != 1) {
        fprintf(stderr, "Erro ao ler a extensão do arquivo compactado.\n");
        fclose(inFile);
        return 1;
    }

    No* root = lerArvore(inFile);
    if (!root) {
        fprintf(stderr, "Erro ao ler a árvore de Huffman do arquivo compactado.\n");
        fclose(inFile);
        return 1;
    }

    FILE* outFile = fopen(outputFile, "wb");
    if (!outFile) {
        perror("Erro ao abrir arquivo de saída para escrita");
        liberarArvore(root);
        fclose(inFile);
        return 1;
    }

    //unsigned char byteLido;
    No* current = root;
    int bit;

    while ((bit = fgetc(inFile)) != EOF) {
        for (int i = 7; i >= 0; i--) {
            int bitValor = (bit >> i) & 1;
            if (bitValor == 0) {
                current = current->esq;
            } else {
                current = current->dir;
            }

            if (current->esq == NULL && current->dir == NULL) {
                fwrite(&current->data, 1, 1, outFile);
                current = root;
            }
        }
    }

    fclose(inFile);
    fclose(outFile);
    liberarArvore(root);

    // Renomear o arquivo de saída para incluir a extensão original
    char outputFilenameWithExt[1024];
    snprintf(outputFilenameWithExt, sizeof(outputFilenameWithExt), "%s.%s", outputFile, extension);
    remove(outputFile); // Remove o arquivo sem extensão
    if (rename(outputFile, outputFilenameWithExt) != 0) {
        perror("Erro ao renomear o arquivo de saída");
        return 1;
    }

    return 0;
}