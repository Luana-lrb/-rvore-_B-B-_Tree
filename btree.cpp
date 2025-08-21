#include <sys/types.h>
#include <sys/stat.h>

#include "btree.h"

#ifndef _BTREE_CPP
#define _BTREE_CPP

bool fileExists(const char *filename)
{
    struct stat statBuf;
    if (stat(filename, &statBuf) < 0)
        return false;
    return S_ISREG(statBuf.st_mode);
}

btree::btree()
{
    char nomearquivo[20] = "arvoreb.dat";

    // se arquivo ja existir, abrir e carregar cabecalho
    if (fileExists(nomearquivo))
    {
        // abre arquivo
        arquivo = fopen(nomearquivo, "r+b");
        leCabecalho();
    }
    // senao, criar novo arquivo e salvar o cabecalho
    else
    {
        // cria arquivo
        arquivo = fopen(nomearquivo, "w+b");

        // atualiza cabecalho
        cabecalhoArvore.numeroElementos = 0;
        cabecalhoArvore.paginaRaiz = -1;
        cabecalhoArvore.alturaArvore = 0;
        cabecalhoArvore.numeroPaginas = 0;
        salvaCabecalho();
    }
}

btree::~btree()
{
    // fechar arquivo
    fclose(arquivo);
}

int btree::computarTaxaOcupacao()
{
    return (cabecalhoArvore.numeroElementos / (ORDEM * ORDEM)) * 100;
}

void btree::insereChave(int chave, int valor)
{
    int chaveProp, paginaIrma;
    insereRecursivo(chave, valor, cabecalhoArvore.paginaRaiz, 1, &chaveProp, &paginaIrma);
}


int btree::insereRecursivo(int chave, int valor, int paginaId, int nivel, int *chaveProp, int *paginaIrma)
{
    pagina *pg;

    // Caso 1: Árvore vazia - criar primeira página (raiz)
    if (cabecalhoArvore.paginaRaiz == -1)
    {
        int novaPaginaId;
        pg = novaPagina(&novaPaginaId);
        pg->numeroElementos = 1;
        pg->chaves[0] = chave;
        pg->valores[0] = valor;

        // Configurar como raiz
        cabecalhoArvore.paginaRaiz = novaPaginaId;
        cabecalhoArvore.alturaArvore = 1;
        cabecalhoArvore.numeroElementos++;

        salvaPagina(novaPaginaId, pg);
        salvaCabecalho();

        delete pg;
        return 0; // Sucesso, sem propagação
    }

    pg = lePagina(paginaId);

    // ===== Caso 2: Não é nível de folha - continuar descendo na árvore =====
    if (nivel < cabecalhoArvore.alturaArvore)
    {

        // Encontrar qual página filha seguir
        int proximaPagina = pg->valores[pg->numeroElementos];

        int i;
        // Prcurar a posição correta baseada nas chaves
        for (i = 0; i < pg->numeroElementos; i++)
        {
            if (chave <= pg->chaves[i])
            {
                proximaPagina = pg->valores[i];
                break;
            }
        }

        // Chamada recursiva para o próximo nível
        int resultado = insereRecursivo(chave, valor, proximaPagina, nivel + 1, chaveProp, paginaIrma);

        // retorna direto se não houve criação de nova página
        if (resultado == 0)
        {
            delete pg;
            return resultado;
        }
        else if (resultado == 1) // houve criação de nova página no nível abaixo
        {

            if (pg->numeroElementos < ORDEM - 1) // Há espaço para inserir
            {
                int posicao = pg->numeroElementos;

                // Encontrar posição de inserção
                for (int i = 0; i < pg->numeroElementos; i++)
                {
                    if (chaveProp[0] < pg->chaves[i])
                    {
                        posicao = i;
                        break;
                    }
                }

                // Deslocar elementos para direita
                for (int i = pg->numeroElementos; i > posicao; i--)
                {
                    pg->chaves[i] = pg->chaves[i - 1];
                    pg->valores[i + 1] = pg->valores[i];
                }

                // Inserir nova chave/valor
                pg->chaves[posicao] = chaveProp[0];
                pg->valores[posicao + 1] = paginaIrma[0];
                // adicionei +1 aqui porque para cada chave tem 2 valores, à esq. e à dir. então é uma casa a mais
                pg->numeroElementos++;

                salvaPagina(paginaId, pg);
                delete pg;
                return 0; // Sucesso, sem propagação
            }
            else // Caso de overflow no pai, precisa dividir a página
            {
                int tempChaves[ORDEM];
                int tempValores[ORDEM + 1];

                // Copiar elementos existentes e inserir novo elemento ordenadamente
                int posicao = pg->numeroElementos;
                for (int i = 0; i < pg->numeroElementos; i++)
                {
                    if (chaveProp[0] < pg->chaves[i])
                    {
                        posicao = i;
                        break;
                    }
                }

                // Preencher array temporário
                int tempIndex = 0;
                for (int i = 0; i < pg->numeroElementos; i++)
                {
                    if (i == posicao)
                    {
                        tempChaves[tempIndex] = chaveProp[0];
                        tempIndex++;
                    }
                    tempChaves[tempIndex] = pg->chaves[i];
                    tempIndex++;
                }

                if (posicao == pg->numeroElementos)
                {
                    tempChaves[tempIndex] = chaveProp[0];
                }

                tempIndex = 0;
                for (int i = 0; i <= pg->numeroElementos; i++)
                {
                    if (i == posicao + 1)
                    {
                        tempValores[tempIndex] = paginaIrma[0];
                        tempIndex++;
                    }
                    tempValores[tempIndex] = pg->valores[i];
                    tempIndex++;
                }
                if (posicao + 1 > pg->numeroElementos)
                {
                    tempValores[tempIndex] = paginaIrma[0];
                }

                // Dividir: primeira metade fica na página atual, segunda metade vai para nova página
                int meio = ORDEM / 2;

                // Limpar página atual e inserir primeira metade
                pg->numeroElementos = meio;
                for (int i = 0; i < meio; i++)
                {
                    pg->chaves[i] = tempChaves[i];
                    pg->valores[i] = tempValores[i];
                }
                pg->valores[meio] = tempValores[meio];

                // Criar nova página com segunda metade
                int novaPaginaId;
                pagina *novaPagina = this->novaPagina(&novaPaginaId);
                novaPagina->numeroElementos = ORDEM - meio;
                for (int i = 0; i < ORDEM - meio; i++)
                {
                    novaPagina->chaves[i] = tempChaves[meio + i];
                    novaPagina->valores[i] = tempValores[meio + i];
                }
                novaPagina->valores[ORDEM - meio] = tempValores[ORDEM];
                // Salvar ambas as páginas
                salvaPagina(paginaId, pg);
                salvaPagina(novaPaginaId, novaPagina);

                if (nivel != 1)
                {
                    chaveProp[0] = tempChaves[meio]; // Chave promovida para cima
                    paginaIrma[0] = novaPaginaId;    // Página irmã criada

                    delete pg;
                    delete novaPagina;
                    return 1;
                }
                else
                {
                    pagina *novaRaiz;
                    int novaRaizId;
                    novaRaiz = this->novaPagina(&novaRaizId);
                    novaRaiz->numeroElementos = 1;
                    novaRaiz->chaves[0] = tempChaves[meio];
                    novaRaiz->valores[0] = paginaId;     // Página antiga
                    novaRaiz->valores[1] = novaPaginaId; // Nova página

                    // Configurar como raiz
                    cabecalhoArvore.paginaRaiz = novaRaizId;
                    cabecalhoArvore.alturaArvore++;
                    // cabecalhoArvore.numeroPaginas++;

                    salvaPagina(novaRaizId, novaRaiz);
                    salvaCabecalho();

                    delete novaRaiz;
                }

                delete pg;
                delete novaPagina;
                return 0; // Sucesso, sem propagação
            }
        }
    }

    // ====== Caso 3: É folha - inserir aqui ======

    // Verificar se há espaço na página
    if (pg->numeroElementos < ORDEM - 1)
    { // Há espaço - inserir ordenadamente
        int posicao = pg->numeroElementos;

        // Encontrar posição de inserção
        for (int i = 0; i < pg->numeroElementos; i++)
        {
            if (chave < pg->chaves[i])
            {
                posicao = i;
                break;
            }
        }

        // Deslocar elementos para direita
        for (int i = pg->numeroElementos; i > posicao; i--)
        {
            pg->chaves[i] = pg->chaves[i - 1];
            pg->valores[i] = pg->valores[i - 1];
        }

        // Inserir nova chave/valor
        pg->chaves[posicao] = chave;
        pg->valores[posicao] = valor;
        pg->numeroElementos++;

        cabecalhoArvore.numeroElementos++;
        salvaPagina(paginaId, pg);
        salvaCabecalho();

        delete pg;
        return 0; // Sucesso, sem propagação
    }

    // ====== Caso 4: Overflow - página está cheia, precisa dividir ======
    
    // Criar array temporário com todos os elementos + novo elemento
    int tempChaves[ORDEM];
    int tempValores[ORDEM + 1];

    // Copiar elementos existentes e inserir novo elemento ordenadamente
    int posicao = pg->numeroElementos;
    for (int i = 0; i < pg->numeroElementos; i++)
    {
        if (chave < pg->chaves[i])
        {
            posicao = i;
            break;
        }
    }

    // Preencher array temporário
    int tempIndex = 0;
    for (int i = 0; i < pg->numeroElementos; i++)
    {
        if (i == posicao)
        {
            tempChaves[tempIndex] = chave;
            tempValores[tempIndex] = valor;
            tempIndex++;
        }
        tempChaves[tempIndex] = pg->chaves[i];
        tempValores[tempIndex] = pg->valores[i];
        tempIndex++;
    }
    if (posicao == pg->numeroElementos)
    {
        tempChaves[tempIndex] = chave;
        tempValores[tempIndex] = valor;
    }

    // Dividir: primeira metade fica na página atual, segunda metade vai para nova página
    int meio = ORDEM / 2;

    // Limpar página atual e inserir primeira metade
    pg->numeroElementos = meio;
    for (int i = 0; i < meio; i++)
    {
        pg->chaves[i] = tempChaves[i];
        pg->valores[i] = tempValores[i];
    }

    // Criar nova página com segunda metade
    int novaPaginaId;
    pagina *novaPagina = this->novaPagina(&novaPaginaId);
    novaPagina->numeroElementos = ORDEM - meio;
    for (int i = 0; i < ORDEM - meio; i++)
    {
        novaPagina->chaves[i] = tempChaves[meio + i];
        novaPagina->valores[i] = tempValores[meio + i];
    }

    // colocar apontamentos entre irmas
    int apontamentoAntigo = pg->valores[ORDEM];
    pg->valores[ORDEM] = novaPaginaId;
    novaPagina->valores[ORDEM] = apontamentoAntigo;

    // Salvar ambas as páginas
    salvaPagina(paginaId, pg);
    salvaPagina(novaPaginaId, novaPagina);

    cabecalhoArvore.numeroElementos++;
    salvaCabecalho();

    if (nivel != 1)
    {
        chaveProp[0] = tempChaves[meio]; // Chave promovida para cima
        paginaIrma[0] = novaPaginaId;    // Página irmã criada

        delete pg;
        delete novaPagina;
        return 1;
    }
    else // Estamos na raiz
    {
        pagina *novaRaiz;
        int novaRaizId;
        novaRaiz = this->novaPagina(&novaRaizId);
        novaRaiz->numeroElementos = 1;
        novaRaiz->chaves[0] = tempChaves[meio];
        novaRaiz->valores[0] = paginaId;     // Página antiga
        novaRaiz->valores[1] = novaPaginaId; // Nova página

        // Configurar como raiz
        cabecalhoArvore.paginaRaiz = novaRaizId;
        cabecalhoArvore.alturaArvore++;

        salvaPagina(novaRaizId, novaRaiz);
        salvaCabecalho();

        delete novaRaiz;
    }

    delete pg;
    delete novaPagina;
    return 0; // Sucesso, sem propagação
}

void btree::removeChave(int chave)
{
    // neste trabalho, não é necessário implementar a remoção!

    // se remover, atualizar cabecalho
    if (true)
    {
        cabecalhoArvore.numeroElementos--;
        salvaCabecalho();
    }
}

int btree::buscaChave(int chave)
{
    return buscaChaveAux(chave, 1, cabecalhoArvore.paginaRaiz);
}

int btree::buscaChaveAux(int chave, int nivel, int paginaId)
{
    pagina *pg = lePagina(paginaId);

    if (nivel < cabecalhoArvore.alturaArvore)
    {
        // Encontrar qual página filha seguir
        int proximaPagina = pg->valores[pg->numeroElementos];
        int i;

        // Prcurar a posição correta baseada nas chaves

        for (i = 0; i < pg->numeroElementos; i++)
        {
            if (chave <= pg->chaves[i])
            {
                proximaPagina = pg->valores[i];
                break;
            }
        }

        // Chamada recursiva para o próximo nível
        int resultado = buscaChaveAux(chave, nivel + 1, proximaPagina);

        // retorna direto se não houve criação de nova página
        if (resultado == -1)
        {
            delete pg;
            return resultado;
        }
        else
        {
            delete pg;
            return resultado; // retorna o valor encontrado
        }
    }
    else // É folha - procurar aqui
    {

        // Procurar chave na página atual
        for (int i = 0; i < pg->numeroElementos; i++)
        {
            if (pg->chaves[i] == chave)
            {
                int valor = pg->valores[i];
                delete pg;
                return valor; // se encontrar chave, retornar valor
            }
        }
        // Se não encontrar, retornar -1
        delete pg;
        return -1;
    }
}

void btree::depuracao()
{
    // imprime cabeçalho
    printf("Cabecalho:\n");
    printf("paginaRaiz      = %d\n", cabecalhoArvore.paginaRaiz);
    printf("alturaArvore    = %d\n", cabecalhoArvore.alturaArvore);
    printf("numeroElementos = %d\n", cabecalhoArvore.numeroElementos);
    printf("numeroPaginas   = %d\n", cabecalhoArvore.numeroPaginas);

    // salta cabeçalho
    fseek(arquivo, sizeof(cabecalhoArvore), SEEK_SET);
    for (int i = 0; i < cabecalhoArvore.numeroPaginas; i++)
    {
        // le pagina
        pagina pg;
        fread(&pg, sizeof(pagina), 1, arquivo);
        printf("\n==========");
        printf("\npagina %d\n", i);
        printf("\nchaves: ");
        for (int j = 0; j < pg.numeroElementos; j++)
            printf("%d ", pg.chaves[j]);
        printf("\nvalores: ");
        for (int j = 0; j <= pg.numeroElementos; j++)
            printf("%d ", pg.valores[j]);
        // Mas aí tem q ignorar que vai mostrar um valor a mais no nós folhas
    }
}

#endif /* _BTREE_CPP */
