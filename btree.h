#ifndef _BTREE_H
#define _BTREE_H

#include <stdio.h>
#include <cstring>

/*
 * Definicao da ordem da arvore
 */
#define ORDEM 510
// #define ORDEM 3

/*
 * Definicao da estrutura de dados do cabecalho.
 * Um objetivo do cabecalho é guardar qual o numero da pagina raiz da arvore
 */
#pragma pack(push, 1)
struct cabecalhoB
{
    int paginaRaiz;           // numero da pagina raiz da arvore
    int alturaArvore;         // altura da arvore
    int numeroElementos;      // numero de chaves armazenadas na arvore
    int numeroPaginas;        // numero de paginas da arvore
    int reservado[ORDEM - 2]; //
};
typedef struct cabecalhoB cabecalho;
/*
 * Definicao da estrutura de dados das paginas da arvore
 */
struct paginaB
{
    int numeroElementos; // numero de elementos na pagina
    int numeroPagina;    // vamos guardar o numero da pagina dentro da propria pagina
    int chaves[ORDEM - 1];
    int valores[ORDEM];
};
typedef struct paginaB pagina;
#pragma pack(pop)

class btree
{
public:
    /*
     * Construtor. Abre arquivo de indice.
     */
    btree();

    /*
     * Destrutor. Fecha arquivo de indice.
     */
    virtual ~btree();

    /*
     * Insere par chave e valor para o registro na árvore
     * Tarefas:
     * - localizar pagina para inserir registro
     * - inserir ordenado na pagina
     * - atualizar numeroElementos na pagina
     * - atualizar recursivamente as páginas ancestrais
     */
    void insereChave(int chave, int valor);

    /*
     * Remove par chave e offset.
     * Tarefas:
     * - localizar pagina para remover registro
     * - inserir ordenado na pagina
     * - atualizar numeroElementos na pagina
     * - atualizar recursivamente as páginas ancestrais
     */
    void removeChave(int chave);

    /**
     * Chamada pelo usuário 
     */
    int buscaChave(int chave);

    /*
     * Busca chave e retorna o offset. Retorna -1 caso nao encontre a chave
     * Tarefas:
     * - localizar pagina
     * - retorna offset
     */
    int buscaChaveAux(int chave, int nivel,  int paginaId);

    /*
     * Retorna numero de elementos armazenado no cabecalho da arvore
     */
    int getNumeroElementos() { return cabecalhoArvore.numeroElementos; }

    /*
     * Retorna altura da arvore armazenada no cabecalho da arvore
     */
    int getAlturaArvore() { return cabecalhoArvore.alturaArvore; }

    /*
     * Retorna o numero medio de elementos por pagina da arvore. Considerar apenas paginas folha
     */
    int computarTaxaOcupacao();

    void depuracao();

private:
    /*
     * Cabecalho da arvore
     */
    cabecalho cabecalhoArvore;

    /*
     * Instancia para ler uma pagina
     */
    pagina paginaAtual;

    /*
     * Manipulador do arquivo de dados
     */
    FILE *arquivo;

    /*
     * Criaçao de uma nova pagina. Parametro com numero da pagina deve ser passado por referencia (exemplo: int idpagina;
     * btree->novaPagina(&idpagina);) pois no retorno da funçao o idpagina tera o numero da nova pagina.
     */
    pagina *novaPagina(int *idPagina)
    {
        pagina *pg = new pagina;
        pg->numeroElementos = 0;
        fseek(arquivo, 0, SEEK_END);
        fwrite(pg, sizeof(pagina), 1, arquivo);
        leCabecalho();
        cabecalhoArvore.numeroPaginas++;
        salvaCabecalho();
        *idPagina = cabecalhoArvore.numeroPaginas-1;
        pg->numeroPagina = *idPagina;
        return pg;
    }

    /*
     * Leitura de uma pagina existente.
     */
    pagina *lePagina(int idPagina)
    {
        if (idPagina < 0 || idPagina >= cabecalhoArvore.numeroPaginas)
        {
            return NULL;
            printf("===> ERRO: tentou acessar pagina que nao deveria existir");
        }
        pagina *pg = new pagina;
        fseek(arquivo, sizeof(cabecalhoArvore) + (idPagina) * sizeof(pagina), SEEK_SET);
        memset(pg, 0, sizeof(pagina));
        size_t bytesRead = fread(pg, sizeof(pagina), 1, arquivo);

        if (bytesRead != 1)
        {
            printf("\n => ERRO: Nao foi possivel ler a pagina %d\n", idPagina);

            if (feof(arquivo))
            {
                printf("Fim do arquivo alcancado\n");
            }
            if (ferror(arquivo))
            {
                printf("Erro de I/O no arquivo\n");
                clearerr(arquivo); // Limpar flag de erro
            }
            pg = NULL;
        }

        return pg;
    }

    /*
     * Persistencia de uma pagina.
     */
    void salvaPagina(int idPagina, pagina *pg)
    {
        fseek(arquivo, sizeof(cabecalhoArvore) + (idPagina) * sizeof(pagina), SEEK_SET);
        size_t bytesWritten = fwrite(pg, sizeof(pagina), 1, arquivo);

        if (bytesWritten != 1)
        {
            printf("ERRO: Nao foi possivel salvar a pagina %d\n", idPagina);
        }
        else
        {
            fflush(arquivo); // Forçar escrita no disco, pra não dar chance de perder
        }
    }

    /*
     * Salva o cabecalho
     */
    void salvaCabecalho()
    {
        fseek(arquivo, 0, SEEK_SET);
        fwrite(&cabecalhoArvore, sizeof(cabecalhoArvore), 1, arquivo);
    }

    /*
     * Le o cabecalho
     */
    void leCabecalho()
    {
        fseek(arquivo, 0, SEEK_SET);
        fread(&cabecalhoArvore, sizeof(cabecalhoArvore), 1, arquivo);
    }

    /*
     * Insere par na árvore
     */
    int insereRecursivo(int chave, int valor, int pagina, int nivel, int *chaveProp, int *paginaIrma);

    /*
     * Depuração: imprime arquivo
     */
};

#endif /* _BTREE_H */
