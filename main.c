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

Processo processos[NUM_QUADROS]; // array com o numero maximo de processos disponiveis. Cada processo preciso de pelo menos 1 quadro;
int quantidade_processos = 0;

void inicializar_memoria(){
  // nao necessario, porem enuncioado comenta que todos os quadros precisam ser iniciado como vazios
  for (int i = 0; i < TAM_MEMORIA_FISICA; i++) {
    memoria_fisica[i] = 0;
  }

  // todos os quadros livres
  for (int i = 0; i < NUM_QUADROS; i++) {
    quadros_ocupados[i] = 0;
  }
}

int contar_quadros_livres(){
  int livres = 0;

  for (int i = 0; i < NUM_QUADROS; i++){
    if (quadros_ocupados[i] == 0){
      livres++;
    }
  }

  return livres;
}

void vizualizar_memoria() {
  int quadros_livres = contar_quadros_livres();
  float percentual_livre = ((float)quadros_livres / NUM_QUADROS) * 100;

  printf("Memória livre: %.2f%%\n", percentual_livre);

  for (int quadro = 0; quadro < NUM_QUADROS; quadro++) {
    printf("Quadro %d %s: ", quadro, (quadros_ocupados[quadro]) ? "[Ocupado]" : "[Livre]");

    int inicio = quadro * TAM_PAGINA;

    for (int deslocamento = 0; deslocamento < TAM_PAGINA; deslocamento++) {
      printf("%3u ", memoria_fisica[inicio + deslocamento]); // exibe minimo 3 digitos
    }

    printf("\n");
  }
}

int encontrar_quadro_livre() {
  for (int i = 0; i < NUM_QUADROS; i++) {
    if (quadros_ocupados[i] == 0) {
      return i;
    }
  }

  return -1;
}

int busca_processo_por_pid(int pid) {
  for (int i = 0; i < quantidade_processos; i++) {
    if (pid == processos[i].pid) {
      return i;
    }
  }

  return -1;
}

int memoria_suficiente(int num_paginas_processo) {
  return contar_quadros_livres() >= num_paginas_processo;
}

void limpar_buffer(){
  int c;

  while ((c = getchar()) != "\n" && c!= EOF){
  }
}

typedef struct {
  int id;
  int tamanho;
} InputProcesso;

int inputs_criar_processo(InputProcesso *inputs){
  printf("\n2 - Criar processo\n");

  while (1) {
    printf("Defina o identificador do processo: ");
    int id = scanf("%d", &inputs->id);

    if (id != 1) {
      printf("Valor invalido, somente inteiros\n");
      limpar_buffer();
      continue;
    }

    if (inputs->id < 0) {
      printf("Identificador não pode ser negativo.\n");
      continue;
    }
   
    int index = busca_processo_por_pid(inputs->id);
    if (index != -1) {
      printf("Processo com esse identificador ja existe, escolha outro valor\n");
      continue;
    }

    break;
  }

  while (1) {
    printf("Defina o tamanho do processo (em bytes): ");
    int tamanho = scanf("%d", &inputs->tamanho);

    if (tamanho != 1) {
      printf("Valor invalido, somente inteiros\n");
      limpar_buffer();
      continue;
    }
    
    if (inputs->tamanho <= 0) {
      printf("Tamanho deve ser maior que zero.\n");
      continue;
    }
    
    // Se o tamanho informado for maior que o tamanho máximo configurado, uma mensagem deve ser exibida e um novo valor deve ser solicitado.
    if (inputs->tamanho > TAM_MAX_PROCESSO) {
      printf("Tamanho do processo excede o máximo permitido.\n");
      continue;
    }
    
    // Se não houver memória suficiente para alocar o processo, uma mensagem deve ser exibida e o usuário deve poder solicitar outra opção.
    int numero_paginas = (inputs->tamanho + TAM_PAGINA - 1) / TAM_PAGINA;
    if (!memoria_suficiente(numero_paginas)) {
      printf("Memória insuficiente, selecione outra opção\n");
      return 0;
    }

    break;
  }

  return 1;
}

int alocar_paginas(Processo *p) {
  // encontra um quadro livre
  // quadro encontrado fica ocupado
  // indica o quadro que foi ocupado na tabela de paginas
  // copia o conteudo da memoria logica para a memoria fisica
  for (int pagina = 0; pagina < p->num_paginas; pagina++) {
    int quadro_livre = encontrar_quadro_livre();

    if (quadro_livre == -1) {
      printf("Erro: nao existem quadros livres.\n");
      return 0;
    }

    quadros_ocupados[quadro_livre] = 1;  //marca como ocupado
    p->tabela_paginas[pagina] = quadro_livre;

    int inico_logico = pagina * TAM_PAGINA;
    int inicio_fisico = quadro_livre * TAM_PAGINA;

    for (int deslocamento = 0; deslocamento < TAM_PAGINA; deslocamento++) {
      int posicao_logica = inico_logico + deslocamento;
      int posicao_fisica = inicio_fisico + deslocamento;

      if (posicao_logica < p->tamanho) {
        memoria_fisica[posicao_fisica] = p->memoria_logica[posicao_logica];
      } else {
        memoria_fisica[posicao_fisica] = 0; // Preenche com 0 o resto do quadro
      }
    }
  }

  return 1;
}

void criar_processo() {
  InputProcesso resposta;
  
  int inputs_validos = inputs_criar_processo(&resposta);

  if (!inputs_validos){
    return;
  }

  Processo p;

  p.pid = resposta.id;
  p.tamanho = resposta.tamanho;
  p.num_paginas = (resposta.tamanho + TAM_PAGINA - 1) / TAM_PAGINA;
  p.memoria_logica = malloc(resposta.tamanho);

  if(p.memoria_logica == NULL) {
    printf("Erro ao alocar memoria lógica\n");
    free(p.memoria_logica);
    return;
  }

  p.tabela_paginas = malloc(p.num_paginas * sizeof(int));

  if(p.tabela_paginas == NULL) {
    printf("Erro ao alocar tabela de páginas\n");
    free(p.tabela_paginas);
    return;
  }

  for (int i = 0; i < resposta.tamanho; i++) {
    p.memoria_logica[i] = rand() % 256;
  }

  int alocacao = alocar_paginas(&p);

  if (!alocacao) {
    free(p.memoria_logica);
    free(p.tabela_paginas);
    return;
  }

  if (quantidade_processos >= NUM_QUADROS){
    printf("Limite de processos atingido.\n");
    free(p.memoria_logica);
    free(p.tabela_paginas);
    return;
  }

  processos[quantidade_processos] = p;
  quantidade_processos++;
}

Processo input_id_processo(){
  printf("\n3 - Visualizar tabela de paginas\n");
  
  while (1) {
    int id;

    printf("Insira o identificador do processo: ");
    int leitura = scanf("%d", &id);
  
    if (leitura != 1) {
      printf("Valor invalido, somente inteiros\n");
      limpar_buffer();
      continue;
    }

    int index = busca_processo_por_pid(id);

    if (index == -1) {
      printf("Processo com esse identificador não existe, tente outro identificador\n");
      continue;
    }
    
    return processos[index];
    
  }
}

void exibir_tabela_paginas() {
  Processo p = input_id_processo();

  printf("Processo %d\n", p.pid);
  printf("Tamanho: %d bytes\n", p.tamanho);
  printf("Numero de páginas\n: %d", p.num_paginas);
  printf("Tabela de páginas:\n");

  for (int i = 0; i < p.num_paginas; i++) {
    printf("Página %d -> Quadro %d", i, p.tabela_paginas[i]);
  }
}

void liberar_processos() {
  for (int i = 0; i < quantidade_processos; i++) {
    free(processos[i].memoria_logica);
    free(processos[i].tabela_paginas);
  }
}

int main() {
  srand(time(NULL));  // seed para gerar numeros aleatorios que seram usados na memoria logica do processo. Requisito

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
        vizualizar_memoria();
        break;
      case 2:
        criar_processo();
        break;
      case 3:
        exibir_tabela_paginas();
        break;
      case 0:
        printf("Encerrando...");
        liberar_processos(); // liberar memoria alocada com free()
        return 0;
      default:
        printf("Opcao invalida.\n");
    }
  }
}