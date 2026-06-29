#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int TAM_MEMORIA_FISICA;
int TAM_PAGINA;
int TAM_MAX_PROCESSO;
int NUM_QUADROS;

unsigned char *memoria_fisica;
int *quadros_livres;

typedef struct {
    int pid;
    int tamanho;
    int num_paginas;
    unsigned char *memoria_logica;
    int *tabela_paginas;
} Processo;

Processo *processos;
int quantidade_processos = 0;

void inicializar_memoria() {
    for (int i = 0; i < TAM_MEMORIA_FISICA; i++)
        memoria_fisica[i] = 0;

    for (int i = 0; i < NUM_QUADROS; i++)
        quadros_livres[i] = 1;
}

int contar_quadros_livres() {
    int livres = 0;

    for (int i = 0; i < NUM_QUADROS; i++)
        if (quadros_livres[i])
            livres++;

    return livres;
}

void visualizar_memoria() {
    int quadros_disponiveis = contar_quadros_livres();
    float percentual_livre = ((float) quadros_disponiveis / NUM_QUADROS) * 100;

    printf("Memória livre: %.2f%%\n", percentual_livre);

    for (int quadro = 0; quadro < NUM_QUADROS; quadro++) {
        printf("Quadro %d %s: ", quadro,
               quadros_livres[quadro] ? "[Livre]" : "[Ocupado]");

        int inicio = quadro * TAM_PAGINA;

        for (int deslocamento = 0; deslocamento < TAM_PAGINA; deslocamento++)
            printf("%3u ", (unsigned int) memoria_fisica[inicio + deslocamento]);

        printf("\n");
    }
}

int encontrar_quadro_livre() {
    for (int i = 0; i < NUM_QUADROS; i++)
        if (quadros_livres[i])
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

void inputs_criar_processo(InputProcesso *inputs) {
    printf("\n2 - Criar processo\n");

    while (1){
        printf("Defina o identificador do processo: ");
        scanf("%d", &inputs->id);

        if (busca_processo_por_pid(inputs->id) != -1) {
          printf("Ja existe um processo com esse identificador. Informe outro valor.\n");
          continue;
        }

        break;
    }

    while (1){
        printf("Defina o tamanho do processo (em bytes): ");
        scanf("%d", &inputs->tamanho);

        if (inputs->tamanho > TAM_MAX_PROCESSO) {
          printf("Tamanho do processo excede o máximo permitido. Informe outro valor.\n");
          continue;
        }

        break;
    }
}

int alocar_paginas(Processo *p) {
    for (int pagina = 0; pagina < p->num_paginas; pagina++) {
        int quadro_livre = encontrar_quadro_livre();

        if (quadro_livre == -1) {
            printf("Erro: nao existem quadros livres.\n");
            return 0;
        }

        quadros_livres[quadro_livre] = 0;
        p->tabela_paginas[pagina] = quadro_livre;

        int inicio_logico = pagina * TAM_PAGINA;
        int inicio_fisico = quadro_livre * TAM_PAGINA;

        for (int deslocamento = 0; deslocamento < TAM_PAGINA; deslocamento++) {
            int posicao_logica = inicio_logico + deslocamento;
            int posicao_fisica = inicio_fisico + deslocamento;

            if (posicao_logica < p->tamanho)
                memoria_fisica[posicao_fisica] =
                    p->memoria_logica[posicao_logica];
        }
    }

    return 1;
}

void criar_processo() {
    InputProcesso resposta;

    inputs_criar_processo(&resposta);

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

    // insere na memoria logica valores aleatorios de 1 a 255 pois 0 esta representando vazio 
    for (int i = 0; i < p.tamanho; i++)
        p.memoria_logica[i] = (rand() % 255) + 1;

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
        printf("Nenhum processo criado!\n");
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
    printf("Informe o tamanho da memória física (bytes): ");
    scanf("%d", &TAM_MEMORIA_FISICA);

    printf("Informe o tamanho da página (bytes): ");
    scanf("%d", &TAM_PAGINA);

    if (TAM_PAGINA > TAM_MEMORIA_FISICA) {
      printf("Erro: o tamanho da pagina nao pode ser maior que a memoria fisica.\n");
      return 1;
    }

    printf("Informe o tamanho máximo do processo (bytes): ");
    scanf("%d", &TAM_MAX_PROCESSO);

    if (TAM_MAX_PROCESSO > TAM_MEMORIA_FISICA) {
        printf("Erro: o tamanho máximo do processo nao pode ser maior que a memoria fisica.\n");
        return 1;
    }

    NUM_QUADROS = TAM_MEMORIA_FISICA / TAM_PAGINA;
    memoria_fisica = malloc(TAM_MEMORIA_FISICA * sizeof(unsigned char));
    quadros_livres = malloc(NUM_QUADROS * sizeof(int));
    processos = malloc(NUM_QUADROS * sizeof(Processo));

    if (memoria_fisica == NULL || quadros_livres == NULL || processos == NULL) {
      printf("Erro ao alocar memoria para estruturas principais");

      free(memoria_fisica);
      free(quadros_livres);
      free(processos);

      return 1;
    }

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
                printf("Encerrando...\n");
                liberar_processos();
                free(memoria_fisica);
                free(quadros_livres);
                free(processos);
                return 0;

            default:
                printf("Opcao invalida.\n");
        }
    }
}
