/**
 * Author:          Samuel Marsh
 * Date:            30/10/2015
 */ 

#include "mysh.h"

/**
 * Forward declarations so that the structure array variable below can be
 * declared at the top of the file...
 */
int command_cd(CMDTREE *, int, char **);
int command_set(CMDTREE *, int, char **);
int command_exit(CMDTREE *, int, char **);
int command_time(CMDTREE *, int, char **);

/**
 * A structure array holding internal commands used by mysh.
 */
struct {
  //the title of the command, compared with the user input (argv[0]) to
  //determine if this command should be executed.
  char *name;
  //the function to be executed, taking the command tree and arguments as
  //parameters.
  int (* main)(CMDTREE *, int, char **);
} internal_commands[] = {
  {"cd", command_cd},
  {"set", command_set},
  {"exit", command_exit},
  {"time", command_time}
};

#define N_INTERNAL_COMMANDS (sizeof(internal_commands) / sizeof(internal_commands[0]))

/**
 * An array of structures holding the internal variables that can be modified
 * using the 'set' command
 */
struct {
  char *identifier;
  char **var_ptr;
} internal_variables[] = {
  {"PATH", &PATH},
  {"HOME", &HOME},
  {"CDPATH", &CDPATH}
};

#define N_INTERNAL_VARIABLES (sizeof(internal_variables) / sizeof(internal_variables[0]))

/**
 * Attempts to execute an internal command with name matching the first argument
 * in argv.
 *
 * @param  t           the command tree, passed to the internal command main function
 * @param  argc        the argument count, passed to the internal command main function
 * @param  argv        the arguments, passed to the internal command main function
 * @param  exit_status a pointer to an integer to which the exit status of the internal
 *                     command will be set
 * @return             whether a matching internal command was found
 */
bool execute_internal_command(CMDTREE *t, int argc, char *argv[], int *exit_status)
{
  //loop through internal commands and execute if appropriate
  for (int i = 0; i < N_INTERNAL_COMMANDS; ++i)
  {
    if (strcmp(argv[0], internal_commands[i].name) == 0)
    {
      *exit_status = internal_commands[i].main(t, argc, argv);
      return true;
    }
  }
  return false;
}

/**
 * Changes the working directory, possibly using the CDPATH variable to locate
 * the referenced directory when a relative path is given.
 *
 * @param  t    unused
 * @param  argc the number of arguments
 * @param  argv the arguments
 * @return      EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int command_cd(CMDTREE *t, int argc, char *argv[])
{
  if (argc <= 1)
  {
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
    char *full_path = locate_file(argv[1], CDPATH);
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
  fprintf(stderr, "%s: %s: No such file or directory\n", argv0, argv[1]);
  return EXIT_FAILURE;
}

/**
 * Sets the value of an internal variable to have a new value. The new value is
 * not validated (i.e. if PATH is set to be something that is not a colon-separated
 * list of directories, this function will not complain - as per bash functionality).
 *
 * @param  ident one of PATH, HOME, or CDPATH
 * @param  val   the new value of the internal variable. for PATH and CDPATH,
 *               this should be a colon-separated list of directories. for HOME
 *               this should be a directory.
 * @return       EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int set_variable(char *ident, char *val)
{
  for (int i = 0; i < N_INTERNAL_VARIABLES; ++i)
  {
    if (strcmp(ident, internal_variables[i].identifier) == 0)
    {
      //found the appropriate variable - first, free the current value, then
      //set the new value
      free(*internal_variables[i].var_ptr);
      *internal_variables[i].var_ptr = strdup(val);
      check_allocation(*internal_variables[i].var_ptr);
      return EXIT_SUCCESS;
    }
  }

  //if we reach here, there are no internal variables matching the given
  //identifier
  fprintf(stderr, "%s: unrecognised variable: '%s'\n", argv0, ident);
  return EXIT_FAILURE;
}

/**
 * Sets the value of an internal variable.
 *
 * @param  t    unused
 * @param  argc the number of arguments
 * @param  argv the arguments
 * @return      EXIT_SUCCESS on success, EXIT_FAILURE on failure
 */
int command_set(CMDTREE *t, int argc, char *argv[])
{
  if (argc >= 3)
  {
    return set_variable(argv[1], argv[2]);
  }
  else
  {
    fprintf(stderr, "%s: %s: usage: set [PATH|CDPATH|HOME] [newval]\n", argv0, argv[0]);
    return EXIT_FAILURE;
  }
}

/**
 * Exits the program. If an argument is given, the program exits with that value.
 * Otherwise, the exit status of the most recently executed command is used.
 *
 * @param  t    unused
 * @param  argc the number of arguments
 * @param  argv the arguments
 * @return      nothing on success, EXIT_FAILURE on failure
 */
int command_exit(CMDTREE *t, int argc, char *argv[])
{
  if (argc == 1)
  {
    //no additional arguments provided - exit with status of most recently
    //executed command
    exit(last_exit_status);
  }
  else
  {
    //argument specified - exit with that value
    if (isdigit(argv[1][0]))
    {
        exit(atoi(argv[1]));
    }
    else
    {
        fprintf(stderr, "%s: %s: %s: numeric argument required\n", argv0, argv[0], argv[1]);
        exit(EXIT_FAILURE);
    }
  }

  fprintf(stderr, "%s: failed to exit process\n", argv0);
  return EXIT_FAILURE;
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
 * Executes a command node, timing the execution and printing it to stderr.
 *
 * @param  t           the command tree structure, pointing to a node of type COMMAND
 * @param  argc        the number of arguments
 * @param  argv        the arguments
 * @return             the exit status of the timed command
 */
int command_time(CMDTREE *t, int argc, char *argv[])
{
  --argc;
  ++argv;

  if (argc < 1)
  {
    fprintf(stderr, "Usage: time [command] [args ...]\n");
    return EXIT_FAILURE;
  }

  struct timeval st_start, st_end;

  if (gettimeofday(&st_start, NULL) == -1)
  {
    MYSH_PERROR("time_command");
    return EXIT_FAILURE;
  }

  //execute the command, record exit status
  int exit_status = execute_generic_command(t, argc, argv);

  if (gettimeofday(&st_end, NULL) == -1)
  {
    MYSH_PERROR("time_command");
    return EXIT_FAILURE;
  }

  int time_taken = timeval_to_millis(&st_end) - timeval_to_millis(&st_start);
  fprintf(stderr, "%imsec\n", time_taken);

  return exit_status;
}
