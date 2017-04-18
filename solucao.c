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

  int fd1[2], fd2[2];
  pid_t pid,pid2;

  // Conectar pipes nos filhos.

  /* Criando nosso Pipe */
      if(pipe(fd1)<0)
      {
          perror("pipe") ;
          return -1 ;
      }

      if(pipe(fd2)<0)
      {
          perror("asdf");
          return -2;
      }
      // Criando o processo filho
      if ((pid = fork()) < 0)
      {
        perror("fork");
        exit(1);
      }

      // Processo pai
      if(pid>0)
      {

        // Criando o processo filho2
        if ((pid2 = fork()) < 0)
        {
          perror("fork2");
          exit(1);
        }

        //Aqui é papis
        if(pid2>0)
        {
          // Fechando a porta de escrita
          close(fd1[1]);
          close(fd2[1]);

          char str_recebida[256]="";

          read(fd1[0], str_recebida, sizeof(str_recebida));

          printf("%s - DEU CARAI\n", str_recebida);
        }
        else
        {
          //filho 2
          char str[256]="";

          // Fechando a porta de leitura
          close(fd2[0]);

          printf("Escreve aí filha da puta 2:\n" );
          scanf("%s", str);

          // printf("timestamp: %Lf\n", ((long double)(timestamp2-timestamp1))/CLOCKS_PER_SEC);
          write(fd2[1], str, sizeof(str) + 1);

        }
      }
      // Processo Filho 1
      else
      {
        char str[256]="";

        // Fechando a porta de leitura
        close(fd1[0]);

        printf("Escreve aí filha da puta:\n" );
        scanf("%s", str);

        // printf("timestamp: %Lf\n", ((long double)(timestamp2-timestamp1))/CLOCKS_PER_SEC);
        write(fd1[1], str, sizeof(str) + 1);

        //Mandando o mlk dormir
        sleep(random_number());
      }

  return 0;

}
