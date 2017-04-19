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
#define MAX_RUNNING_TIME 10

int random_number()
{
	long randomNumber, max;

	max = RAND_MAX - (RAND_MAX % 3);

	do
  {
		randomNumber = rand();
	} while(randomNumber >= max);

	if(randomNumber < (max/3))
  {
		return 0;
	}
  else if(randomNumber < 2 * (max/3))
  {
		return 1;
	}
  else
  {
		return 2;
	}
}

void clean_output()
{
  remove("output.txt");
}

// Formats the struct timeval to the required style.
void timestamp(struct timeval initial_time, struct timeval end, double min_sec[])
{
  double elapsed_time = ((end.tv_sec  - initial_time.tv_sec) * 1000000u + end.tv_usec - initial_time.tv_usec) / 1.e6;

  min_sec[0] = (int) elapsed_time/60; // Getting minut
	min_sec[1] = (elapsed_time - min_sec[0] * 60); // Getting seconds
}

/*##############################################################################################################*/
void write_to_file(int pipe[], struct timeval parent_initial_time)
{
  // Creating an output file if it does not exist. Else, append it.
  FILE *output = fopen("output.txt", "a");

  if(output == NULL)
  {
    printf("ERROR: File could not be opened!\n");
    exit(1);
  }

  fd_set set;
  struct timeval timeout;
	int ready = -1;

  FD_ZERO(&set);
  FD_SET(pipe[0], &set);

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;

	ready = select(FD_SETSIZE, &set, NULL, NULL, &timeout);

	// If the file descripor has activity
  if (ready)
  {
          // Open pipe stream read end
          FILE* stream;
          stream = fdopen (pipe[0], "r");

          //Ready file and print to output
          char buffer[1024];
          double time_sec_mili[2];
          if ((fgets (buffer, sizeof(buffer), stream) != NULL))
          {
              struct timeval parent_end_time;
              gettimeofday(&parent_end_time, NULL);
              timestamp(parent_initial_time, parent_end_time, time_sec_mili);
              // fprintf(output, "%.0lf:%08.5lf: %s", time_sec_mili[0], time_sec_mili[1], buffer);
              fprintf(output, "%.0lf:%06.3lf: %s", time_sec_mili[0], time_sec_mili[1], buffer);
          }
    }

  fclose(output);
}

void receive_messages(int passive_pipe[],int active_pipe[], struct timeval parent_initial_time)
{
  // Closing write end of stream.
	close(active_pipe[1]);
  close(passive_pipe[1]);

	struct timeval parent_current_time;
	gettimeofday(&parent_current_time, NULL);
	double parent_running_time = -1;

	while(parent_running_time < MAX_RUNNING_TIME)
	{
		write_to_file(active_pipe, parent_initial_time);
		write_to_file(passive_pipe, parent_initial_time);

		// Calculating parent running time
		gettimeofday(&parent_current_time, NULL);
		parent_running_time = ((parent_current_time.tv_sec + (parent_current_time.tv_usec/1000000u)) - (parent_initial_time.tv_sec + (parent_initial_time.tv_usec/1000000u)));
	}
}

// Sends a message from the child processes to the parent.
void sendmessage(int *pipe, int message_id, double *time_sec_mili, const char* message)
{
  FILE *final_message;
  close(pipe[0]);

  final_message = fdopen(pipe[1],"w");
  // fprintf(final_message, "%.0lf:%08.5lf: Mensagem %d %s\n", time_sec_mili[0], time_sec_mili[1], message_id, message);
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

      // Reading messagens from children
      receive_messages(passive_pipe,active_pipe, parent_initial_time);

      // Killing child processes
			printf("\nATENCAO: Tempo excedido! Finalizando o programa!\n");
			sleep(1);
      kill(passive_process, SIGKILL);
      kill(active_process, SIGKILL);
    }
    else      //Active Child Process
    {
			char keyboard_mes[100];
			struct timeval initial_time, end_time;
			double time_sec_mili[2];
			int message_id = 1; // Index of messages

      gettimeofday(&initial_time, NULL);

      while (TRUE)
      {
				// Getting the user message
				printf("Digite uma mensagem: \n");
				fgets(keyboard_mes, 100, stdin);

				// Excluding the \n at the string end
				keyboard_mes[strlen(keyboard_mes)-1]= '\0';

				// Assembling the string
        char final_string[120] = "do usuario: <";
        strcat(final_string, keyboard_mes);
        strcat(final_string, ">");

				gettimeofday(&end_time, NULL);
        timestamp(initial_time, end_time, time_sec_mili);
        sendmessage(active_pipe, message_id, time_sec_mili, final_string);

        message_id++;
      }
    }
  }
  else     // Passive Child Process
  {
    struct timeval initial_time, end_time;
    double time_sec_mili[2];
    char passive_message[] = {"do filho dorminhoco"};
    int message_id = 1; // Index of messages

    gettimeofday(&initial_time, NULL);

    while (1)
    {
      gettimeofday(&end_time, NULL);
      timestamp(initial_time, end_time, time_sec_mili);
      sendmessage(passive_pipe, message_id, time_sec_mili, passive_message);
      message_id++;

      // Sleeping 0-2 seconds
      sleep(random_number());
    }
  }

  return 0;
}
