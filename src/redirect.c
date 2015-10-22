#include "mysh.h"

//TODO - should stdin redirection attempt to find the file using the PATH variable?

/**
 * Redirects the specified (open) stream to read from/to a file specified by
 * the given path. Prints an error message and exits on failure.
 *
 * @param file_no the file number of the open file (e.g. fileno(stdin))
 * @param path    where the file to read from or write to is located
 * @param mode    the access mode for opening the file, as per fopen (e.g. "r")
 */
void redirect_io_stream(int file_no, char *path, char *mode)
{
  FILE *fp = fopen(path, mode);

  if (fp == NULL)
  {
    //failed to open - print error and quit
    perror(path);
    exit(EXIT_FAILURE);
  }

  if (dup2(fileno(fp), file_no) == -1)
  {
    //failed to redirect - print error and quit
    perror("dup2()");
    exit(EXIT_FAILURE);
  }

  //if we reach this, all has gone well - we can close the opened file now
  fclose(fp);
}

/**
 * Checks if the root node of a command tree requires I/O redirection, and if so,
 * enables it. Quiet upon success, prints an error message and exits upon failure
 * (note - this function should always be executed in a child process, not the 'main'
 * mysh process)
 *
 * @param t the command tree - should be of type command or of type subshell
 */
void set_redirection(CMDTREE *t)
{
  //redirect input if required
  if (t->infile != NULL)
  {
    redirect_io_stream(fileno(stdin), t->infile, "r");
  }

  //redirect output if required
  if (t->outfile != NULL)
  {
    redirect_io_stream(fileno(stdout), t->outfile, t->append ? "a" : "w");
  }
}
