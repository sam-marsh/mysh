#include "mysh.h"

/**
 * Prints a time in milliseconds to the error stream.
 *
 * @param time the execution time in milliseconds
 */
void print_execution_time(int time)
{
  fprintf(stderr, "%imsec\n", time);
}

/**
 * Converts a timeval structure to a time in milliseconds.
 *
 * @param  tv a pointer to the timeval structure - this will not be modified
 * @return    the time represented by the timeval, converted to milliseconds
 */
int timeval_to_millis(struct timeval * const tv)
{
  return tv->tv_sec * 1000 + tv->tv_usec / 1000;
}

/**
 * Executes a command node, timing the execution.
 *
 * @param  command     the command tree structure, pointing to a node of type COMMAND
 * @param  path        the path to the program or script
 * @param  argv        the arguments to be passed to the program
 * @param  exit_status the address of an int variable where the exit status of the
 *                     program will be stored
 * @return             the number of milliseconds taken to execute the command
 */
int time_command(CMDTREE *command, char *path, char **argv, int *exit_status)
{
  int err;
  struct timeval st_start, st_end;

  if ((err = gettimeofday(&st_start, NULL)) == -1)
  {
    MYSH_PERROR("time_command");
  }

  //execute the command, storing the exit status in the variable passed to the function
  *exit_status = execute_command(command, path, argv);

  if ((err = gettimeofday(&st_end, NULL)) == -1)
  {
    MYSH_PERROR("time_command");
  }

  return timeval_to_millis(&st_end) - timeval_to_millis(&st_start);
}
