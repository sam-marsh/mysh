/**
 * Author:          Samuel Marsh
 * Date:            30/10/2015
 */

#include "mysh.h"

/**
 * Locates a file by name and returns the full (absolute) path of that file. The
 * returned string is dynamically allocated and the memory must be freed after use.
 *
 * @param  name     the relative or absolute path of the file
 * @param  prefixes a colon-separated list of directories - the target file is
 *                  searched for in these directories
 * @return          the full path on success (possibly appended to a prefix path in
 *                  the PATH variable) or NULL on failure/if not found.
 */
char *locate_file(char *name, char *prefixes)
{
  struct stat s;

  if (strchr(name, '/') != NULL)
  {
    //path is absolute - just check if the file exists/is readable
    if (stat(name, &s) != -1)
    {
      //if so, return a duplicate string - returned value always
      //must be freed
      char *result = strdup(name);
      check_allocation(result);
      return result;
    }
  }
  else
  {
    char *tmp_path = strdup(prefixes);
    check_allocation(tmp_path);

    //split up the path using colon character
    char *token = strtok(tmp_path, ":");
    while (token != NULL)
    {
      //concatenate together to make full path
      char full_path[MAXPATHLEN];
      sprintf(full_path, "%s/%s", token, name);

      //check if it exists
      if (stat(full_path, &s) != -1)
      {
        free(tmp_path);
        char *result = strdup(full_path);
        check_allocation(full_path);
        return result;
      }

      //move to the next token in the PATH variable
      token = strtok(NULL, ":");
    }

    free(tmp_path);
  }

  //didn't find anything - return null
  return NULL;
}

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
    MYSH_PERROR(path);
    exit(EXIT_FAILURE);
  }

  if (dup2(fileno(fp), file_no) == -1)
  {
    //failed to redirect - print error and quit
    MYSH_PERROR(path);
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
