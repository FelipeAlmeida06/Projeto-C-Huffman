# Projeto-C-Huffman

# Compressor de Arquivos com Algoritmo de Huffman em C

Este projeto implementa um compressor e descompressor de arquivos utilizando o algoritmo de Huffman, desenvolvido na linguagem C.

O sistema é capaz de comprimir e descomprimir diferentes tipos de arquivos binários, como:
- Arquivos textos (```.txt```)
- Imagens (```.jpg```, ```.png```)
- Arquivos de áudio (```.mp3```)
- Arquivos de vídeo (```.mp4```)
- e outros formatos

O programa analisa os bytes do arquivo, constrói dinamicamente uma árvore de Huffman, gera códigos binários otimizados e salva o resultado em um arquivo comprimido com extensão ```.pcb```, preservando a extensão original para recuperação correta durante a descompressão.

# Objetivos do Projeto

- Implementar o algoritmo de Huffman 
- Trabalahar com compressão de dados em nível de byte
- Manipular arquivos binários
- Utilizar estruturas dinâmicas, ponteiros e árvores binárias

# Funcionalidades

A compressão de arquivos, realiza:
- Leitura binária do arquivo original
- Contagem de frequência de cada byte (0–255)
- Construção da Árvore de Huffman
- Serialização da árvore no arquivo comprimido
- Preservação da extensão original do arquivo

A descompressão de arquivos, realiza:
- Leitura e validação do arquivo .pcb
- Reconstrução da árvore de Huffman
- Leitura bit a bit dos dados comprimidos
- Reconstrução do arquivo original com extensão correta
- Verificação de integridade dos dados

# O que é o Algoritmo de Huffman?

O algoritmo ou método de Huffman é um algoritmo de compressão de dados sem perdas que atribui códigos de comprimento variável a símbolos (letras, bytes), sendo os mais frequentes codificados com menos bits e os menos frequentes com mais bits, usando uma árvore binária para criar essa codificação otimizada, resultando em arquivos menores.

Em resumo:
- Contar a frequência de cada byte do arquivo
- Criar nós para cada byte único
- Construir a árvore de Huffman com base nas menores frequências
- Percorrer a árvore para gerar códigos binários
- Substituir os bytes originais pelos códigos compactados
- Armazenar a árvore + dados no arquivo comprimido

# Interface do Sistema

```bash
1. Comprimir arquivo
2. Descomprimir arquivo
3. Sair do programa
```

O programa também exibe:
- Tamanho original antes compressão/descompressão
- Tamanho após compressão/descompressão
- Taxa de compressão (%)
- Caminho do arquivo gerado

# Requisitos para utilizar o sistema

- Sistema operacional compatível com GCC e C99
- Compilador gcc instalado
- Terminal de comandos

# Como executar localmente?

## 1. Clone o repositório

```bash
https://github.com/FelipeAlmeida06/Projeto-C-Huffman
```

## 2. Acesse a pasta do projeto
```bash
cd Compressor_Huffman
```

## 3. Compilar o programa
```bash
gcc -o huffman main.c huffman.c codigo.c -Wall -Wextra -std=c99
```

## 4. Executar a aplicação
```bash
./huffman
```

Projeto desenvolvido para fins de aprendizado, focados em:
- Estruturas de dados, em especial Árvores binárias
- Manipulação de arquivos binários
- Algoritmo de Huffman
- Programação na linguagem C