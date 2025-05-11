#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Adicionado para strcspn e strcat
#include <sys/stat.h>

#ifdef _WIN32
    #define CLEAR_CMD "cls"
//#else
    //#define CLEAR_CMD "clear"
#endif


#define MAX_PATH_LEN 1024

void clearScreen() {
    system(CLEAR_CMD);
}

long getFileSize(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    return size;
}

void getFileExtension(const char* filename, char* extension, size_t extSize) {
    const char* dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        strncpy(extension, dot + 1, extSize - 1);
        extension[extSize - 1] = '\0';
    } else {
        extension[0] = '\0';
    }
}

void showSuccessMessage(const char* operation, long originalSize, long newSize, const char* filePath) {
    clearScreen();
    printf("=============================================\n");
    printf("|         OPERAÇÃO REALIZADA COM SUCESSO!   |\n");
    printf("=============================================\n\n");

    printf("✔ %s concluída com sucesso!\n\n", operation);

    printf("📊 Estatísticas:\n");
    printf("---------------------------------------------\n");
    printf("Tamanho original:           %ld bytes\n", originalSize);
    printf("Tamanho após %s:       %ld bytes\n", operation, newSize);

    if (originalSize > 0) {
        double ratio = (1.0 - (double)newSize / originalSize) * 100.0;
        printf("Taxa de compressão:         %.2f%%\n", ratio);
    }

    printf("\n📁 Arquivo salvo em:\n");
    printf("🔒 %s\n\n", filePath);
}

void menu() {
    printf("======================================================================\n");
    printf("Trabalho 2 - Linguagens de Programação\n");
    printf("MENU DE OPERAÇÕES\n");
    printf("1 ----- Comprimir arquivo \n");
    printf("2 ----- Descomprimir arquivo \n");
    printf("3 ----- Sair do programa \n");
    printf("Escolha uma opção (1-3): ");
}



int main() {
    int opcao;
    char inputPath[MAX_PATH_LEN];
    char outputPath[MAX_PATH_LEN];
    char nomeBase[MAX_PATH_LEN];
    char outputFull[MAX_PATH_LEN];

    long originalSize, compressedSize, decompressedSize;  // Declare as variáveis fora do switch

    do {
        menu();
        scanf("%d", &opcao);
        while (getchar() != '\n');

        switch (opcao) {
            case 1:
                printf("\nDigite o caminho do arquivo a ser comprimido: "); // **PROMPT CORRETO - ENTRADA PRIMEIRO**
                fgets(inputPath, sizeof(inputPath), stdin);
                inputPath[strcspn(inputPath, "\n")] = '\0';

                printf("Digite o nome base do arquivo de saída (sem extensão): "); // **PROMPT CORRETO - SAÍDA DEPOIS**
                fgets(nomeBase, sizeof(nomeBase), stdin);
                nomeBase[strcspn(nomeBase, "\n")] = '\0';

                snprintf(outputPath, sizeof(outputPath), "%s", nomeBase);

                comprimir(inputPath, outputPath);

                originalSize = getFileSize(inputPath);
                compressedSize = getFileSize(outputPath);

                showSuccessMessage("Compressão", originalSize, compressedSize, outputPath);
                break;

            case 2:
                printf("\nDigite o caminho do arquivo a ser descomprimido (.huff): ");
                fgets(inputPath, sizeof(inputPath), stdin);
                inputPath[strcspn(inputPath, "\n")] = '\0';

                printf("Digite o nome base do arquivo de saída (sem extensão): ");
                fgets(nomeBase, sizeof(nomeBase), stdin);
                nomeBase[strcspn(nomeBase, "\n")] = '\0';

                descomprimir(inputPath, nomeBase);

                // Após descomprimir, a extensão original é restaurada no nome final
                compressedSize = getFileSize(inputPath);
                snprintf(outputFull, sizeof(outputFull), "%s", nomeBase);
                decompressedSize = getFileSize(outputFull);

                showSuccessMessage("Descompressão", decompressedSize, compressedSize, outputFull);
                break;

            case 3:
                printf("\nObrigado por utilizar o programa! Saindo...\n");
                break;

            default:
                printf("Opção inválida! Tente novamente.\n");
        }
    } while (opcao != 3);

    return 0;
}


// Entrar na pasta do projeto: cd "Compressor_Huffman"
// Executar: gcc -o huffman main.c huffman.c codigo.c -Wall -Wextra -std=c99
// Executar: ./huffman
//C:/Users/Felipe Almeida/Desktop/teste
//escolher um arquivo txt, pdf, imagem etc para comprimir