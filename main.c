#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAM_MEMORIA_FISICA 1024
#define TAM_PAGINA 64
#define TAM_MAX_PROCESSO 512
#define NUM_QUADROS (TAM_MEMORIA_FISICA / TAM_PAGINA)

unsigned char memoria_fisica[TAM_MEMORIA_FISICA];
int quadros_ocupados[NUM_QUADROS];

typedef struct {
    int pid;
    int tamanho;
    int num_paginas;
    unsigned char *memoria_logica;
    int *tabela_paginas;
} Processo;

Processo processos[NUM_QUADROS];
int quantidade_processos = 0;

void inicializar_memoria() {
    for (int i = 0; i < TAM_MEMORIA_FISICA; i++)
        memoria_fisica[i] = 0;

    for (int i = 0; i < NUM_QUADROS; i++)
        quadros_ocupados[i] = 0;
}

int contar_quadros_livres() {
    int livres = 0;

    for (int i = 0; i < NUM_QUADROS; i++)
        if (!quadros_ocupados[i])
            livres++;

    return livres;
}

void visualizar_memoria() {
    int quadros_livres = contar_quadros_livres();
    float percentual_livre = ((float) quadros_livres / NUM_QUADROS) * 100;

    printf("Memória livre: %.2f%%\n", percentual_livre);

    for (int quadro = 0; quadro < NUM_QUADROS; quadro++) {
        printf("Quadro %d %s: ", quadro,
               quadros_ocupados[quadro] ? "[Ocupado]" : "[Livre]");

        int inicio = quadro * TAM_PAGINA;

        for (int deslocamento = 0; deslocamento < TAM_PAGINA; deslocamento++)
            printf("%3u ", (unsigned int) memoria_fisica[inicio + deslocamento]);

        printf("\n");
    }
}

int encontrar_quadro_livre() {
    for (int i = 0; i < NUM_QUADROS; i++)
        if (!quadros_ocupados[i])
            return i;

    return -1;
}

int busca_processo_por_pid(int pid) {
    for (int i = 0; i < quantidade_processos; i++)
        if (pid == processos[i].pid)
            return i;

    return -1;
}
int memoria_suficiente(int num_paginas_processo) {
    return contar_quadros_livres() >= num_paginas_processo;
}


typedef struct {
    int id;
    int tamanho;
} InputProcesso;

int inputs_criar_processo(InputProcesso *inputs) {
    printf("\n2 - Criar processo\n");

    printf("Defina o identificador do processo: ");
    scanf("%d", &inputs->id);

    printf("Defina o tamanho do processo (em bytes): ");
    scanf("%d", &inputs->tamanho);

    return 1;
}

int alocar_paginas(Processo *p) {
    for (int pagina = 0; pagina < p->num_paginas; pagina++) {
        int quadro_livre = encontrar_quadro_livre();

        if (quadro_livre == -1) {
            printf("Erro: nao existem quadros livres.\n");
            return 0;
        }

        quadros_ocupados[quadro_livre] = 1;
        p->tabela_paginas[pagina] = quadro_livre;

        int inicio_logico = pagina * TAM_PAGINA;
        int inicio_fisico = quadro_livre * TAM_PAGINA;

        for (int deslocamento = 0; deslocamento < TAM_PAGINA; deslocamento++) {
            int posicao_logica = inicio_logico + deslocamento;
            int posicao_fisica = inicio_fisico + deslocamento;

            if (posicao_logica < p->tamanho)
                memoria_fisica[posicao_fisica] =
                    p->memoria_logica[posicao_logica];
            else
                memoria_fisica[posicao_fisica] = 0;
        }
    }

    return 1;
}

void criar_processo() {
    InputProcesso resposta;

    if (quantidade_processos >= NUM_QUADROS) {
    printf("Limite máximo de processos atingido.\n");
    return;
}

    inputs_criar_processo(&resposta);
    if (resposta.tamanho > TAM_MAX_PROCESSO) {
      printf("Tamanho do processo excede o máximo permitido.\n");
      return;
    }

    int numero_paginas = (resposta.tamanho + TAM_PAGINA - 1) / TAM_PAGINA;

    if (!memoria_suficiente(numero_paginas)) {
        printf("Memória insuficiente para criar o processo.\n");
        return;
}

    Processo p;

    p.pid = resposta.id;
    p.tamanho = resposta.tamanho;
    p.num_paginas = numero_paginas;
    p.memoria_logica = malloc(resposta.tamanho);

    if (p.memoria_logica == NULL) {
        printf("Erro ao alocar memoria lógica\n");
        return;
    }

    p.tabela_paginas = malloc(p.num_paginas * sizeof(int));

    if (p.tabela_paginas == NULL) {
        printf("Erro ao alocar tabela de páginas\n");
        free(p.memoria_logica);
        return;
    }

    for (int i = 0; i < resposta.tamanho; i++)
        p.memoria_logica[i] = rand() % 256;

    if (!alocar_paginas(&p)) {
        free(p.memoria_logica);
        free(p.tabela_paginas);
        return;
    }

    processos[quantidade_processos++] = p;

    printf("Processo %d criado com sucesso. Paginas: %d\n",
           p.pid, p.num_paginas);
    }
           Processo *input_id_processo() {
    printf("\n3 - Visualizar tabela de paginas\n");

    int id;

    printf("Insira o identificador do processo: ");
    scanf("%d", &id);

    int index = busca_processo_por_pid(id);

    if (index == -1) {
        printf("Processo nao encontrado.\n");
        return NULL;
    }

    return &processos[index];
}

void exibir_tabela_paginas() {
    if (quantidade_processos == 0) {
        printf("Nenhum processo criado!");
        return;
    }

    Processo *p = input_id_processo();

    if (p == NULL) {
        return;
    }

    printf("Processo %d\n", p->pid);
    printf("Tamanho: %d bytes\n", p->tamanho);
    printf("Numero de páginas: %d\n", p->num_paginas);
    printf("Tabela de páginas:\n");

    for (int i = 0; i < p->num_paginas; i++) {
        printf("Página %d -> Quadro %d\n", i, p->tabela_paginas[i]);
    }
}

void liberar_processos() {
    for (int i = 0; i < quantidade_processos; i++) {
        free(processos[i].memoria_logica);
        free(processos[i].tabela_paginas);
    }
}

int main() {
    srand(time(NULL));

    inicializar_memoria();

    while (1) {
        int opcao;

        printf("\n1 - Visualizar memoria\n");
        printf("2 - Criar processo\n");
        printf("3 - Visualizar tabela de paginas\n");
        printf("0 - Sair\n");
        printf("Opcao: ");

        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                visualizar_memoria();
                break;

            case 2:
                criar_processo();
                break;

            case 3:
                exibir_tabela_paginas();
                break;

            case 0:
                printf("Encerrando...");
                liberar_processos();
                return 0;

            default:
                printf("Opcao invalida.\n");
        }
    }
}
