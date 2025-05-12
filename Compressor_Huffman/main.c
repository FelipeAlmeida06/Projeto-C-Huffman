#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Adicionado para strcspn e strcat
#include <sys/stat.h>  // Para função stat()

#include <errno.h>

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
    // Logs adicionais
    printf("Tentando abrir arquivo: %s\n", filename);
    
    // Verificar se o arquivo existe
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Erro ao abrir arquivo: %s\n", filename);
        perror("Detalhes");
        return -1;
    }

    // Verificação de parâmetro nulo
    if (filename == NULL) {
        fprintf(stderr, "Erro: Nome do arquivo é nulo\n");
        return -1;
    }

    // Verificar se é um arquivo regular
    struct stat st;
    if (stat(filename, &st) != 0) {
        fprintf(stderr, "Erro ao obter informações do arquivo: %s\n", filename);
        fclose(f);
        return -1;
    }

    // Verificar se é um arquivo válido para leitura
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Erro: Não é um arquivo regular: %s\n", filename);
        fclose(f);
        return -1;
    }

    // Calcular tamanho do arquivo
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);

    // Verificações adicionais de tamanho
    if (size < 0) {
        fprintf(stderr, "Erro: Tamanho de arquivo inválido para %s\n", filename);
        return -1;
    }

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
    
    // Verificação para tamanhos inválidos
    if (originalSize == -1 || newSize == -1) {
        printf("Erro: Não foi possível determinar o tamanho dos arquivos.\n");
        return;
    }

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
    //int result;

    long originalSize, compressedSize, decompressedSize;  // Declare as variáveis fora do switch

    #ifdef _WIN32
        #include <direct.h> // Para _getcwd
        #define getcwd _getcwd
    //#else
        //#include <unistd.h> // Para getcwd
    #endif

    do {
        menu();
        scanf("%d", &opcao);
        while (getchar() != '\n');

        switch (opcao) {
            case 1:
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Diretório de trabalho atual: %s\n", cwd);
                } else {
                    perror("getcwd() error");
                }

                printf("\nDigite o nome do arquivo a ser comprimido (dentro da pasta 'testes'): ");
                fgets(nomeBase, sizeof(nomeBase), stdin);
                nomeBase[strcspn(nomeBase, "\n")] = '\0';

                // PARA COMPRIMIR:
                printf("Digite o nome base do arquivo de saída (sem extensão): ");
                fgets(nomeBase, sizeof(nomeBase), stdin);
                nomeBase[strcspn(nomeBase, "\n")] = '\0';

                if ((size_t)snprintf(inputPath, sizeof(inputPath), "testes/%s", nomeBase) >= sizeof(inputPath)) {
                    fprintf(stderr, "Erro: Caminho do arquivo de entrada muito longo.\n");
                    continue;
                }

                if ((size_t)snprintf(outputPath, sizeof(outputPath), "testes/%s_compactado", nomeBase) >= sizeof(outputPath)) {
                    fprintf(stderr, "Erro: Caminho do arquivo de saída muito longo.\n");
                    continue;
                }

                comprimir(inputPath, outputPath);

                originalSize = getFileSize(inputPath);
                compressedSize = getFileSize(outputPath);

                showSuccessMessage("Compressão", originalSize, compressedSize, outputPath);
                break;

            case 2:
                printf("\nDigite o nome do arquivo a ser descomprimido (dentro da pasta 'testes'): ");
                fgets(nomeBase, sizeof(nomeBase), stdin);
                nomeBase[strcspn(nomeBase, "\n")] = '\0';

                if ((size_t)snprintf(inputPath, sizeof(inputPath), "testes/%s", nomeBase) >= sizeof(inputPath)) {
                    fprintf(stderr, "Erro: Caminho do arquivo de entrada muito longo.\n");
                    continue;
                }

                inputPath[strcspn(inputPath, "\n")] = '\0'; // Remove newline

                // PARA DESCOMPRIMIR:
                printf("Digite o nome base do arquivo de saída (sem extensão): ");
                fgets(nomeBase, sizeof(nomeBase), stdin);
                nomeBase[strcspn(nomeBase, "\n")] = '\0';

                // Remove a extensão .huff do nome do arquivo de entrada
                char *lastDot = strrchr(inputPath, '.');
                if (lastDot != NULL && strcmp(lastDot, ".huff") == 0) {
                    *lastDot = '\0';
                }

                if ((size_t)snprintf(outputPath, sizeof(outputPath), "testes/%s_descompactado", nomeBase) >= sizeof(outputPath)) {
                    fprintf(stderr, "Erro: Caminho do arquivo de saída muito longo.\n");
                    continue;
                }

                descomprimir(inputPath, outputPath);

                // Renomeia o arquivo de saída para incluir a extensão original
                char originalExtension[MAX_EXT_LEN];
                getFileExtension(inputPath, originalExtension, sizeof(originalExtension));
                
                if ((size_t)snprintf(outputFull, sizeof(outputFull), "testes/%s_descompactado.%s", nomeBase, originalExtension) >= sizeof(outputFull)) {
                    fprintf(stderr, "Erro: Caminho do arquivo de saída final muito longo.\n");
                    continue;
                }

                decompressedSize = getFileSize(outputFull);
                compressedSize = getFileSize(inputPath); // Usa o arquivo compactado para comparação

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

/*
Entrar na pasta do projeto: cd "Compressor_Huffman"
Executar: gcc -o huffman main.c huffman.c codigo.c -Wall -Wextra -std=c99  ou gcc -g -Wall -Wextra -o huffman main.c huffman.c codigo.c
Executar: ./huffman
antigo:    C:/Users/Felipe Almeida/Desktop/teste/
novo:      C:\Users\Felipe Almeida\Documents\GitHub\Projeto-C-Huffman\Compressor_Huffman\testes\
Digite o nome do arquivo a ser comprimido (dentro da pasta 'testes'):   exemplo.txt, exemplo2.txt, Programador.jpg, Tipos.pdf 
(pode se adicionar mais arquivos de outras extensões para testar)
escolher um arquivo txt, pdf, imagem etc para comprimir
*/