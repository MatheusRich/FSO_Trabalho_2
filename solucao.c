#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#define MAX_RUNNING_TIME 30

// This function generates a random number between 0 and max. In this case, max was defined as 2.
// Addapted from: http://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
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
  } while (num_rand - defect <= (unsigned long)x); // This is carefully written not to overflow

  // Truncated division is intentional
  return x/bin_size;
}

double *dorminhoco(double tempo[], clock_t comeco, clock_t fim)
{
  comeco = clock();

  //Código

  fim = clock();


  double diff1 =  ((double) (fim - comeco)) / (CLOCKS_PER_SEC);
  tempo[0] = (int) diff1/60;
  tempo[1] = (diff1 - tempo[0] * 60);
  printf("%.0lf:%06.3lf: Mensagem do filho dorminhoco\n", tempo[0] , tempo[1]);

  return tempo;
}

void write_to_file(int passive_pipe[], clock_t parent_initial_time)
{

}

void receive_messages(int passive_pipe[],int active_pipe[], clock_t parent_initial_time)
{

  close(passive_pipe[1]);
  close(active_pipe[1]);

    write_to_file(passive_pipe, parent_initial_time);
    write_to_file(active_pipe, parent_initial_time);
}

int main()
{
  srand((unsigned)time(NULL));

  int passive_pipe[2], active_pipe[2];
  pid_t passive_process,active_process;

  // Creating pipes
  pipe(passive_pipe);
  pipe(active_pipe);

  // Creating passive child process
  passive_process = fork();

  // Parent Process
  if(passive_process>0)
  {
    // Creating active child process
    active_process = fork();

    if(active_process>0)
    {
      // Parent Process
      clock_t parent_initial_time = clock();
      clock_t parent_current_time = clock();
      double parent_running_time = -1;

      while(parent_running_time < MAX_RUNNING_TIME)
      {
        // Reading messagens from children
        receive_messages(passive_pipe,active_pipe, parent_initial_time);

        // Calculating parent running time
        parent_current_time = clock();
        parent_running_time = (double)(parent_current_time - parent_initial_time)/CLOCKS_PER_SEC;
      }
    }
    else
    {
      //Active Child Process

      printf("So fi bom\n");
    }
  }
  else
  {
    // Passive Child Process
    printf("So fi preguiça\n");
    // Sleeping
    sleep(random_number());
  }

  return 0;

}
