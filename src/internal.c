#include "mysh.h"

int execute_exit(CMDTREE *t)
{
  if (t->argc == 1)
  {
    exit(last_exit_status);
  }
  else
  {
    exit(atoi(t->argv[1]));
  }

  fprintf(stderr, "internal error: failed to exit process\n");
  return EXIT_FAILURE;
}

int set_variable(char *ident, char *val)
{
  struct {
    char *identifier;
    char **var_ptr;
  } vars[] = {
    {"PATH", &PATH},
    {"HOME", &HOME},
    {"CDPATH", &CDPATH}
  };
  int n = sizeof(vars) / sizeof(vars[0]);

  for (int i = 0; i < n; ++i)
  {
    if (strcmp(ident, vars[i].identifier) == 0)
    {
      *vars[i].var_ptr = strdup(val);
      return EXIT_SUCCESS;
    }
  }

  fprintf(stderr, "unrecognized variable: '%s'\n", ident);
  return EXIT_FAILURE;
}

/**
 * Changes the working directory, possibly using the CDPATH variable to locate
 * the referenced directory when a relative path is given.
 *
 * @param  name the name of the directory - can be a relative or absolute path
 * @return      EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int change_dir(char *name)
{
  if (name == NULL)
  {
    if (chdir(HOME) == -1)
    {
      perror(HOME);
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }
  else if (strchr(name, '/') != NULL)
  {
    //path is absolute - try to change to it
    if (chdir(name) == -1)
    {
      perror(name);
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }
  else
  {
    char *tmp_path = strdup(CDPATH);

    char *token = strtok(tmp_path, ":");
    while (token != NULL)
    {
      char full_path[MAXPATHLEN];
      sprintf(full_path, "%s/%s", token, name);

      struct stat s;
      int err = stat(full_path, &s);
      if (err != -1 && S_ISDIR(s.st_mode))
      {
        if (chdir(full_path) == -1)
        {
          perror(full_path);
          return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
      }
      token = strtok(NULL, ":");
    }
  }

  fprintf(stderr, "%s: No such file or directory\n", name);
  return EXIT_FAILURE;
}

/**
 * Locates a file by name and returns the full (absolute) path of that file. The
 * returned string is allocated and must be freed after use.
 *
 * @param  name the relative or absolute path of the file
 * @return      the full path on success (possibly appended to a prefix path in
 *              the PATH variable) or NULL on failure/if not found.
 */
char *locate_file(char *name)
{
  if (strchr(name, '/') != NULL)
  {
    //path is absolute - just check if the file exists/is readable
    if (access(name, R_OK) == 0)
    {
      //if so, return a duplicate string - returned value always
      //must be freed
      return strdup(name);
    }
  }
  else
  {
    //TODO - don't use strtok
    char *tmp_path = strdup(PATH);
    //split up the path using colon character
    char *token = strtok(tmp_path, ":");
    while (token != NULL)
    {
      //concatenate together to make full path
      char full_path[MAXPATHLEN];
      sprintf(full_path, "%s/%s", token, name);

      //check if we can access for reading
      if (access(full_path, R_OK) == 0)
      {
        free(tmp_path);
        return strdup(full_path);
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
    perror("gettimeofday()");
  }

  //execute the command, storing the exit status in the variable passed to the function
  *exit_status = execute_command(command, path, argv);

  if ((err = gettimeofday(&st_end, NULL)) == -1)
  {
    perror("gettimeofday()");
  }

  return timeval_to_millis(&st_end) - timeval_to_millis(&st_start);
}
