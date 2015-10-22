#include "mysh.h"

/**
 * Terminates the program as per the arguments given in the command tree node.
 * If the command tree specifies an argument, the program exits with that value,
 * otherwise the exit status of the most recently executed command is used.
 *
 * @param  t the command tree, with root node of type N_COMMAND
 * @return   nothing on success, EXIT_FAILURE on exit failure...
 */
int execute_exit(CMDTREE *t)
{
  if (t->argc == 1)
  {
    //no additional arguments provided - exit with status of most recently
    //executed command
    exit(last_exit_status);
  }
  else
  {
    //argument specified - convert it to an integer, and exit with that value
    exit(atoi(t->argv[1]));
  }

  fprintf(stderr, "%s: failed to exit process\n", argv0);
  return EXIT_FAILURE;
}

/**
 * Sets the value of an internal variable to have a new value. The new value is
 * not validated (i.e. if PATH is set to be something that is not a colon-separated
 * list of directories, this function will not complain).
 *
 * @param  ident one of PATH, HOME, or CDPATH
 * @param  val   the new value of the internal variable. for PATH and CDPATH,
 *               this should be a colon-separated list of directories. for HOME
 *               this should be a directory.
 * @return       EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
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
      //found the appropriate variable - first, free the current value, then
      //set the new value
      free(*vars[i].var_ptr);
      *vars[i].var_ptr = strdup(val);
      check_allocation(*vars[i].var_ptr);
      return EXIT_SUCCESS;
    }
  }

  //if we reach here, there are no internal variables matching the given
  //identifier
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
    //the second argument (following 'cd') is ALWAYS passed to this function. if
    //this is NULL, it means there was no second argument and we just change to
    //the home directory.
    if (chdir(HOME) == -1)
    {
      MYSH_PERROR(HOME);
      return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
  }
  else
  {
    //determine full path of the directory and attempt to change to it
    char *full_path = locate_file(name, CDPATH);
    if (full_path != NULL)
    {
      if (chdir(full_path) == -1)
      {
        MYSH_PERROR(full_path);
        free(full_path);
        return EXIT_FAILURE;
      }
      free(full_path);
      return EXIT_SUCCESS;
    }
  }

  //if we get here, no directory could be found using CDPATH
  fprintf(stderr, "%s: No such file or directory\n", name);
  return EXIT_FAILURE;
}

/**
 * Locates a file by name and returns the full (absolute) path of that file. The
 * returned string is allocated and must be freed after use.
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
