/*
 * ALUNOS:  Luana Rodrigues - 12311BCC028
 *          Daniel Borges - 12311BCC005
 * File:   main.cpp
 */

#include <stdlib.h>
#include <time.h>

#include "btree.h"

int main(int argc, char **argv)
{

    // iniciar a semente aleatoria
    srand(time(NULL));

    remove("arvoreb.dat"); // durante o desenvolvimento a cada execção o arquivo é apagado

    // criar arvore b
    btree *arvore = new btree();

    printf("Use essas chaves como exemplos de consultas que devem ser encontradas: ");

    // inserir numeros aleatorios na arvore
    for (int i = 0; i < 10000; i++)
    {
        int valor = rand() % 1000000 + 1;
        arvore->insereChave(valor, valor + 1);
        if (i % 1000 == 0)
        {
            printf("%d", valor);
            if (i < 8001) printf(", ");
        }
    }

    printf("\n\nEstatisticas:\n");
    printf("Numero de elementos: %d\n", arvore->getNumeroElementos());
    printf("Altura da arvore: %d\n", arvore->getAlturaArvore());
    printf("Taxa de ocupacao: %d %\n", arvore->computarTaxaOcupacao());

    int opcao = 0;
    while (opcao != 5)
    {
        printf("\n\nMenu: 1-inserir 2-remover 3-consultar 4-depurar 5-sair: ");
        scanf("%d", &opcao);
        switch (opcao)
        {
            int valor, offset;
        case 1:
            printf("\nInsere chave: ");
            scanf("%d", &valor);
            arvore->insereChave(valor, valor);
            break;
        case 2:
            printf("\nRemove chave: ");
            scanf("%d", &valor);
            arvore->removeChave(valor);
            break;
        case 3:
            printf("\nConsulta chave: ");
            scanf("%d", &valor);
            offset = arvore->buscaChave(valor);
            if (offset == -1)
                printf("\nChave %d nao encontrada.\n", valor);
            else
                printf("\nChave %d encontrada e offset=%d.\n", valor, offset);
            break;
        case 4:
            printf("\nDepuracao: ");
            arvore->depuracao();
            break;
        default:
            opcao = 5;
            break;
        }
    }

    delete arvore;

    return (EXIT_SUCCESS);
}
