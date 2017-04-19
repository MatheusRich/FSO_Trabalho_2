#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#define TRUE 1
#define FALSE 0
#define MAX_RUNNING_TIME 30

// This function generates a random number between 0 and max. In this case, max was defined as 2.
// Addapted from available: http://stackoverflow.com/questions/2509679/how-to-generate-a-random-number-from-within-a-range
// long random_number()
// {
//   long max = 2;
//   // max <= RAND_MAX < ULONG_MAX, so this is okay.
//   unsigned long num_bins = (unsigned long) max + 1,
//                             num_rand = (unsigned long) RAND_MAX + 1,
//                             bin_size = num_rand / num_bins,
//                             defect   = num_rand % num_bins;
//   long x;
//
//   do
//   {
//     x = random();
//   } while (num_rand - defect <= (unsigned long)x); // This is carefully written not to overflow
//
//   // Truncated division is intentional
//   return x/bin_size;
// }

long random_number()
{
  int random_num = -1;

  random_num = rand()%3;

  return random_num;
}

void clean_output()
{
  remove("output.txt");
}

void formatTimestamp(struct timeval start, struct timeval end, double times[]){
  //times[0] = minutes
  //times[1] = seconds.miliseconds
  double cpuTimeUsed = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;

  times[0] = (int) cpuTimeUsed/60;
  times[1] = (cpuTimeUsed - times[0] * 60);
}

int is_ready(int ready, fd_set set, struct timeval timeout)
{

  if (select(FD_SETSIZE, &set, NULL, NULL, &timeout))
    return TRUE;

  return FALSE;
}

// Formats the struct timeval to the required style.
//times[0] = minutes
//times[1] = seconds.miliseconds
void timestamp(struct timeval start, struct timeval end, double times[])
{
  double cpuTimeUsed = ((end.tv_sec  - start.tv_sec) * 1000000u + end.tv_usec - start.tv_usec) / 1.e6;

  times[0] = (int) cpuTimeUsed/60;
  times[1] = (cpuTimeUsed - times[0] * 60);
}

void write_to_file(int pipe[], struct timeval parent_initial_time)
{

  // Creating an output file if it does not exist. Else, append it.
  // clean_output();
  FILE *output = fopen("output.txt", "a");

  if(output == NULL)
  {
    printf("ERROR: File could not be opened!\n");
    exit(1);
  }

  fd_set set;
  struct timeval timeout;

  FD_ZERO(&set);
  FD_SET(pipe[0], &set);

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

  int ready = -1;
  is_ready(ready, set, timeout);
  // int result = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
  // If the file descripor has activity
  if (ready)
  // if(result>0)
  {
          // Open pipe stream read end
          FILE* stream;
          stream = fdopen (pipe[0], "r");

          //Ready file and print to output
          char buffer[1024];
          double time_sec_mili[2];
          if ((fgets (buffer, sizeof(buffer), stream) != NULL))
          {
              struct timeval child_end_time;
              gettimeofday(&child_end_time, NULL);
              timestamp(parent_initial_time, child_end_time, time_sec_mili);
              fprintf(output, "%.0lf:%06.3lf: %s", time_sec_mili[0], time_sec_mili[1], buffer);
          }
    }

  fclose(output);
}

void receive_messages(int passive_pipe[],int active_pipe[], struct timeval parent_initial_time)
{
  // Closing write end of stream.
  close(passive_pipe[1]);
  close(active_pipe[1]);

  write_to_file(active_pipe, parent_initial_time);
  write_to_file(passive_pipe, parent_initial_time);
}

// Sends a message from the child processes to the parent.
void sendmessage(int *pipe, int message_id, double *time_sec_mili, const char* message)
{
  FILE *final_message;
  close(pipe[0]);

  // printf("%.0lf:%06.3lf: Mensagem %d %s\n", time_sec_mili[0], time_sec_mili[1], message_id, message);
  final_message = fdopen(pipe[1],"w");
  fprintf(final_message, "%.0lf:%06.3lf: Mensagem %d %s\n", time_sec_mili[0], time_sec_mili[1], message_id, message);

  fflush(final_message);
}

int main()
{
  clean_output();
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
      struct timeval parent_initial_time;
      gettimeofday(&parent_initial_time, NULL);
      struct timeval parent_current_time;
      gettimeofday(&parent_current_time, NULL);
      double parent_running_time = -1;

      while(parent_running_time < MAX_RUNNING_TIME)
      {
        // Reading messagens from children
        receive_messages(passive_pipe,active_pipe, parent_initial_time);

        // Calculating parent running time
        gettimeofday(&parent_current_time, NULL);
        parent_running_time = ((parent_current_time.tv_sec + (parent_current_time.tv_usec/1000000u)) - (parent_initial_time.tv_sec + (parent_initial_time.tv_usec/1000000u)));
      }

      // Killing child processes
      kill(passive_process, SIGKILL);
      kill(active_process, SIGKILL);
    }
    else
    {
      //Active Child Process
      char keyboard_mes[50];

      printf("Entre com as menssagens: \n");

      //Get current local time to compute timestamp
      struct timeval initial_time, end_time;
      double time_sec_mili[2];
      gettimeofday(&initial_time, NULL); // get intial time

      // Index of messages
      int message_id = 1;

      while (1)
      {
        scanf("%s", keyboard_mes);

        char final_string[80] = "do usuario: <";
        strcat(final_string, keyboard_mes);
        strcat(final_string, ">");

        gettimeofday(&end_time, NULL); // get end of each messagem

        timestamp(initial_time, end_time, time_sec_mili);

        sendmessage(active_pipe, message_id, time_sec_mili, final_string);
        //printf("%s\n", final_string);
        //write_to_file(pipe, userMessage, i, times);
        message_id++;
      }
  }
}
  else
  {
    // Passive Child Process
    struct timeval initial_time, end_time;
    double time_sec_mili[2];

    gettimeofday(&initial_time, NULL);

    // Index of messages
    int message_id = 1;
    // Sleeping
    while (1)
    {
      sleep(random_number());
      gettimeofday(&end_time, NULL);

      timestamp(initial_time, end_time, time_sec_mili);
      sendmessage(passive_pipe, message_id, time_sec_mili, "do filho dorminhoco");
      message_id++;
    }
  }

  return 0;

}
