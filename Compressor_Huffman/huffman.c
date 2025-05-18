#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

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

void escreverNo(FILE* arq, No* no) {
    if (no == NULL) {
        fputc('X', arq); // Marca nulo
        return;
    }

    if (no->esq == NULL && no->dir == NULL) {
        // É uma folha
        fputc('L', arq);
        fwrite(&no->data, sizeof(unsigned char), 1, arq);
    } else {
        // É um nó interno
        fputc('I', arq);
        escreverNo(arq, no->esq);
        escreverNo(arq, no->dir);
    }
}

void escreverArvore(FILE* arquivo, No* raiz, const char* extOrig) {
    // Escreve cabeçalho estruturado
    fprintf(arquivo, "HUFFv2|EXT=%s|TREE=", extOrig);
    printf("[DEBUG - ESCREVER] Cabeçalho escrito: HUFFv2|EXT=%s|TREE=, ftell: %ld\n", extOrig, ftell(arquivo)); // Depuração

    // Serialização da árvore (versão robusta)
    if (raiz == NULL) {
        fputc('X', arquivo); // Marca nulo (nunca deve acontecer em Huffman válido)
        printf("[DEBUG - ESCREVER] Nó Nulo, ftell: %ld\n", ftell(arquivo)); // Depuração
        return;
    }

    if (raiz->esq == NULL && raiz->dir == NULL) {
        printf("[DEBUG - ESCREVER] Tipo: L, Dado: %d, ftell: %ld\n", raiz->data, ftell(arquivo));
        fputc('L', arquivo); // 'L' de folha (Leaf)
        fwrite(&raiz->data, sizeof(unsigned char), 1, arquivo); // Dado binário
        printf("[DEBUG - ESCREVER] Folha escrita, Dado: %d, ftell: %ld\n", raiz->data, ftell(arquivo)); // Depuração
    } else {
        printf("[DEBUG - ESCREVER] Tipo: I, ftell: %ld\n", ftell(arquivo));
        fputc('I', arquivo); // 'I' de nó interno (Internal)
        printf("[DEBUG - ESCREVER] Nó interno escrito, ftell: %ld\n", ftell(arquivo)); // Depuração
        escreverArvore(arquivo, raiz->esq, extOrig);
        escreverArvore(arquivo, raiz->dir, extOrig);
    }

    // Delimitador final
    fprintf(arquivo, "|ENDTREE|");
    printf("[DEBUG - ESCREVER] Delimitador final escrito, ftell: %ld\n", ftell(arquivo)); // Depuração
    
}


No* lerArvore(FILE* arquivo) {
    // Read the node type
    int tipo = fgetc(arquivo);
    if (tipo == EOF) {
        fprintf(stderr, "Erro: Fim inesperado do arquivo\n");
        return NULL;
    }

    No* no = NULL;
    switch (tipo) {
        case 'L': {
            unsigned char data;
            if (fread(&data, 1, 1, arquivo) != 1) {
                fprintf(stderr, "Erro ao ler folha\n");
                return NULL;
            }
            no = criarNo(data, 0);
            break;
        }
        case 'I': {
            no = criarNo('$', 0);
            if (!(no->esq = lerArvore(arquivo)) || !(no->dir = lerArvore(arquivo))) {
                liberarArvore(no);
                return NULL;
            }
            break;
        }
        case '|': // Encontrou delimitador final da árvore
            return NULL;
        case 'X': // Para compatibilidade com nós nulos (embora não devam ocorrer)
            return NULL;
        default:
            fprintf(stderr, "Erro: Tipo de nó desconhecido '%c'\n", tipo);
            return NULL;
    }

    return no;
}

void getFileExtension(const char* filename, char* extension, size_t extSize);

// void comprimir
int comprimir(const char* inputFile, const char* outputFile) {
    // Verificação de entrada
    if (inputFile == NULL || outputFile == NULL) {
        fprintf(stderr, "Erro: Caminho de arquivo inválido\n");
        return 1;
    }

    // Abrir arquivo de entrada
    FILE* inFile = fopen(inputFile, "rb");
    if (!inFile) {
        perror("Erro ao abrir arquivo de entrada");
        return 1;
    }

    // Contar frequências dos bytes
    int frequencias[256] = {0};
    unsigned char byte;
    while (fread(&byte, sizeof(byte), 1, inFile) == 1) {
        frequencias[byte]++;
    }
    fclose(inFile);

    // Construir árvore de Huffman
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
        fprintf(stderr, "Erro ao construir a árvore de Huffman\n");
        return 1;
    }

    // Gerar códigos Huffman
    HuffmanCodes huffmanCodes;
    for (int i = 0; i < 256; i++) {
        huffmanCodes.codigos[i] = NULL;
    }
    gerarCodigos(root, &huffmanCodes);

    const char* ext = strrchr(inputFile, '.');
    char extOrig[MAX_EXT_LEN] = {0};
    if (ext != NULL) {
        strncpy(extOrig, ext + 1, sizeof(extOrig) - 1);
        extOrig[sizeof(extOrig) - 1] = '\0'; // Garante terminação nula
    }

    // Abrir arquivo de saída
    FILE* outFile = fopen(outputFile, "wb");
    if (!outFile) {
        perror("Erro ao abrir arquivo de saída");
        liberarArvore(root);
        return 1;
    }

    // Escrever árvore no novo formato
    escreverArvore(outFile, root, extOrig);
    fclose(outFile);

    // Reabrir arquivos para compressão
    inFile = fopen(inputFile, "rb");
    outFile = fopen(outputFile, "ab");
    if (!inFile || !outFile) {
        perror("Erro ao reabrir arquivos");
        if (inFile) fclose(inFile);
        if (outFile) fclose(outFile);
        liberarArvore(root);
        return 1;
    }

    // Comprimir dados
    unsigned char buffer = 0;
    int bitCount = 0;
    while (fread(&byte, sizeof(byte), 1, inFile) == 1) {
        Codigo* codigo = huffmanCodes.codigos[byte];
        if (codigo) {
            for (int byteIdx = 0; byteIdx < (codigo->tamanho + 7) / 8; byteIdx++) {
                U8 currentByte = codigo->byte[byteIdx];
                int bitsInThisByte = (byteIdx == (codigo->tamanho / 8)) ? 
                                    codigo->tamanho % 8 : 8;
                if (bitsInThisByte == 0) bitsInThisByte = 8;
            
                for (int bitPos = 7; bitPos >= 8 - bitsInThisByte; bitPos--) {
                    if (currentByte & (1 << bitPos)) {
                        buffer |= (1 << (7 - bitCount));
                    }
                    bitCount++;
                
                    if (bitCount == 8) {
                        fputc(buffer, outFile);
                        buffer = 0;
                        bitCount = 0;
                    }
                }
            }
        }
    }

    // Escrever bits restantes
    if (bitCount > 0) {
        fputc(buffer, outFile);
    }

    // Liberar recursos
    fclose(inFile);
    fclose(outFile);
    liberarArvore(root);
    for (int i = 0; i < 256; i++) {
        if (huffmanCodes.codigos[i] != NULL) {
            free(huffmanCodes.codigos[i]);
        }
    }

    return 0;
}

int descomprimir(const char* compressedFile, const char* decompressedFile) {
    // Verificação de tamanhos de caminho
    if (strlen(compressedFile) >= MAX_PATH_LEN_HUFFMAN || strlen(decompressedFile) >= MAX_PATH_LEN_HUFFMAN) {
        fprintf(stderr, "[ERRO] Caminho muito longo (máximo %d caracteres)\n", MAX_PATH_LEN_HUFFMAN-1);
        return 1;
    }

    printf("[DEBUG] Iniciando descompressão de: %s\n", compressedFile);

    // Abrir arquivo comprimido
    FILE* inFile = fopen(compressedFile, "rb");
    if (!inFile) {
        perror("[ERRO] Falha ao abrir arquivo comprimido");
        return 1;
    }

    // Verificar assinatura
    char magic[8] = {0};
    if (fread(magic, 1, 7, inFile) != 7) {
        fprintf(stderr, "[ERRO] Arquivo corrompido (cabeçalho faltando)\n");
        fclose(inFile);
        return 1;
    }
    magic[7] = '\0';
    printf("[DEBUG] Cabeçalho lido: %s\n", magic);

    if (strncmp(magic, "HUFFv2|", 7) != 0) {
        fprintf(stderr, "[ERRO] Formato inválido (esperado: HUFFv2|)\n");
        fclose(inFile);
        return 1;
    }

    // Ler extensão de forma segura
    char extOrig[MAX_EXT_LEN] = {0};
    int c, i = 0;
    
    // Pular até EXT=
    while ((c = fgetc(inFile)) != '=' && c != EOF) {
        if (c == EOF) {
            fprintf(stderr, "[ERRO] Cabeçalho EXT= não encontrado\n");
            fclose(inFile);
            return 1;
        }
    }
    
    // Ler extensão
    while ((c = fgetc(inFile)) != '|' && c != EOF && i < MAX_EXT_LEN-1) {
        extOrig[i++] = c;
    }
    extOrig[i] = '\0';
    printf("[DEBUG] Extensão original: %s\n", extOrig);

    if (c == EOF) {
        fprintf(stderr, "[ERRO] Cabeçalho incompleto (faltando | após extensão)\n");
        fclose(inFile);
        return 1;
    }

    // Pular até TREE=
    while ((c = fgetc(inFile)) != '=' && c != EOF) {
        if (c == EOF) {
            fprintf(stderr, "[ERRO] Cabeçalho TREE= não encontrado\n");
            fclose(inFile);
            return 1;
        }
    }

    // Ler árvore
    printf("[DEBUG] Lendo árvore de Huffman...\n");
    No* root = lerArvore(inFile);
    if (!root) {
        fprintf(stderr, "[ERRO] Árvore de Huffman corrompida\n");
        fclose(inFile);
        return 1;
    }

    // Pular delimitador final
    while ((c = fgetc(inFile)) != '|' && c != EOF);

    // Criar caminho de saída
    char outputPath[MAX_PATH_LEN_HUFFMAN];
    int needed = snprintf(outputPath, MAX_PATH_LEN_HUFFMAN, "%s.%s", decompressedFile, extOrig);
    if (needed >= MAX_PATH_LEN_HUFFMAN) {
        fprintf(stderr, "[ERRO] Caminho de saída muito longo\n");
        liberarArvore(root);
        fclose(inFile);
        return 1;
    }
    printf("[DEBUG] Arquivo de saída: %s\n", outputPath);

    // Abrir arquivo de saída
    FILE* outFile = fopen(outputPath, "wb");
    if (!outFile) {
        perror("[ERRO] Falha ao criar arquivo de saída");
        liberarArvore(root);
        fclose(inFile);
        return 1;
    }

    // Processar dados
    printf("[DEBUG] Processando dados comprimidos...\n");
    No* current = root;
    int byte;
    size_t bytes_written = 0;
    
    while ((byte = fgetc(inFile)) != EOF) {
        for (int i = 7; i >= 0; i--) {
            current = (byte & (1 << i)) ? current->dir : current->esq;
            
            if (!current->esq && !current->dir) {
                if (fputc(current->data, outFile) == EOF) {
                    perror("[ERRO] Falha ao escrever dados descomprimidos");
                    liberarArvore(root);
                    fclose(inFile);
                    fclose(outFile);
                    return 1;
                }
                bytes_written++;
                current = root;
            }
        }
    }

    // Verificar integridade
    if (current != root) {
        fprintf(stderr, "[AVISO] Dados incompletos (último símbolo não finalizado)\n");
    }

    //printf("[DEBUG] Descompressão concluída. %zu bytes escritos.\n", bytes_written);
    printf("[DEBUG] Descompressão concluída. %lu bytes escritos.\n", (unsigned long)bytes_written);

    // Liberar recursos
    fclose(inFile);
    fclose(outFile);
    liberarArvore(root);

    return 0;
}
