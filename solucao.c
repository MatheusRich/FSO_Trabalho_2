#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

// CITAR QUE ACHEI O BAGUI NO STACK overflow
long random_number()
{
  long max = 2;
  // max <= RAND_MAX < ULONG_MAX, so this is okay.
  unsigned long num_bins = (unsigned long) max + 1,
  num_rand = (unsigned long) RAND_MAX + 1,
  bin_size = num_rand / num_bins,
  defect   = num_rand % num_bins;
  long x;

  do
  {
    x = random();
    // This is carefully written not to overflow
  } while (num_rand - defect <= (unsigned long)x);

  // Truncated division is intentional
  return x/bin_size;
}

int main()
{
  srand((unsigned)time(NULL));

  int passive_pipe[2], active_pipe[2];
  pid_t passive_process,active_process;

  /* Criando nosso Pipe */
  pipe(passive_pipe);
  pipe(active_pipe);

  // Criando o processo filho passivo
  passive_process = fork();

  // Processo pai
  if(passive_process>0)
  {
    // Criando o processo filho2
    active_process = fork();

    //Aqui é papis
    if(active_process>0)
    {
      // Fechando a porta de escrita
      close(passive_pipe[1]);
      close(active_pipe[1]);

      char str_recebida[256]="";

      read(active_pipe[0], str_recebida, sizeof(str_recebida));
      printf("%s - Ativo\n", str_recebida);
      read(passive_pipe[0], str_recebida, sizeof(str_recebida));
      printf("%s - PASSIVO\n", str_recebida);

    }
    else
    {
      //Filho Ativo
      char str[256]="";

      // Fechando a porta de leitura
      close(active_pipe[0]);

      printf("Escreve aí filha da puta 2:\n" );
      scanf("%s", str);

      // printf("timestamp: %Lf\n", ((long double)(timestamp2-timestamp1))/CLOCKS_PER_SEC);
      write(active_pipe[1], str, sizeof(str) + 1);

    }
  }
  // Processo Filho Passivo
  else
  {
    char str[256]="messaginha";

    // Fechando a porta de leitura
    close(passive_pipe[0]);

    // printf("timestamp: %Lf\n", ((long double)(timestamp2-timestamp1))/CLOCKS_PER_SEC);
    write(passive_pipe[1], str, sizeof(str) + 1);

    // Mandando o mlk dormir
    sleep(random_number());
  }

  return 0;

}
