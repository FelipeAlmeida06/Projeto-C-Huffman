#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/stat.h>
#include <stddef.h>

#define MAX_PATH_LEN 512    // Definir um tamanho máximo consistente

void clearScreen() {
    system("clear");
}

typedef struct {
    char* operation;
    char* originalSize;
    char* newSize;
    char* filePath;
} SuccessMessage;


char* formatFileSize(size_t bytes) {
    const char* sizes[] = { "B", "KB", "MB", "GB" };
    int i = 0;
    double dblBytes = bytes;

    while (dblBytes >= 1024 && i < 3) {
        dblBytes /= 1024;
        i++;
    }

    char* result = malloc(50 * sizeof(char));
    if (i == 0) {
        snprintf(result, 50, "%.0f %s", dblBytes, sizes[i]);
    } else {
        snprintf(result, 50, "%.2f %s", dblBytes, sizes[i]);
    }
    return result;
}


void showSuccessMessage(const char* operation, const char* originalSize, const char* newSize, const char* filePath) {
    clearScreen();
    printf("=============================================\n");
    printf("|         OPERACAO REALIZADA COM SUCESSO!   |\n");
    printf("=============================================\n\n");
    
    printf("✅ %s concluida com sucesso!\n\n", operation);
    
    printf("Estatisticas:\n");
    printf("---------------------------------------------\n");
    printf("%-25s%s\n", "Tamanho original:", originalSize);
    printf("%-25s%s\n", "Tamanho apos operacao:", newSize);
    
    if (strcmp(originalSize, "N/A") != 0 && strcmp(newSize, "N/A") != 0) {
        double orig, novo;
        sscanf(originalSize, "%lf", &orig);
        sscanf(newSize, "%lf", &novo);
        double ratio = (1 - (novo / orig)) * 100;
        printf("%-25s%.2f%%\n", "Taxa de compressao:", ratio);
    }
    
    printf("\n Arquivo %s salvo em:\n", strcmp(operation, "Compressao") == 0 ? "comprimido" : "descomprimido");
    printf("🔹 %s\n\n", filePath);
}

void getFileSize(const char* filename, size_t* size) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        *size = st.st_size;
    } else {
        *size = 0;
    }
}

int main() {
    char resp = 'L';
    char nomeSemExtensao[256] = {0};
    char inputName[256] = {0};
    char inputDir[256] = {0};
    char compressedFile[MAX_PATH_LEN] = {0};
    char* p;

    do {
        printf("======================================================================\n");
        printf("MENU DE OPCOES\n");
        printf("\n");
        printf("1. Comprimir arquivo \n");
        printf("2. Descomprimir arquivo \n");
        printf("3. Sair do programa \n");
        printf("Escolha uma opcao (1-3): ");
        scanf(" %c", &resp);
        
        switch (resp) {
        case '1': {
            printf("===========================================\n");
            printf("Opcao escolhida: 1. COMPRIMIR arquivo\n");
            printf("\n");
            printf("Digite o endereco do arquivo que deseja comprimir      Ex: C:/seu-usuario/Documents/GitHub\n");
            printf("> ");
            while (getchar() != '\n'); // Limpar buffer
            fgets(inputDir, sizeof(inputDir), stdin);
            inputDir[strcspn(inputDir, "\n")] = '\0';
    
            printf("Digite o nome do arquivo que deseja comprimir          Ex: input.txt\n");
            printf("> ");
            fgets(inputName, sizeof(inputName), stdin);
            inputName[strcspn(inputName, "\n")] = '\0';

            // Extrair nome sem extensão de forma segura
            p = strrchr(inputName, '.');
            if (p) {
                size_t len = p - inputName;
                if (len < sizeof(nomeSemExtensao)) {
                    strncpy(nomeSemExtensao, inputName, len);
                    nomeSemExtensao[len] = '\0';
                }
            } else {
                strncpy(nomeSemExtensao, inputName, sizeof(nomeSemExtensao) - 1);
            }

            // Montar caminhos
            char inputPath[MAX_PATH_LEN];
            snprintf(inputPath, sizeof(inputPath), "%s/%s", inputDir, inputName);
    
            if (snprintf(compressedFile, sizeof(compressedFile), "%s/%.*s.pcb", 
                inputDir, 
                (int)(sizeof(compressedFile) - strlen(inputDir) - 6), // 6 = "/.pcb\0"
                nomeSemExtensao) >= (int)sizeof(compressedFile)) {
                fprintf(stderr, "Erro: Caminho muito longo para arquivo comprimido\n");
                break;
            }

            // Verificar se arquivo de entrada existe
            FILE* testFile = fopen(inputPath, "rb");
            if (!testFile) {
                perror("Erro ao abrir arquivo de entrada");
                break;
            }
            fclose(testFile);
            
            size_t originalBytes;
            getFileSize(inputPath, &originalBytes);
            char* originalSize = formatFileSize(originalBytes);

            // Executar compressão
            if (comprimir(inputPath, compressedFile)) {
                fprintf(stderr, "Erro durante a compressao\n");
                free(originalSize);
                break;
            }

            // Obter tamanho comprimido
            size_t compressedBytes;
            getFileSize(compressedFile, &compressedBytes);
            char* compressedSize = formatFileSize(compressedBytes);

            // Mostrar resultados
            showSuccessMessage("Compressao", originalSize, compressedSize, compressedFile);
    
            // Liberar memória
            free(originalSize);
            free(compressedSize);
            break;
        }
        case '2': {
            printf("===========================================\n");
            printf("Opcao escolhida: 2. DESCOMPRIMIR arquivo\n");
            printf("\n");

            // Limpar buffer
            while (getchar() != '\n');

            // Solicitar caminho COMPLETO
            printf("Digite endereco do arquivo .pcb que deseja descomprimir         Ex: C:/seu-usuario/Documents/GitHub/teste.pcb\n");
            printf("> ");
            
            char fullPath[512];
            fgets(fullPath, sizeof(fullPath), stdin);
            fullPath[strcspn(fullPath, "\n")] = '\0';

            // Verificar extensão .pcb
            if (strstr(fullPath, ".pcb") == NULL) {
                printf("[ERRO] O arquivo deve terminar com .pcb!\n");
                break;
            }

            // Verificar se arquivo existe
            FILE* testFile = fopen(fullPath, "rb");
            if (!testFile) {
                perror("[ERRO] Arquivo nao encontrado");
                break;
            }
            fclose(testFile);

            // Extrair nome base (sem .pcb)
            char baseName[256];
            const char* lastSlash = strrchr(fullPath, '/');
            const char* fileName = lastSlash ? lastSlash + 1 : fullPath;
            strncpy(baseName, fileName, strrchr(fileName, '.') - fileName);
            baseName[strrchr(fileName, '.') - fileName] = '\0';

            // Gerar caminho de saída
            char outputPath[512];
            if (lastSlash) {
                strncpy(outputPath, fullPath, lastSlash - fullPath + 1);
                outputPath[lastSlash - fullPath + 1] = '\0';
            } else {
                strcpy(outputPath, "./");
            }
            strcat(outputPath, "decompressed_");
            strcat(outputPath, baseName);

            // Obter tamanho ANTES da descompressão
            size_t originalSize;
            getFileSize(fullPath, &originalSize);
            char* originalSizeStr = formatFileSize(originalSize);

            // Descomprimir
            if (descomprimir(fullPath, outputPath)) {
                fprintf(stderr, "Erro na descompressao\n");
                free(originalSizeStr);
                break;
            }

            // Obter extensão original
            char extOrig[MAX_EXT_LEN] = {0};
            FILE* inFile = fopen(fullPath, "rb");
            if (inFile) {
                fseek(inFile, 7, SEEK_SET); // Pula "HUFFv2|"
                int c;
                while ((c = fgetc(inFile)) != '=' && c != EOF);
                int i = 0;
                while ((c = fgetc(inFile)) != '|' && c != EOF && i < MAX_EXT_LEN-1) {
                    extOrig[i++] = c;
                }
                fclose(inFile);
            }

            // Montar nome final com extensão
            char finalOutput[512];

            // Por esta versão segura:
            int needed = snprintf(finalOutput, sizeof(finalOutput), "%s.%.*s", 
                outputPath, 
                MAX_EXT_LEN-1, 
                extOrig);
            if (needed >= (int)sizeof(finalOutput)) {
                fprintf(stderr, "Erro: Caminho muito longo para arquivo de saida\n");
                free(originalSizeStr);
                break;
            }

            // Obter tamanho descomprimido
            size_t newSize;
            getFileSize(finalOutput, &newSize);
            char* newSizeStr = formatFileSize(newSize);

            // Mostrar resultados
            showSuccessMessage("Descompressao", originalSizeStr, newSizeStr, finalOutput);

            // Liberar memória
            free(originalSizeStr);
            free(newSizeStr);
            break;
        }
        case '3':
            printf("===========================================\n");
            printf("Opcao escolhida: 3. Sair do programa \n");
            printf("Saindo do programa ...\n");
            break;
        default:
            printf("===========================================\n");
            printf("Opçao escolhida: INVALIDA!\n");
            printf("Digite um numero valido!\n");
            break;
        }
    } while (resp != '3');

    return 0;
}

/*
Entrar na pasta do projeto: cd "Compressor_Huffman"
Executar: gcc -o huffman main.c huffman.c codigo.c -Wall -Wextra -std=c99
Executar: ./huffman
*/