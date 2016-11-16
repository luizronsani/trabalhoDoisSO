/*
Algoritmos de gerenciamento de memóriadasdasdhashdjhasjdhkajshdkjahsdkjhasd
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Quantidade de frames na memória
#define MEM_SIZE        10
// Constante true
#define true            1
// Constante false
#define false           0
// Tamanho máximo da string
#define T_STR           100

// Define se o carga dos processos será pelo arquivo de entrada
#define LERARQUIVO      true

/*
Se a política de paginação for global, será considerado todo o espaço de endereçamento
de memória para uma substituição de página. Entretanto, se a política for local, apenas o
espaço de endereçamento do processo será considerado para paginação.
*/
#define LOCAL           true
#define GLOBAL          true
// Se a política for local, POLITICA_PAG deve ser LOCAL, caso contrário, GLOBAL
#define POLITITA_PAG    LOCAL

// Quantidade frames de memória
#define FRAMES          5

/*
Tipos de Dados
*/
typedef unsigned int u_int;   // Unsigned int
typedef char string[T_STR];   // Definição do tipo string
typedef u_int boolean;        // Definição do tipo boolean

typedef struct page_frame {   // Define a estrutura de um frame de memória
  u_int n_frame;              // Endereço do frame de memória
  clock_t tempo_carga;        // Tempo em que a página foi carregada na memória
  clock_t tempo_acesso;       // Tempo em que a página foi acessada pela última vez
  struct page_frame *next;    // Próximo frame
} t_page_frame;

typedef struct process {      // Define a estrutura do processo
  u_int pid;                  // Id do processo
  u_int ciclos_execucao;      // Quantidade de ciclos a ser executado
  u_int ciclos_executados;    // Quantidade de ciclos excutados
  u_int t_pages;              // Quantidade de páginas pertencentes ao processo
  t_page_frame *paginas;      // Lista de páginas (já está na sequencia de execução)
  struct process *next;       // Apontador para o próximo processo
} t_process;

typedef struct proc_list {    // Define a estrutura da lista de processos a ser executada
  t_process *head_proc;       // Processo que esta no início da lista
  t_process *tail_proc;       // Processo que esta no final da lista
  u_int n_of_process;         // Numero de processos
} t_proc_list;

typedef struct FIFO {
  t_page_frame m_frames[FRAMES];
  u_int index_frame;
  u_int page_victim;
} t_FIFO;

/*
Protótipo das funções auxiliares
*/
int menu();
t_proc_list criaListaProcessos();
FILE *abrirArquivoEntrada(char *nomeArq);
t_page_frame *criaPageFrame(int numero);
t_process *criaProcesso();
void carregarProcessos(t_proc_list *l_process, FILE *entrada);
void lerArquivoEntrada(t_proc_list *l_process, char *nomeArq);
void mostrarDadosFrame(t_page_frame *pf);
void mostrarDadosPageFrame(t_page_frame *pf);
void mostrarDadosProcesso(t_process *proc);
void listarProcessos(t_proc_list l_process);
void liberarMemoria(t_proc_list *l_process);
void inicializarFrameFIFO(t_FIFO *fifo);
void executaPaginacaoFIFO(t_proc_list *l_process);
void executaPaginacaoLRU(t_proc_list *l_process);
void executaPaginacaoSegundaChance(t_proc_list *l_process);
boolean verificaPaginaAlocada(t_page_frame *pf, t_FIFO fifo);

/*
Protótipo dos algoritmos de paginação
*/
int FIFO_page_replacement(t_process *process);
int LRU_page_replacement(t_process *process);
int Second_Chance_page_replacement(t_process *process);


/*
Função principal
*/
int main(int argc, char** argv)
{
  // Lista de processos que serão carregados na memória
  t_proc_list l_process = criaListaProcessos();
    printf("Nome do arquivo de entrada? ");

  #if LERARQUIVO
    string nomeArq;
    printf("Nome do arquivo de entrada? ");
    fflush(stdin);
    gets(nomeArq);
    lerArquivoEntrada(&l_process, nomeArq);
  #endif

  while (true) {

    switch (menu()) {

      case 1:
              break;

      case 2: if (l_process.head_proc)
                listarProcessos(l_process);
              else
                printf("\nA lista de processos esta vazia\n\n");
              break;

      case 3: if (l_process.head_proc)
                executaPaginacaoFIFO(&l_process);
              else
                printf("\nA lista de processos esta vazia\n\n");
              break;

      case 4: if (l_process.head_proc)
                executaPaginacaoLRU(&l_process);
              else
                printf("\nA lista de processos esta vazia\n\n");
              break;

      case 5: if (l_process.head_proc)
                executaPaginacaoSegundaChance(&l_process);
              else
                printf("\nA lista de processos esta vazia\n\n");
              break;

      case 6: if (l_process.head_proc)
                liberarMemoria(&l_process);
              else
                printf("\nA lista de processos esta vazia\n\n");
              break;

      case 0: liberarMemoria(&l_process);
              exit(0);
    }

  }

  return 0;
}

/*
Implementação das funções auxiliares
*/
int menu()
{
  int opc;

  printf("1.Inserir processo manualmente\n");
  printf("2.Mostrar processos carregados\n");
  printf("3.Executar simulacao com algoritmo FIFO\n");
  printf("4.Executar simulacao com algoritmo LRU\n");
  printf("5.Executar simulacao com algoritmo Segunda Chance\n");
  printf("6.Libear memoria (zerar lista de processos)\n");
  printf("0.Sair do programa\n");
  printf("?: ");
  scanf("%d", &opc);

  return opc;
}

FILE *abrirArquivoEntrada(char *nomeArq)
{
  FILE *fp;

  if (!(fp = fopen(nomeArq, "r")))
    return NULL;

  return fp;
}

t_proc_list criaListaProcessos()
{
  t_proc_list list;

  list.head_proc     = NULL;
  list.tail_proc     = NULL;
  list.n_of_process  = 0;

  return list;
}

t_page_frame *criaPageFrame(int numero)
{
  // Aloca memória para a pagina/frame
  t_page_frame *pf = (t_page_frame*)malloc(sizeof(t_page_frame));

  pf->n_frame      = numero;
  pf->tempo_carga  = 0;
  pf->tempo_acesso = 0;
  pf->next         = NULL;

  return pf;
}

t_process *criaProcesso()
{
  // Aloca memoria para o processo
  t_process *proc = (t_process*)malloc(sizeof(t_process));

  proc->ciclos_executados = 0;
  proc->t_pages           = 0;
  proc->next              = NULL;
  proc->paginas           = NULL;

  return proc;
}

void carregarProcessos(t_proc_list *l_process, FILE *entrada)
{
  t_process     *processo;
  t_page_frame  *pf, *aux;
  int            pagina;

  while (!feof(entrada)) {
    aux = NULL;
    processo = criaProcesso();
    fscanf(entrada, "%d", &processo->pid);
    fscanf(entrada, "%d", &processo->ciclos_execucao);

    // Carrega páginas dos processos
    do {
      fscanf(entrada, "%d ", &pagina);
      if (pagina) {  // Se página for 0, indica fim de listagem de páginas
        pf = criaPageFrame(pagina);
        if (!processo->paginas) {
          processo->paginas = pf;
          aux = processo->paginas;
        }
        else {
          aux->next = pf;
          aux = pf;
        }
        processo->t_pages++;
      }
    } while (pagina);

    // Insere processo na fila de processos
    if (!l_process->head_proc) {
      l_process->head_proc = processo;
    }
    else {
      l_process->tail_proc->next = processo;
    }
    l_process->tail_proc = processo;
    l_process->n_of_process++;

  }
}

void lerArquivoEntrada(t_proc_list *l_process, char *nomeArq)
{
  FILE *fp = abrirArquivoEntrada(nomeArq);

  if (!fp) {
    fprintf(stderr, "Erro ao abrir arquivo %s\n", nomeArq);
  }
  else {
    carregarProcessos(l_process, fp);
    close(fp);
  }
}

void mostrarDadosFrame(t_page_frame *pf)
{
  printf("Numero de frame....: %d\n", pf->n_frame);
  printf("Tempo de carga.....: %u\n", pf->tempo_carga);
  printf("Tempo de acesso....: %u\n\n", pf->tempo_acesso);
}

void mostrarDadosPageFrame(t_page_frame *pf)
{

  while (pf) {
    mostrarDadosFrame(pf);
    pf = pf->next;
  }

}

void mostrarDadosProcesso(t_process *proc)
{

  printf("\n");
  printf("Identificacao do processo.....: %d\n", proc->pid);
  printf("Ciclos de execucao............: %d\n", proc->ciclos_execucao);
  printf("Ciclos de executados..........: %d\n", proc->ciclos_executados);
  printf("Quantidade de paginas.........: %d\n", proc->t_pages);
  printf("********** Paginas **********\n");
  mostrarDadosPageFrame(proc->paginas);
  printf("\n");

}

void listarProcessos(t_proc_list l_process)
{
  t_process *proc = l_process.head_proc;

  while (proc) {
    mostrarDadosProcesso(proc);
    proc = proc->next;
  }
}

void liberarMemoria(t_proc_list *l_process)
{
  t_page_frame *pf      = NULL;
  t_process    *proc    = NULL;

  // Enquanto existirem processos na lista de processos
  while (l_process->head_proc) {
    proc = l_process->head_proc;
    pf   = proc->paginas;

    // Libera memória das páginas dos processos
    while (proc->paginas) {
      // Avança uma unidade da lista de páginas do processo
      proc->paginas = proc->paginas->next;
      // Libera memória da lista de páginas do processo
      free(pf);
      // Corrigi ponteiro de páginas
      pf = proc->paginas;
    }

    // Avança uma unidade na lista de processos
    l_process->head_proc = l_process->head_proc->next;
    // Libera memória do processo
    free(proc);
  }

  // Recria lista de processos
  *l_process = criaListaProcessos();
}

void inicializarFrameFIFO(t_FIFO *fifo)
{
  int i;

  // Zera todos os frames de memória
  for (i = 0; i < FRAMES; i++)
    fifo->m_frames[i].n_frame = 0;

  // Inicializa índices
  fifo->index_frame = 0;
  fifo->page_victim = 0;

}

void executaPaginacaoFIFO(t_proc_list *l_process)
{
  FILE *f_saida;
  t_process *process = NULL;


  // Abre arquivo de saída
  if (!(f_saida = fopen("FIFO.txt", "w+"))) {
    printf("Não foi possivel criar arquivo FIFO.txt\n");
    return;
  }

  // Cabeçalho do arquivo
  fprintf(f_saida,"*************************************************************\n");
  fprintf(f_saida,"    Relatorio de troca de paginas com o algoritmo FIFO\n");
  fprintf(f_saida,"*************************************************************\n\n");

  // Executa processos
  process = l_process->head_proc;
  while (process) {
    fprintf(f_saida, "Processo........................: %d\n", process->pid);
    fprintf(f_saida, "Quantidade de paginas trocadas..: %d\n\n", FIFO_page_replacement(process));

    // Seleciona outro processo
    process = process->next;
  }

  // Fecha o arquivo de saída
  fclose(f_saida);

}

void executaPaginacaoLRU(t_proc_list *l_process)
{

}

void executaPaginacaoSegundaChance(t_proc_list *l_process)
{

}

boolean verificaPaginaAlocada(t_page_frame *pf, t_FIFO fifo)
{
  int i;

  for (i = 0; i < FRAMES; i++) {
    if (pf->n_frame == fifo.m_frames[i].n_frame)
      return true;
  }

  return false;
}

/*
Implementação dos algoritmos de paginação
*/
int FIFO_page_replacement(t_process *process)
{
  t_FIFO fifo;
  inicializarFrameFIFO(&fifo);

  clock_t clock_start = clock();
  u_int troca_paginas = 0;

  while (process->paginas) {
    // Verifica se a página já não está na memória
    if (!verificaPaginaAlocada(process->paginas, fifo)) {
      // Carrega página na memória
      fifo.m_frames[fifo.index_frame] = *(process->paginas);
      fifo.m_frames[fifo.index_frame].tempo_carga = (clock() - clock_start) / CLOCKS_PER_SEC;
      fifo.m_frames[fifo.index_frame].tempo_acesso = (clock() - clock_start) / CLOCKS_PER_SEC;

      // Ajuste do índice de frames (fila)
      fifo.index_frame = (fifo.index_frame + 1) % FRAMES;

      // Verifica se a página será substituída por outra
      if ((fifo.index_frame == fifo.page_victim) && process->paginas->next) {
        troca_paginas++;
        // Redefine página vítima (inicialmente é zero)
        fifo.page_victim = (fifo.page_victim + 1) % FRAMES;
      }
    }
    else {
      if (!process->paginas->next)
        troca_paginas--;
    }

    // Define outra página que será carregada na memória
    process->paginas = process->paginas->next;
  }

  // Retorna o número de troca de páginas
  return troca_paginas;
}

int LRU_page_replacement(t_process *process)
{

}

int Second_Chance_page_replacement(t_process *process)
{

}
